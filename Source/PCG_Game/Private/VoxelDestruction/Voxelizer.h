// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voxelizer.generated.h"

UCLASS()
class AVoxelizer : public AActor
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

	// 非异步 C++ 实现
	UFUNCTION(BlueprintCallable,CallInEditor)
	void Voxelize();
	
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
	//TArray<TObjectPtr<UTextureRenderTarget2D>> CurrentRenderTarget;
	UPROPERTY(VisibleAnywhere,Category= "Voxelization")
	UTextureRenderTarget2D* CurrentRenderTarget;
	
	UPROPERTY(VisibleAnywhere,Category= "Voxelization")
	USceneCaptureComponent2D* CaptureComponent;
	
	
	TSet<FVector> VoxelCheckSet;
	
	// 需要异步回读 RenderTarget 否则性能极低
	// 非异步 C++ 实现
	void SetView(const FVector& SampleDirection);
	void Sample();
	void BuildInstanceMesh();

	FVector SnapExtentToVoxelSize(const FVector& Extent) const;
	FVector RTSpaceToWorldSpace(const float Depth,const float X,const float Y,const FTransform ViewTransform,const UTextureRenderTarget2D* RenderTarget) const;
};

inline void AVoxelizer::SetTarget(AActor* NewTarget)
{
	VoxelizationTarget = NewTarget;
}
