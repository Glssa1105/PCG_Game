// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid.generated.h"


#define HAS_BIT(Bitmask,Bit)  (((Bitmask) & static_cast<uint8>(Bit)) == static_cast<uint8>(Bit))
#define SET_BIT(Bitmask,Bit) (Bitmask |= static_cast<uint8>(Bit))
#define CLEAR_BIT(Bitmask,Bit) ((Bitmask) &= static_cast<uint8>(~(Bit)))

UENUM(BlueprintType,Meta = (Bitflags,UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EGridSlot : uint8
{
	Null				= 0			UMETA(Hidden),
	// 空槽
	Empty				= 1 << 0	UMETA(DisplayName = "空槽"),
	// 允许非全连接
	UnFullConnectSlot	= 1 << 1	UMETA(DisplayName = "非全槽"),
	// 允许全连接
	FullConnectSlot		= 1 << 2    UMETA(DisplayName = "全连接槽"),
};



ENUM_CLASS_FLAGS(EGridSlot);

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 X_Forward_Accept;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 Y_Forward_Accept;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 X_Backward_Accept;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 Y_Backward_Accept;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 X_Forward_Self;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 Y_Forward_Self;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 X_Backward_Self;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum = "/Script/PCG_Game.EGridSlot"), Category="Grid Slot Accept")
	int32 Y_Backward_Self;

	
	
	int32 GetDirectionSelfBitmask(const int32 GridRotation,const int32 DirectionIndex) const;

	int32 GetDirectionAcceptBitmask(const int32 GridRotation,const int32 DirectionIndex) const;
	
	bool CheckConnectionValid(const int32 GridRotation,const int32 DirectionIndex, const int32 TargetSelfBitmask,const int32 TargetAcceptBitmask) const;
};
