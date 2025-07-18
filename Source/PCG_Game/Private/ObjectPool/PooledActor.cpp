#include "PooledActor.h"

APooledActor::APooledActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

APooledActor::~APooledActor()
{
}

void APooledActor::OnPulledFromPool_Implementation()
{
	SetActorLocation(FVector::Zero());
	EnableAllPhysics();
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	LastUsedTime = GetWorld()->GetTimeSeconds();
}

void APooledActor::OnReturnedToPool_Implementation()
{
	SetActorLocation(FVector::Zero());
	DisableAllPhysics();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void APooledActor::OnReuse_Implementation()
{
	LastUsedTime = GetWorld()->GetTimeSeconds();
}

float APooledActor::GetLastUseTime() const
{
	return LastUsedTime;
}

void APooledActor::DisableAllPhysics()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);
	for(UPrimitiveComponent* Component:PrimitiveComponents)
	{
		if(Component && Component->IsSimulatingPhysics())
		{
			Component->SetSimulatePhysics(false);
		}
	}
}

void APooledActor::EnableAllPhysics()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);
	for(UPrimitiveComponent* Component:PrimitiveComponents)
	{
		if(Component)
		{
			Component->SetSimulatePhysics(true);
		}
	}
}
