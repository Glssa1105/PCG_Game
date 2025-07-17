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
	
	// 当Actor从对象池中取出时调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooled Actor")
	void OnPulledFromPool();
	virtual void OnPulledFromPool_Implementation();

	// 当Actor返回对象池时调用 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooled Actor")
	void OnReturnedToPool();
	virtual void OnReturnedToPool_Implementation();

	// 当Actor被重使用时调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooled Actor")
	void OnReuse();
	virtual void OnReuse_Implementation();
	
	UFUNCTION(BlueprintCallable, Category = "Pooled Actor")
	float GetLastUseTime();

	
private:
	float LastUsedTime;
};