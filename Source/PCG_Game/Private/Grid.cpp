// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid.h"

// Sets default values
AGrid::AGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	if (StaticMeshComponent->GetStaticMesh())
    {
        FBox Bounds = StaticMeshComponent->GetStaticMesh()->GetBoundingBox();
        StaticMeshComponent->SetRelativeLocation(-Bounds.GetCenter());
    }
}

// Called when the game starts or when spawned
void AGrid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int32 AGrid::GetDirectionSelfBitmask(const int32 GridRotation,const int32 DirectionIndex) const 
{
	switch ((GridRotation + DirectionIndex)%4)
	{
		case 0:
		    return X_Forward_Self;
			break;
		case 1:
			return Y_Forward_Self;
			break;
		case 2:
			return X_Backward_Self;
			break;
		case 3:
			return Y_Backward_Self;
			break;
		default:
			return 0;
	}
}

int32 AGrid::GetDirectionAcceptBitmask(const int32 GridRotation, const int32 DirectionIndex) const
{
	switch ((GridRotation + DirectionIndex)%4)
	{
	case 0:
		return X_Forward_Accept;
		break;
	case 1:
		return Y_Forward_Accept;
		break;
	case 2:
		return X_Backward_Accept;
		break;
	case 3:
		return Y_Backward_Accept;
		break;
	default:
		return 0;
	}
}

// 当 Self 为 Accept 子集时通过，双向检查
bool AGrid::CheckConnectionValid(const int32 GridRotation, const int32 DirectionIndex, const int32 TargetAcceptBitmask,const int32 TargetSelfBitmask) const
{
	int32 SelfBitmask = GetDirectionSelfBitmask(GridRotation,DirectionIndex);
	int32 AcceptBitmask = GetDirectionAcceptBitmask(GridRotation,DirectionIndex);
	
	auto IsSubsetOf = [](int32 maskA , int32 maskB)
	{
		uint32 uMaskA = static_cast<uint32>(maskA);
		uint32 uMaskB = static_cast<uint32>(maskB);
		return (uMaskA & uMaskB) == uMaskA;
	};
	
	return IsSubsetOf(SelfBitmask,TargetAcceptBitmask) &&
		IsSubsetOf(TargetSelfBitmask,AcceptBitmask);
}



