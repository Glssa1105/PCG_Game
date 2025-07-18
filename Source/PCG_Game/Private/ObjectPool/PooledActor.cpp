#include "PooledActor.h"

APooledActor::APooledActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APooledActor::OnPulledFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	LastUsedTime = GetWorld()->GetTimeSeconds();
}

void APooledActor::OnReturnedToPool_Implementation()
{
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