#include "WaveFunctionCollapse.h"

#include "WFCTileActor.h"
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

    GenerateGrid();
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
    
    if (!SolveWFC())
    {
        UE_LOG(LogTemp, Error, TEXT("WFC Generation failed"));
    }
    else
    {
        if (FallBackSeed.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("No Seed defined in FallBackSeed"));
            return ;
        }
        ClearGrid();
        int Index = RandomStream.RandRange(0,FallBackSeed.Num());
        RandomStream.Initialize(FallBackSeed[Index]);
        InitializeGrid();
        SolveWFC();
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
    if (bEnableBacktracking)
    {
        return SolveWFCWithBacktracking();
    }

    for (CurrentRetry = 0; CurrentRetry < MaxRetries; CurrentRetry++)
    {
        InitializeGrid();
        
        int32 Iterations = 0;
        bool bSuccess = true;
        
        while (Iterations < MaxIterations && bSuccess)
        {
            Iterations++;

            FIntPoint LowestEntropyPos = FindLowestEntropyCell();
            
            if (LowestEntropyPos.X == -1)
            {
                return true;
            }
            
            if (Grid[LowestEntropyPos.X][LowestEntropyPos.Y].Entropy == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Contradiction found at (%d, %d), retry %d"), 
                       LowestEntropyPos.X, LowestEntropyPos.Y, CurrentRetry + 1);
                bSuccess = false;
                break;
            }
            
            CollapseCell(LowestEntropyPos.X, LowestEntropyPos.Y);
            PropagateConstraints(LowestEntropyPos.X, LowestEntropyPos.Y);
        }
        
        if (!bSuccess)
        {
            RandomSeed += 1000;
            RandomStream.Initialize(RandomSeed);
            ClearGrid();
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("WFC failed after %d retries"), MaxRetries);
    return false;
}

bool AWaveFunctionCollapse::SolveWFCWithBacktracking()
{
    for (CurrentRetry = 0; CurrentRetry < MaxRetries; CurrentRetry++)
    {
        InitializeGrid();
        ClearSnapshots();
        
        int32 Iterations = 0;
        
        while (Iterations < MaxIterations)
        {
            Iterations++;

            FIntPoint LowestEntropyPos = FindLowestEntropyCell();
            
            if (LowestEntropyPos.X == -1)
            {
                UE_LOG(LogTemp, Log, TEXT("WFC completed successfully with backtracking"));

                // 连通性测试
                if (IsConnected())
                {
                    return true;
                }
                // 生成失败需要重试
                UE_LOG(LogTemp, Warning, TEXT("Cannot connect, restarting"));
                break;
            }
            
            if (Grid[LowestEntropyPos.X][LowestEntropyPos.Y].Entropy == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Contradiction at (%d, %d), attempting backtrack"), 
                       LowestEntropyPos.X, LowestEntropyPos.Y);
                
                if (RestoreSnapshot())
                {
                    continue;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cannot backtrack further, restarting"));
                    break;
                }
            }
            
            // 回溯用
            SaveSnapshot(LowestEntropyPos);
            
            CollapseCell(LowestEntropyPos.X, LowestEntropyPos.Y);
            PropagateConstraints(LowestEntropyPos.X, LowestEntropyPos.Y);
        }
        
        // 如果到达这里，说明需要重试
        RandomSeed += 1000;
        RandomStream.Initialize(RandomSeed);
        ClearGrid();
    }
    
    return false;
}

void AWaveFunctionCollapse::SaveSnapshot(const FIntPoint& LastCollapsedCell)
{
    // 限制快照数量
    if (Snapshots.Num() >= MaxBacktrackSteps)
    {
        Snapshots.RemoveAt(0);
    }
    
    FWFCSnapshot Snapshot;
    Snapshot.GridState = Grid;
    Snapshot.LastCollapsedCell = LastCollapsedCell;
    Snapshots.Add(Snapshot);
}

bool AWaveFunctionCollapse::RestoreSnapshot()
{
    if (Snapshots.Num() == 0)
    {
        return false;
    }
    
    FWFCSnapshot LastSnapshot = Snapshots.Last();
    Snapshots.RemoveAt(Snapshots.Num() - 1);
    
    Grid = LastSnapshot.GridState;
    
    FIntPoint ProblemCell = LastSnapshot.LastCollapsedCell;
    if (IsValidPosition(ProblemCell.X, ProblemCell.Y))
    {
        FWFCCell& Cell = Grid[ProblemCell.X][ProblemCell.Y];
        
        if (Cell.bCollapsed && !Cell.SelectedTile.IsEmpty())
        {
            Cell.PossibleTiles.Remove(Cell.SelectedTile);
            Cell.bCollapsed = false;
            Cell.SelectedTile = "";
            Cell.Entropy = Cell.PossibleTiles.Num();
        }
    }
    
    return true;
}

