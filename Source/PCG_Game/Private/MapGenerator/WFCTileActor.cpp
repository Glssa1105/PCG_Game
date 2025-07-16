#include "WFCTileActor.h"

AWFCTileActor::AWFCTileActor()
{
	PrimaryActorTick.bCanEverTick = false;
    
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCastShadow(true);
	MeshComponent->bReceivesDecals = true;
}

void AWFCTileActor::SetTileMesh(UStaticMesh* Mesh)
{
	if (MeshComponent && Mesh)
	{
		MeshComponent->SetStaticMesh(Mesh);
	}
}

void AWFCTileActor::SetTileID(const FString& InTileID)
{
	TileID = InTileID;
}
