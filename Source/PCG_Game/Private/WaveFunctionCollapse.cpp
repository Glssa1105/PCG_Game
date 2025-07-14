#include "WaveFunctionCollapse.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AWaveFunctionCollapse::AWaveFunctionCollapse()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RandomStream.Initialize(RandomSeed);
}

void AWaveFunctionCollapse::BeginPlay()
{
    Super::BeginPlay();
    
    TileMap.Empty();
    for (const FWFCTile& Tile : TileSet)
    {
        if (!Tile.TileID.IsEmpty())
        {
            TileMap.Add(Tile.TileID, Tile);
        }
    }
}

void AWaveFunctionCollapse::GenerateGrid()
{
    if (TileSet.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No tiles defined in TileSet"));
        return;
    }

    ClearGrid();
    RandomStream.Initialize(RandomSeed);

    InitializeGrid();
    
    if (SolveWFC())
    {
        UE_LOG(LogTemp, Log, TEXT("WFC Generation completed successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WFC Generation failed"));
    }
}

void AWaveFunctionCollapse::ClearGrid()
{
    ClearGeneratedMeshes();
    Grid.Empty();
}

void AWaveFunctionCollapse::SetSeed(int32 NewSeed)
{
    RandomSeed = NewSeed;
    RandomStream.Initialize(RandomSeed);
}

void AWaveFunctionCollapse::InitializeGrid()
{
    Grid.SetNum(GridWidth);
    for (int32 X = 0; X < GridWidth; X++)
    {
        Grid[X].SetNum(GridHeight);
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            FWFCCell& Cell = Grid[X][Y];
            Cell.bCollapsed = false;
            Cell.SelectedTile = "";
            Cell.PossibleTiles.Empty();
            
            for (const FWFCTile& Tile : TileSet)
            {
                if (!Tile.TileID.IsEmpty())
                {
                    Cell.PossibleTiles.Add(Tile.TileID);
                }
            }
            
            Cell.Entropy = Cell.PossibleTiles.Num();
        }
    }
}

bool AWaveFunctionCollapse::SolveWFC()
{
    int32 Iterations = 0;
    
    while (Iterations < MaxIterations)
    {
        Iterations++;

        FIntPoint LowestEntropyPos = FindLowestEntropyCell();
        
        if (LowestEntropyPos.X == -1)
        {
            return true;
        }
        
        if (Grid[LowestEntropyPos.X][LowestEntropyPos.Y].Entropy == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("WFC failed: contradiction at (%d, %d)"), LowestEntropyPos.X, LowestEntropyPos.Y);
            return false;
        }
        
        CollapseCell(LowestEntropyPos.X, LowestEntropyPos.Y);

        PropagateConstraints(LowestEntropyPos.X, LowestEntropyPos.Y);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WFC exceeded maximum iterations"));
    return false;
}

FIntPoint AWaveFunctionCollapse::FindLowestEntropyCell()
{
    int32 MinEntropy = INT32_MAX;
    TArray<FIntPoint> CandidateCells;
    
    for (int32 X = 0; X < GridWidth; X++)
    {
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            const FWFCCell& Cell = Grid[X][Y];
            if (!Cell.bCollapsed)
            {
                if (Cell.Entropy < MinEntropy)
                {
                    MinEntropy = Cell.Entropy;
                    CandidateCells.Empty();
                    CandidateCells.Add(FIntPoint(X, Y));
                }
                else if (Cell.Entropy == MinEntropy)
                {
                    CandidateCells.Add(FIntPoint(X, Y));
                }
            }
        }
    }
    
    if (CandidateCells.Num() == 0)
    {
        return FIntPoint(-1, -1);
    }
    
    int32 RandomIndex = RandomStream.RandRange(0, CandidateCells.Num() - 1);
    return CandidateCells[RandomIndex];
}

void AWaveFunctionCollapse::CollapseCell(int32 X, int32 Y)
{
    FWFCCell& Cell = Grid[X][Y];
    
    if (Cell.PossibleTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Trying to collapse cell with no possible tiles"));
        return;
    }

    FString SelectedTile = SelectRandomTile(Cell.PossibleTiles);
    
    Cell.bCollapsed = true;
    Cell.SelectedTile = SelectedTile;
    Cell.PossibleTiles.Empty();
    Cell.PossibleTiles.Add(SelectedTile);
    Cell.Entropy = 1;
    SpawnTileAtPosition(X, Y, SelectedTile);
    
    UE_LOG(LogTemp, Log, TEXT("Collapsed cell (%d, %d) to %s"), X, Y, *SelectedTile);
}