void AWaveFunctionCollapse::ClearSnapshots()
{
    Snapshots.Empty();
}

bool AWaveFunctionCollapse::IsConnected() const
{
    if (Grid.Num() == 0 || Grid[0].Num() == 0)
    {
        return false;
    }

    // BFS Check
    TSet<FIntPoint> Visited;
    TQueue<FGridLocation> Queue;

    for (int32 X = 0; X < GridWidth; ++X)
    {
        const FWFCCell& StartCell = Grid[X][0];
        if (StartCell.bCollapsed && TileMap.Contains(StartCell.SelectedTile))
        {
            const FWFCTile& Tile = TileMap[StartCell.SelectedTile];
            if (Tile.Y_negReachable)
            {
                Queue.Enqueue(FGridLocation(X, 0));
                Visited.Add(FIntPoint(X, 0));
            }
        }
    }

    while (!Queue.IsEmpty())
    {
        FGridLocation CurrentLocation;
        Queue.Dequeue(CurrentLocation);

        int32 CurrentX = CurrentLocation.X;
        int32 CurrentY = CurrentLocation.Y;

        const FWFCCell& CurrentCell = Grid[CurrentX][CurrentY];
        if (!CurrentCell.bCollapsed || !TileMap.Contains(CurrentCell.SelectedTile)) continue;

        const FWFCTile& CurrentTile = TileMap[CurrentCell.SelectedTile];
        
        if (CurrentY == GridHeight - 1 && CurrentTile.Y_posReachable)
        {
            return true;
        }
        
        TArray<FIntPoint> Neighbors = {
            FIntPoint(CurrentX, CurrentY + 1), // 上 (Y+)
            FIntPoint(CurrentX, CurrentY - 1), // 下 (Y-)
            FIntPoint(CurrentX - 1, CurrentY), // 右 (X-)
            FIntPoint(CurrentX + 1, CurrentY)  // 左 (X+)
        };
        
        for (int32 i = 0; i < Neighbors.Num(); ++i)
        {
            FIntPoint NeighborPos = Neighbors[i];

            if (IsValidPosition(NeighborPos.X, NeighborPos.Y) && !Visited.Contains(NeighborPos))
            {
                const FWFCCell& NeighborCell = Grid[NeighborPos.X][NeighborPos.Y];
                if (NeighborCell.bCollapsed && TileMap.Contains(NeighborCell.SelectedTile))
                {
                    const FWFCTile& NeighborTile = TileMap[NeighborCell.SelectedTile];
                    bool bCanConnect = false;

                    switch (i)
                    {
                        case 0: 
                            bCanConnect = CurrentTile.Y_posReachable && NeighborTile.Y_negReachable;
                            break;
                        case 1: 
                            bCanConnect = CurrentTile.Y_negReachable && NeighborTile.Y_posReachable;
                            break;
                        case 2: //
                            bCanConnect = CurrentTile.X_negReachable && NeighborTile.X_posReachable;
                            break;
                        case 3: 
                            bCanConnect = CurrentTile.X_posReachable && NeighborTile.X_negReachable;
                            break;
                    }

                    if (bCanConnect)
                    {
                        Visited.Add(NeighborPos);
                        Queue.Enqueue(FGridLocation(NeighborPos.X, NeighborPos.Y));
                    }
                }
            }
        }
    }
    
    return false;
}

