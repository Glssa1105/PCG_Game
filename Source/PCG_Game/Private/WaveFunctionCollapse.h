#pragma once

#include "CoreMinimal.h"
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
    int32 RandomSeed = 12345;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Settings")
    int32 MaxIterations = 1000;

private:
    TArray<TArray<FWFCCell>> Grid;

    TMap<FString, FWFCTile> TileMap;

    TArray<UStaticMeshComponent*> GeneratedMeshes;

    FRandomStream RandomStream;

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
    FIntPoint FindLowestEntropyCell();
    void CollapseCell(int32 X, int32 Y);
    void PropagateConstraints(int32 X, int32 Y);
    void UpdateCellPossibilities(int32 X, int32 Y);
    bool IsValidNeighbor(const FString& TileID, const FString& NeighborID, int32 Direction);
    void SpawnTileAtPosition(int32 X, int32 Y, const FString& TileID);
    void ClearGeneratedMeshes();
    
    bool IsValidPosition(int32 X, int32 Y) const;
    int32 GetDirection(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const;
    FString SelectRandomTile(const TArray<FString>& PossibleTiles);
    void CalculateEntropy(int32 X, int32 Y);
};