void AWaveFunctionCollapse::PropagateConstraints(int32 X, int32 Y)
{
    TArray<FIntPoint> CellsToUpdate;
    CellsToUpdate.Add(FIntPoint(X, Y));
    
    while (CellsToUpdate.Num() > 0)
    {
        FIntPoint CurrentPos = CellsToUpdate[0];
        CellsToUpdate.RemoveAt(0);
        
        TArray<FIntPoint> Neighbors = {
            FIntPoint(CurrentPos.X, CurrentPos.Y - 1), 
            FIntPoint(CurrentPos.X + 1, CurrentPos.Y),
            FIntPoint(CurrentPos.X, CurrentPos.Y + 1), 
            FIntPoint(CurrentPos.X - 1, CurrentPos.Y) 
        };
        
        for (const FIntPoint& NeighborPos : Neighbors)
        {
            if (IsValidPosition(NeighborPos.X, NeighborPos.Y))
            {
                int32 OldEntropy = Grid[NeighborPos.X][NeighborPos.Y].Entropy;
                UpdateCellPossibilities(NeighborPos.X, NeighborPos.Y);
                if (Grid[NeighborPos.X][NeighborPos.Y].Entropy != OldEntropy)
                {
                    CellsToUpdate.AddUnique(NeighborPos);
                }
            }
        }
    }
}

void AWaveFunctionCollapse::UpdateCellPossibilities(int32 X, int32 Y)
{
    FWFCCell& Cell = Grid[X][Y];
    
    if (Cell.bCollapsed)
    {
        return;
    }
    
    TArray<FString> NewPossibleTiles;
    
    for (const FString& TileID : Cell.PossibleTiles)
    {
        bool bIsValid = true;
        
        TArray<FIntPoint> Neighbors = {
            FIntPoint(X, Y - 1),
            FIntPoint(X + 1, Y), 
            FIntPoint(X, Y + 1), 
            FIntPoint(X - 1, Y)
        };
        
        for (int32 Dir = 0; Dir < 4; Dir++)
        {
            const FIntPoint& NeighborPos = Neighbors[Dir];
            
            if (IsValidPosition(NeighborPos.X, NeighborPos.Y))
            {
                const FWFCCell& NeighborCell = Grid[NeighborPos.X][NeighborPos.Y];
                
                bool bHasValidNeighbor = false;
                for (const FString& NeighborTileID : NeighborCell.PossibleTiles)
                {
                    if (IsValidNeighbor(TileID, NeighborTileID, Dir))
                    {
                        bHasValidNeighbor = true;
                        break;
                    }
                }
                
                if (!bHasValidNeighbor)
                {
                    bIsValid = false;
                    break;
                }
            }
        }
        
        if (bIsValid)
        {
            NewPossibleTiles.Add(TileID);
        }
    }
    
    Cell.PossibleTiles = NewPossibleTiles;
    Cell.Entropy = Cell.PossibleTiles.Num();
}

bool AWaveFunctionCollapse::IsValidNeighbor(const FString& TileID, const FString& NeighborID, int32 Direction)
{
    if (!TileMap.Contains(TileID))
    {
        return false;
    }
    
    const FWFCTile& Tile = TileMap[TileID];
    
    switch (Direction)
    {
        case 0: // 上
            return Tile.UpNeighbors.Contains(NeighborID);
        case 1: // 右
            return Tile.RightNeighbors.Contains(NeighborID);
        case 2: // 下
            return Tile.DownNeighbors.Contains(NeighborID);
        case 3: // 左
            return Tile.LeftNeighbors.Contains(NeighborID);
        default:
            return false;
    }
}

void AWaveFunctionCollapse::SpawnTileAtPosition(int32 X, int32 Y, const FString& TileID)
{
    if (!TileMap.Contains(TileID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Tile ID %s not found in TileMap"), *TileID);
        return;
    }
    
    const FWFCTile& Tile = TileMap[TileID];
    
    if (Tile.Mesh)
    {
        UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Mesh_%d_%d"), X, Y));
        MeshComponent->SetStaticMesh(Tile.Mesh);
        MeshComponent->SetupAttachment(RootComponent);
        FVector Position = FVector(X * TileSize, Y * TileSize, 0.0f);
        MeshComponent->SetWorldLocation(GetActorLocation() + Position);
        
        GeneratedMeshes.Add(MeshComponent);
    }
}

void AWaveFunctionCollapse::ClearGeneratedMeshes()
{
    for (UStaticMeshComponent* MeshComponent : GeneratedMeshes)
    {
        if (MeshComponent)
        {
            MeshComponent->DestroyComponent();
        }
    }
    GeneratedMeshes.Empty();
}

bool AWaveFunctionCollapse::IsValidPosition(int32 X, int32 Y) const
{
    return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

FString AWaveFunctionCollapse::SelectRandomTile(const TArray<FString>& PossibleTiles)
{
    if (PossibleTiles.Num() == 0)
    {
        return "";
    }
    
    float TotalWeight = 0.0f;
    for (const FString& TileID : PossibleTiles)
    {
        if (TileMap.Contains(TileID))
        {
            TotalWeight += TileMap[TileID].Weight;
        }
    }

    float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const FString& TileID : PossibleTiles)
    {
        if (TileMap.Contains(TileID))
        {
            CurrentWeight += TileMap[TileID].Weight;
            if (RandomValue <= CurrentWeight)
            {
                return TileID;
            }
        }
    }

    return PossibleTiles[0];
}