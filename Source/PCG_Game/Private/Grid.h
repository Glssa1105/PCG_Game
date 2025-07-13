// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid.generated.h"


UENUM(BlueprintType,Meta = (Bitflags))
enum class EGridSlot : uint8
{
	// 允许为空
	AllowEmpty = 0,
	// 允许有物体
	Allow = 1 << 0,
	// 允许全连接
	FullConnect = 1 << 1
};

UCLASS()
class AGrid : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Grid")
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Slot")
	EGridSlot X_Forward;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Slot")
	EGridSlot Y_Forward;
	// UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Slot")
	// EGridSlot Z_Forward;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Slot")
	EGridSlot X_Backward;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Slot")
	EGridSlot Y_Backward;
	// UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Slot")
	// EGridSlot Z_Backward;

	EGridSlot GetXForwardSlot() const {return this->X_Forward;}
	EGridSlot GetXBackwardSlot() const {return this->X_Forward;}
	EGridSlot GetYForwardSlot() const {return this->X_Forward;}
	EGridSlot GetYBackwardSlot() const {return this->X_Forward;}
	// EGridSlot GetXForwardSlot() const {return this->X_Forward;}
	// EGridSlot GetXForwardSlot() const {return this->X_Forward;}

};
