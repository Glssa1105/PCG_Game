// Fill out your copyright notice in the Description page of Project Settings.


#include "UDestructibleISMComponent.h"

#include "PooledActor.h"
#include "Voxelizer.h"
#include "Kismet/GameplayStatics.h"

TArray<AActor*> UDestructibleISMComponent::RemoveInstancesOverlappingSphere(const FVector& Center, float Radius,
                                                                            bool bSphereInWorldSpace)
{
	TArray<AActor*> SpawnedActors;
	TArray<int32> RemoveInstancesIndexes = GetInstancesOverlappingSphere(Center, Radius, bSphereInWorldSpace);
    
	for (int32 Index : RemoveInstancesIndexes)
	{
		FTransform Transform = FTransform::Identity;
		if (!GetInstanceTransform(Index, Transform, true))
		{
			UE_LOG(LogTemp, Error, TEXT("Trying to get a transform at an invalid index"));
			continue;
		}

		auto Mesh = GenerateMesh;
		
		if (Mesh && GetWorld())
		{
			FActorSpawnParameters SpawnParameters;
			AActor* NewActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Transform);
			if (NewActor)
			{
				UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(NewActor);
				if (MeshComponent)
				{
					MeshComponent->SetStaticMesh(Mesh);
					MeshComponent->SetWorldTransform(Transform);
             
					MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					MeshComponent->SetSimulatePhysics(true);
					
					NewActor->SetRootComponent(MeshComponent);
					MeshComponent->RegisterComponent();
             
					SpawnedActors.Add(NewActor);
				}
			}
		} 
	}
    
	if (RemoveInstancesIndexes.Num() && !RemoveInstances(RemoveInstancesIndexes))
	{
		for (auto Index : RemoveInstancesIndexes)
		{
			UE_LOG(LogTemp, Log, TEXT("Try to remove index:%d"),Index);
		}
		UE_LOG(LogTemp, Error, TEXT("Remove instances failed!"));
	}
    
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
		//
		// auto Mesh = GenerateMesh;
		//
		// if (Mesh && GetWorld())
		// {
		// 	FActorSpawnParameters SpawnParameters;
		// 	AActor* NewActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Transform);
		// 	if (NewActor)
		// 	{
		// 		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(NewActor);
		// 		if (MeshComponent)
		// 		{
		// 			MeshComponent->SetStaticMesh(Mesh);
		// 			MeshComponent->SetWorldTransform(Transform);
		//            
		// 			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 			MeshComponent->SetSimulatePhysics(true);
		// 			
		// 			NewActor->SetRootComponent(MeshComponent);
		// 			MeshComponent->RegisterComponent();
		// 			SpawnedActors.Add(NewActor);
		// 		}
		// 	}
		// } 
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
