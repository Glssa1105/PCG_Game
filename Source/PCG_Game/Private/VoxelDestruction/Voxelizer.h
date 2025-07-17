// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voxelizer.generated.h"

UCLASS()
class PCG_GAME_API AVoxelizer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* NewTarget);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Voxelization")
	AActor* VoxelizationTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Voxelization",meta = (Units = "cm"))
	float VoxelSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Voxelization")
	UStaticMesh* VoxelStaticMesh;

	UPROPERTY(EditAnywhere,Blueprintable,Category="Voxelization")
	UClass* ISM_Class;

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* DefaultSceneRoot;
	
private:
	UPROPERTY()
	TArray<UTextureRenderTarget2D*> RenderTargets;
	TArray<TArray<FLinearColor>> RawColorsArrays;
	TArray<FTransform> ViewTransforms;
	
	UPROPERTY(VisibleAnywhere,Category= "Voxelization")
	USceneCaptureComponent2D* CaptureComponent;

private:
	// 做缓存机制
	TMap<UStaticMesh*,TArray<FVector>> VoxelizationCache;
	TSet<FVector> VoxelCheckSet;

	// 非异步 C++ 实现
public:
	UFUNCTION(BlueprintCallable,CallInEditor)
	void Voxelize();
private:
	// 需要异步回读 RenderTarget 否则性能极低
	bool RenderTargetReadBack(bool bFlushImmediately);
	void SetView(const int32 DirectionIndex,const FVector& SampleDirection);
	void Sample(int32 DirectionIndex);
	void BuildInstanceMesh();

	// 异步 C++ 实现
public:
	UFUNCTION(BlueprintCallable,CallInEditor)
	void StartVoxelize();
private:
	FRenderCommandFence ReadBackFence;
	bool bIsStartVoxelize = false;
	
	void CompleteVoxelize();
	bool CheckReadbackComplete();
	
	
private:
	// Help Function
	FVector SnapExtentToVoxelSize(const FVector& Extent) const;
	FVector RTSpaceToWorldSpace(const float Depth,const float X,const float Y,const FTransform ViewTransform,const UTextureRenderTarget2D* RenderTarget) const;
};
