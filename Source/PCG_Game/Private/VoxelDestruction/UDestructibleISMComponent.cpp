// Fill out your copyright notice in the Description page of Project Settings.


#include "UDestructibleISMComponent.h"

#include "Voxelizer.h"
#include "Kismet/GameplayStatics.h"
#include "ObjectPool/PooledActor.h"
#include "ObjectPool/ObjectPoolComponent.h"

TArray<AActor*> UDestructibleISMComponent::RemoveInstancesOverlappingSphere(const FVector& Center, float Radius, bool bSphereInWorldSpace)
{
	TArray<AActor*> SpawnedActors;

	const TArray<int32> OverlappingInstancesIndices = GetInstancesOverlappingSphere(Center, Radius, bSphereInWorldSpace);

	if (OverlappingInstancesIndices.IsEmpty())
	{
		return SpawnedActors;
	}

	for (const int32 Index : OverlappingInstancesIndices)
	{
		FTransform InstanceTransform;
		if (GetInstanceTransform(Index, InstanceTransform, true)) // bWorldSpace = true
		{
			if (VoxelPoolComponent)
			{
				APooledActor* NewActor = VoxelPoolComponent->GetPooledActor();
				if (NewActor)
				{
					NewActor->SetActorTransform(InstanceTransform);
					SpawnedActors.Add(NewActor);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to get a pooled actor from VoxelPoolComponent."));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("VoxelPoolComponent is null."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not get instance transform for index: %d"), Index);
		}
	}
	RemoveInstances(OverlappingInstancesIndices);

	return SpawnedActors;
}

TArray<AActor*> UDestructibleISMComponent::RemoveAllInstances()
{
	TArray<AActor*> SpawnedActors;
	int32 Count = GetInstanceCount();
	
	for (int32 Index = 0; Index < Count; Index++)
	{
		FTransform Transform = FTransform::Identity;
		if (!GetInstanceTransform(Index, Transform, true))
		{
			UE_LOG(LogTemp, Error, TEXT("Trying to get a transform at an invalid index"));
			continue;
		}
		// Use Object Pool
		if(GetWorld())
		{
			APooledActor* NewActor = VoxelPoolComponent->GetPooledActor();
			if (!NewActor)
			{
				UE_LOG(LogTemp, Error, TEXT("Trying to get a pooledActor failed"));
				break;
			}
			NewActor->SetActorTransform(Transform);
			SpawnedActors.Add(NewActor);
		}
	}
	ClearInstances();
	return SpawnedActors;
}

void UDestructibleISMComponent::BeginPlay()
{
	Super::BeginPlay();

// FOR TEST
	auto TestActor = UGameplayStatics::GetActorOfClass(GetWorld(),AVoxelizer::StaticClass());
	VoxelPoolComponent = TestActor->FindComponentByClass<UObjectPoolComponent>();
}
