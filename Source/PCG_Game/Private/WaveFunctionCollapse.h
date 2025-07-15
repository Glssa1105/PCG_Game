#pragma once

#include "CoreMinimal.h"
#include "WFCTileActor.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "WaveFunctionCollapse.generated.h"

USTRUCT(BlueprintType)
struct FWFCTile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TileID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UpNeighbors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RightNeighbors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DownNeighbors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> LeftNeighbors;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight = 1.0f;

    FWFCTile()
    {
        Mesh = nullptr;
        TileID = "";
        Weight = 1.0f;
    }
};

USTRUCT()
struct FWFCCell
{
    GENERATED_BODY()

    TArray<FString> PossibleTiles;
    bool bCollapsed = false;
    FString SelectedTile = "";
    
    int32 Entropy = 0;

    FWFCCell()
    {
        bCollapsed = false;
        SelectedTile = "";
        Entropy = 0;
    }
};

USTRUCT()
struct FWFCSnapshot
{
    GENERATED_BODY()

    TArray<TArray<FWFCCell>> GridState;
    FIntPoint LastCollapsedCell;
    
    FWFCSnapshot()
    {
        LastCollapsedCell = FIntPoint(-1, -1);
    }
};

UCLASS()
class PCG_GAME_API AWaveFunctionCollapse : public AActor
{
    GENERATED_BODY()

public:
    AWaveFunctionCollapse();

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 GridWidth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 GridHeight = 10;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    float TileSize = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    TArray<FWFCTile> TileSet;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 RandomSeed = 5201314;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 MaxIterations = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    TSubclassOf<AWFCTileActor> TileActorClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    bool bEnableBacktracking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 MaxBacktrackSteps = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 MaxRetries = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    bool bAllowFallbackTiles = true;


private:
    TArray<TArray<FWFCCell>> Grid;

    TMap<FString, FWFCTile> TileMap;

    TArray<AWFCTileActor*> GeneratedTiles;

    FRandomStream RandomStream;
    TArray<FWFCSnapshot> Snapshots;
    int32 CurrentRetry = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "WFC")
    void GenerateGrid();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void ClearGrid();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void SetSeed(int32 NewSeed);

private:
    void InitializeGrid();
    bool SolveWFC();
    bool SolveWFCWithBacktracking();
    
    FIntPoint FindLowestEntropyCell();
    void CollapseCell(int32 X, int32 Y);
    void PropagateConstraints(int32 X, int32 Y);
    void UpdateCellPossibilities(int32 X, int32 Y);
    bool IsValidNeighbor(const FString& TileID, const FString& NeighborID, int32 Direction);
    void SpawnTileAtPosition(int32 X, int32 Y, const FString& TileID);
    void ClearGeneratedMeshes();
    
    bool IsValidPosition(int32 X, int32 Y) const;
    FString SelectRandomTile(const TArray<FString>& PossibleTiles);
    
    void SaveSnapshot(const FIntPoint& LastCollapsedCell);
    bool RestoreSnapshot();
    void ClearSnapshots();
    
    bool HasContradiction() const;
    void HandleContradiction();
    FString GetFallbackTile(int32 X, int32 Y);
    void ValidateTileConstraints();
    
};