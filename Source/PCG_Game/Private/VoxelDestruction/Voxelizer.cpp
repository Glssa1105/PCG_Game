// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelDestruction/Voxelizer.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"

// Sets default values
AVoxelizer::AVoxelizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("SceneCaptureComponent2D");
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

// Called when the game starts or when spawned
void AVoxelizer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVoxelizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVoxelizer::SetView(const FVector& SampleDirection)
{
	FVector TargetOrigin = FVector::ZeroVector;
	FVector TargetBoxExtent = FVector::ZeroVector;
	GetActorBounds(false,TargetOrigin,TargetBoxExtent);
	FVector SnappedExtent = SnapExtentToVoxelSize(TargetBoxExtent);

	FVector NewLocation = FVector::ZeroVector;
	NewLocation = TargetOrigin - SnappedExtent*SampleDirection;

	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation,TargetOrigin);

	CaptureComponent->SetWorldLocationAndRotation(NewLocation,NewRotation);

	FVector BoundSizeInView = SnappedExtent * 2;
	FVector RotatedBoundSize = NewRotation.RotateVector(BoundSizeInView);
	FVector AbsRotatedBoundSize = RotatedBoundSize.GetAbs();
	
	FVector StandardRotatedBoundSize = FVector(round(AbsRotatedBoundSize.X),round(AbsRotatedBoundSize.Y),round(AbsRotatedBoundSize.Z));

	CaptureComponent->OrthoWidth = StandardRotatedBoundSize.Y;

	float RT_Width = ceil(StandardRotatedBoundSize.Y/VoxelSize);
	float RT_Height = ceil(StandardRotatedBoundSize.Z/VoxelSize);

	CurrentRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(),RT_Width,RT_Height);
	CaptureComponent->TextureTarget = CurrentRenderTarget;

	CaptureComponent->ShowOnlyActorComponents(VoxelizationTarget);

	// 可能 Deferred？
	CaptureComponent->CaptureScene();
}

void AVoxelizer::Sample()
{
	TArray<FLinearColor> RawColorsArray;
	UKismetRenderingLibrary::ReadRenderTargetRaw(GetWorld(),CurrentRenderTarget,RawColorsArray,false);

	for (auto RawColor : RawColorsArray)
	{
		float Depth = RawColor.R;

		// 超出阈值则判断非对象，
		if (Depth > 6500.f)
			continue;

		float Size_X = CurrentRenderTarget->SizeX;
		float Size_Y = CurrentRenderTarget->SizeY;

		
	}
}

FVector AVoxelizer::SnapExtentToVoxelSize(const FVector& Extent) const
{
	FVector SnapExtent = FVector::ZeroVector;

	FVector Scale = Extent / VoxelSize;
	SnapExtent.X = ceil(Scale.X)*VoxelSize;
	SnapExtent.Y = ceil(Scale.Y)*VoxelSize;
	SnapExtent.Z = ceil(Scale.Z)*VoxelSize;
	return SnapExtent;
}

// FVector AVoxelizer::RTSpaceToWorldSpace(const float Depth, const float X, const float Y, FTransform ViewTransform,
// 	USceneCaptureComponent2D RenderTarget)
// {
// 	//FVector ReturnValue = FVector::ZeroVector;
// 	
// }

