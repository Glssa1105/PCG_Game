// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EGridSlot : uint8
{
	Valid = 0,
	Invalid = 1,
};

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid.generated.h"
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

	bool IsXForwardValid() const {return this->X_Forward == EGridSlot::Valid;}
	bool IsXBackwardValid() const {return this->X_Backward == EGridSlot::Valid;}
	bool IsYForwardValid() const {return this->Y_Forward == EGridSlot::Valid;}
	bool IsYBackwardValid() const {return this->Y_Backward == EGridSlot::Valid;}
	//bool IsZForwardValid() const {return this->Z_Forward == EGridSlot::Valid;}
	//bool IsZBackwardValid() const {return this->Z_Backward == EGridSlot::Valid;}
};
