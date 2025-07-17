// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelDestruction/Voxelizer.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "Rendering/RenderingCommon.h"
#include "GlobalShader.h"

// Sets default values
AVoxelizer::AVoxelizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComponent->ProjectionType = ECameraProjectionMode::Type::Orthographic;
	CaptureComponent->CaptureSource = SCS_SceneDepth;
	CaptureComponent->SetupAttachment(RootComponent);
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
	if (bIsStartVoxelize)
	{
		if (CheckReadbackComplete())
		{
			CompleteVoxelize();
			bIsStartVoxelize = false;
			UE_LOG(LogTemp, Log, TEXT("ReadBackFence Complete"));
		}
	}
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

	RenderTargets.SetNum(DirectionList.Num());
	RawColorsArrays.SetNum(DirectionList.Num());
	ViewTransforms.SetNum(DirectionList.Num());
	
	for(int32 DirectionIndex =0;DirectionIndex<DirectionList.Num();DirectionIndex++)
	{
		SetView(DirectionIndex,DirectionList[DirectionIndex]);
	}
	
	RenderTargetReadBack(true);
	for(int32 DirectionIndex =0;DirectionIndex<DirectionList.Num();DirectionIndex++)
	{
		Sample(DirectionIndex);
	}
	BuildInstanceMesh();
}

bool AVoxelizer::RenderTargetReadBack(bool bFlushImmediately)
{
	for (int32 Index = 0;Index < RenderTargets.Num();Index++)
	{
		UTextureRenderTarget2D* TextureRenderTarget = RenderTargets[Index];
		TArray<FLinearColor>& OutLinearSamples = RawColorsArrays[Index];
		if (TextureRenderTarget != nullptr)
		{
			const int32 NumSamples = TextureRenderTarget->SizeX * TextureRenderTarget->SizeY;
			OutLinearSamples.Reset(NumSamples);
		
			FIntRect InSrcRect(0,0,TextureRenderTarget->SizeX,TextureRenderTarget->SizeY);
			FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_MinMax);
			FRenderTarget* RenderTarget = TextureRenderTarget->GameThread_GetRenderTargetResource();
		
			OutLinearSamples.Reset();
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
				[RenderTarget_RT = RenderTarget,SrcRect_RT = InSrcRect,OutData_RT = &OutLinearSamples,Flag_RT=InFlags](FRHICommandListImmediate& RHICmdList)
				{
					RHICmdList.ReadSurfaceData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flag_RT);	
				});
			if (bFlushImmediately)
			{
				FlushRenderingCommands();
				if (OutLinearSamples.Num() == 0)
				{
					UE_LOG(LogTemp,Error,TEXT("GPU Resource readback failed!"));
					return false;
				}
			}
		}
	}
	if (!bFlushImmediately)
	{
		ReadBackFence.BeginFence();
		UE_LOG(LogTemp, Display, TEXT("Start ReadBackFence"));
	}
	
	return true;
}

