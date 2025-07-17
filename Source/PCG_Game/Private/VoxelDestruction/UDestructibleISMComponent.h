// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "UDestructibleISMComponent.generated.h"

UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), Blueprintable)
class PCG_GAME_API UDestructibleISMComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Static Mesh")
	TObjectPtr<UStaticMesh> GenerateMesh = nullptr;
	
	UFUNCTION(BlueprintCallable)
	TArray<AActor*> RemoveInstancesOverlappingSphere(const FVector& Center, float Radius, bool bSphereInWorldSpace = true);

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> RemoveAllInstances();

protected:
	virtual void BeginPlay() override;

	// TEST
private:
	UPROPERTY()
	UObjectPoolComponent* VoxelPoolComponent;
};
