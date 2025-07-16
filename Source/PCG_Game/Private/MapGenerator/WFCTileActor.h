#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "WFCTileActor.generated.h"

UCLASS()
class PCG_GAME_API AWFCTileActor : public AActor
{
    GENERATED_BODY()

public:
    AWFCTileActor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

public:
    void SetTileMesh(UStaticMesh* Mesh);
    void SetTileID(const FString& TileID);
    
private:
    FString TileID;
};