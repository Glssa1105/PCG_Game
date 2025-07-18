#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PooledActor.generated.h"

UCLASS()
class PCG_GAME_API APooledActor : public AActor
{
	GENERATED_BODY()
public:
	APooledActor();
	~APooledActor();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooled Actor")
	void OnPulledFromPool();
	virtual void OnPulledFromPool_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooled Actor")
	void OnReturnedToPool();
	virtual void OnReturnedToPool_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooled Actor")
	void OnReuse();
	virtual void OnReuse_Implementation();
	
	UFUNCTION(BlueprintCallable, Category = "Pooled Actor")
	float GetLastUseTime() const;

private:
	void DisableAllPhysics();
	void EnableAllPhysics();
	float LastUsedTime = 0.f;
};