// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelDestruction/Voxelizer.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/InstancedStaticMeshComponent.h"
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
	CaptureComponent->ProjectionType = ECameraProjectionMode::Type::Orthographic;
	
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	

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

void AVoxelizer::Voxelize()
{
	VoxelCheckSet.Empty();
	const TArray<FVector> DirectionList = {
		{1,0,0},
		{-1,0,0},
		{0,1,0},
		{0,-1,0},
		{0,0,1},
		{0,0,-1}
	};

	for(auto Direction:DirectionList)
	{
		SetView(Direction);
		Sample();
	}
	BuildInstanceMesh();
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

	for (int i = 0;i<RawColorsArray.Num();++i)
	{
		auto RawColor = RawColorsArray[i];
		float Depth = RawColor.R;

		// 超出阈值则判断非对象，
		if (Depth > 6500.f)
			continue;

		int32 Size_X = CurrentRenderTarget->SizeX;
		int32 Size_Y = CurrentRenderTarget->SizeY;

		float RT_Space_X = i%Size_X;
		float RT_Space_Y = Size_Y - i/Size_X -1;

		FTransform ViewTransform = CaptureComponent->GetComponentTransform();
		FVector WorldLocation = RTSpaceToWorldSpace(Depth,RT_Space_X,RT_Space_Y,ViewTransform,CurrentRenderTarget);
		
		VoxelCheckSet.Add(WorldLocation);
	}
}

void AVoxelizer::BuildInstanceMesh()
{
	FVector SpawnTransform = VoxelizationTarget->GetActorLocation();
	AActor* SpawnedActor = GetWorld()->SpawnActor(
		ISM_Class->GetClass(),&SpawnTransform);


	if(!SpawnedActor)
	{
		UE_LOG(LogTemp,Error,TEXT("Spawn actor failed!"));
		return ;
	}
	
	UInstancedStaticMeshComponent* ISMComponent = 
		SpawnedActor->FindComponentByClass<UInstancedStaticMeshComponent>();

	if(!ISMComponent)
	{
		UE_LOG(LogTemp,Error,TEXT("Didn't find UInstancedStaticMeshComponent from %s"),*SpawnedActor->GetName());
		return;
	}

	TArray<FTransform> InstanceTransforms;
	for(auto Pos:VoxelCheckSet)
	{
		FTransform VoxelTransform;
		VoxelTransform.SetLocation(Pos - SpawnTransform);
		VoxelTransform.SetScale3D(FVector(VoxelSize));
		InstanceTransforms.Add(VoxelTransform);
	}

	ISMComponent->AddInstances(InstanceTransforms,false);
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

FVector AVoxelizer::RTSpaceToWorldSpace(const float Depth, const float X, const float Y, const FTransform ViewTransform,
	const UTextureRenderTarget2D* RenderTarget) const
{
	if(!RenderTarget)
	{
		UE_LOG(LogTemp,Error,TEXT("Render Target is NULL !"));
		return FVector::Zero();
	}
	float Size_X = RenderTarget->SizeX;
	float Size_Y = RenderTarget->SizeY;

	FVector WorldLocation = FVector(floor(Depth/VoxelSize),X-Size_X/2.0f,Y-Size_Y/2.0f);

	FTransform Transform;
	Transform.SetLocation(ViewTransform.GetLocation());
	Transform.SetRotation(ViewTransform.GetRotation());
	Transform.SetScale3D(FVector(VoxelSize));
	
	return UKismetMathLibrary::TransformLocation(Transform,WorldLocation+FVector(0.5f));
	//FVector ReturnValue = FVector::ZeroVector;
}