void AVoxelizer::SetView(const int32 DirectionIndex,const FVector& SampleDirection)
{
	FVector TargetOrigin = FVector::ZeroVector;
	FVector TargetBoxExtent = FVector::ZeroVector;
	VoxelizationTarget->GetActorBounds(false,TargetOrigin,TargetBoxExtent);
	FVector SnappedExtent = SnapExtentToVoxelSize(TargetBoxExtent);

	FVector NewLocation = FVector::ZeroVector;
	NewLocation = TargetOrigin - SnappedExtent*SampleDirection;

	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation,TargetOrigin);

	CaptureComponent->SetWorldLocationAndRotation(NewLocation,NewRotation);
	ViewTransforms[DirectionIndex] = CaptureComponent->GetComponentTransform();

	FVector BoundSizeInView = SnappedExtent * 2;
	FVector RotatedBoundSize = NewRotation.RotateVector(BoundSizeInView);
	FVector AbsRotatedBoundSize = RotatedBoundSize.GetAbs();
	
	FVector StandardRotatedBoundSize = FVector(round(AbsRotatedBoundSize.X),round(AbsRotatedBoundSize.Y),round(AbsRotatedBoundSize.Z));

	CaptureComponent->OrthoWidth = StandardRotatedBoundSize.Y;

	float RT_Width = ceil(StandardRotatedBoundSize.Y/VoxelSize);
	float RT_Height = ceil(StandardRotatedBoundSize.Z/VoxelSize);

	RenderTargets[DirectionIndex] = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(),RT_Width,RT_Height);
	CaptureComponent->TextureTarget = RenderTargets[DirectionIndex];
	
	CaptureComponent->ClearShowOnlyComponents();
	CaptureComponent->ShowOnlyActorComponents(VoxelizationTarget);

	// 可能 Deferred？
	CaptureComponent->CaptureScene();
	
	// CHECK 1
	//UKismetSystemLibrary::PrintString(GetWorld(),"Origin" +TargetOrigin.ToString() + "Extent" + TargetBoxExtent.ToString(),true,true,FLinearColor(0,0.66,1),5);
	// CHECK 2
	//UKismetSystemLibrary::PrintString(GetWorld(),SnappedExtent.ToString(),true,true,FLinearColor(0,0.66,1),5);
	// CHECK 3
	// FString OutString = "Ortho Width:" + FString::SanitizeFloat(StandardRotatedBoundSize.Y) + "RenderTarget Width:" + FString::SanitizeFloat(RT_Width) +"RenderTarget Height"+ FString::SanitizeFloat(RT_Height);
	// UKismetSystemLibrary::PrintString(GetWorld(),OutString,true,true,FLinearColor(0,0.66,1),5);
}

void AVoxelizer::Sample(int32 DirectionIndex)
{
	TArray<FLinearColor> RawColorsArray = RawColorsArrays[DirectionIndex];
	UTextureRenderTarget2D* CurrentRT = RenderTargets[DirectionIndex];
	//UKismetRenderingLibrary::ReadRenderTargetRaw(GetWorld(),CurrentRenderTarget,RawColorsArray,false);
	//RenderTargetReadBack(RenderTarget[DirectionIndex],RawColorsArray);
	
	for (int i = 0;i<RawColorsArray.Num();++i)
	{
		auto RawColor = RawColorsArray[i];
		float Depth = RawColor.R;

		// 超出阈值则判断非对象，
		if (Depth > 6500.f)
			continue;

		int32 Size_X = CurrentRT->SizeX;
		int32 Size_Y = CurrentRT->SizeY;

		float RT_Space_X = i%Size_X;
		float RT_Space_Y = Size_Y - i/Size_X -1;

		FTransform& ViewTransform = ViewTransforms[DirectionIndex];
		FVector WorldLocation = RTSpaceToWorldSpace(Depth,RT_Space_X,RT_Space_Y,ViewTransform,CurrentRT);
		
		VoxelCheckSet.Add(WorldLocation);
	}
}

void AVoxelizer::BuildInstanceMesh()
{
	FVector SpawnTransform = VoxelizationTarget->GetActorLocation();
	AActor* SpawnedActor = GetWorld()->SpawnActor(
		ISM_Class,&SpawnTransform);
	
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

void AVoxelizer::StartVoxelize()
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

	RenderTargets.Empty();
	RawColorsArrays.Empty();
	ViewTransforms.Empty();
	
	RenderTargets.SetNum(DirectionList.Num());
	RawColorsArrays.SetNum(DirectionList.Num());
	ViewTransforms.SetNum(DirectionList.Num());
	
	for(int32 DirectionIndex =0;DirectionIndex<DirectionList.Num();DirectionIndex++)
	{
		SetView(DirectionIndex,DirectionList[DirectionIndex]);
	}
	
	RenderTargetReadBack(false);
	bIsStartVoxelize = true;
}

void AVoxelizer::CompleteVoxelize()
{
	// Hardcode 6 Directions
	for(int32 DirectionIndex =0;DirectionIndex<6;DirectionIndex++)
	{
		Sample(DirectionIndex);
	}
	BuildInstanceMesh();
}

bool AVoxelizer::CheckReadbackComplete()
{
	return ReadBackFence.IsFenceComplete();
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
}

void AVoxelizer::SetTarget(AActor* NewTarget)
{
	VoxelizationTarget = NewTarget;
}