// Fall Back 有错误
// void AWaveFunctionCollapse::HandleContradiction()
// {
//     for (int32 X = 0; X < GridWidth; X++)
//     {
//         for (int32 Y = 0; Y < GridHeight; Y++)
//         {
//             FWFCCell& Cell = Grid[X][Y];
//             if (!Cell.bCollapsed && Cell.Entropy == 0)
//             {
//                 if (bAllowFallbackTiles)
//                 {
//                     // Fallback最多使用 ，再Fallback第一个
//                     FString FallbackTile = GetFallbackTile(X, Y);
//                     if (!FallbackTile.IsEmpty())
//                     {
//                         Cell.PossibleTiles.Add(FallbackTile);
//                         Cell.Entropy = 1;
//                         UE_LOG(LogTemp, Warning, TEXT("Applied fallback tile %s to cell (%d, %d)"), 
//                                *FallbackTile, X, Y);
//                     }
//                 }
//             }
//         }
//     }
// }
//
// FString AWaveFunctionCollapse::GetFallbackTile(int32 X, int32 Y)
// {
//     TMap<FString, int32> TileFrequency;
//     
//     for (const FWFCTile& Tile : TileSet)
//     {
//         if (!Tile.TileID.IsEmpty())
//         {
//             TileFrequency.Add(Tile.TileID, 0);
//         }
//     }
//     
//     TArray<FIntPoint> Neighbors = {
//         FIntPoint(X +1 , Y ), FIntPoint(X , Y + 1),
//         FIntPoint(X -1, Y ), FIntPoint(X , Y -1)
//     };
//     
//     for (const FIntPoint& NeighborPos : Neighbors)
//     {
//         if (IsValidPosition(NeighborPos.X, NeighborPos.Y))
//         {
//             const FWFCCell& NeighborCell = Grid[NeighborPos.X][NeighborPos.Y];
//             if (NeighborCell.bCollapsed && !NeighborCell.SelectedTile.IsEmpty())
//             {
//                 if (TileFrequency.Contains(NeighborCell.SelectedTile))
//                 {
//                     TileFrequency[NeighborCell.SelectedTile]++;
//                 }
//             }
//         }
//     }
//     
//     FString BestTile = "";
//     int32 MaxFrequency = -1;
//     
//     for (const auto& Pair : TileFrequency)
//     {
//         if (Pair.Value > MaxFrequency)
//         {
//             MaxFrequency = Pair.Value;
//             BestTile = Pair.Key;
//         }
//     }
//     
//     if (BestTile.IsEmpty() && TileSet.Num() > 0)
//     {
//         BestTile = TileSet[0].TileID;
//     }
//     
//     return BestTile;
// }

// 检查所有所有规则可用
// void AWaveFunctionCollapse::ValidateTileConstraints()
// {
//     for (const FWFCTile& Tile : TileSet)
//     {
//         if (Tile.TileID.IsEmpty()) continue;
//
//         TArray<TArray<FString>*> NeighborArrays = {
//             const_cast<TArray<FString>*>(&Tile.YposNeighbors),
//             const_cast<TArray<FString>*>(&Tile.XnegNeighbors),
//             const_cast<TArray<FString>*>(&Tile.YnegNeighbors),
//             const_cast<TArray<FString>*>(&Tile.XposNeighbors)
//         };
//         
//         for (int32 Dir = 0; Dir < 4; Dir++)
//         {
//             for (const FString& NeighborID : *NeighborArrays[Dir])
//             {
//                 if (!TileMap.Contains(NeighborID))
//                 {
//                     UE_LOG(LogTemp, Warning, TEXT("Tile %s references non-existent neighbor %s in direction %d"), 
//                            *Tile.TileID, *NeighborID, Dir);
//                 }
//             }
//         }
//     }
// }

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
    if (Cell.bCollapsed)
    {
        return;
    }
    
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
    
    //UE_LOG(LogTemp, Log, TEXT("Collapsed cell (%d, %d) to %s"), X, Y, *SelectedTile);
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
            FIntPoint(X + 1 , Y     ),
            FIntPoint(X     , Y + 1 ), 
            FIntPoint(X - 1 , Y     ), 
            FIntPoint(X     , Y - 1 )
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
            return Tile.YposNeighbors.Contains(NeighborID);
        case 1: // 右
            return Tile.XnegNeighbors.Contains(NeighborID);
        case 2: // 下
            return Tile.YnegNeighbors.Contains(NeighborID);
        case 3: // 左
            return Tile.XposNeighbors.Contains(NeighborID);
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
    
    if (TileActorClass)
    {
   
        FVector Position = GetActorLocation() + FVector(X * TileSize, Y * TileSize, 0.0f);
        FRotator Rotation = GetActorRotation();
        // 生成Tile Actor

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AWFCTileActor* TileActor = GetWorld()->SpawnActor<AWFCTileActor>(TileActorClass,Position,Rotation,SpawnParams);
        // FVector Scale3D = FVector(TileSize / 100);
        // TileActor->SetActorScale3D(Scale3D);
        if (TileActor)
        {
            // 设置网格和ID
            TileActor->SetTileMesh(Tile.Mesh);
            TileActor->SetTileID(TileID);
            
            GeneratedTiles.Add(TileActor);
            //UE_LOG(LogTemp, Log, TEXT("Spawned tile %s at position (%d, %d)"), *TileID, X, Y);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn tile actor for %s"), *TileID);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TileActorClass not set"));
    }
}

void AWaveFunctionCollapse::ClearGeneratedMeshes()
{
    for (AWFCTileActor* TileActor : GeneratedTiles)
    {
        if (TileActor && IsValid(TileActor))
        {
            TileActor->Destroy();
        }
    }
    GeneratedTiles.Empty();
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