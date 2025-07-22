// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelizableComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_GAME_API UVoxelizableComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UVoxelizableComponent();
};
