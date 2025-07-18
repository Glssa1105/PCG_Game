#pragma once
#include "ObjectpoolComponent.generated.h"

class APooledActor;

UCLASS(meta = (BlueprintSpawnableComponent))
class UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UObjectPoolComponent();
	
	UFUNCTION(BlueprintCallable,Category = "Object Pool")
	APooledActor* GetPooledActor();	

	UFUNCTION(BlueprintCallable,Category = "Object Pool")
	void ReturnPooledActor(APooledActor* ActorToReturn);

	UFUNCTION(BlueprintCallable,Category = "Object Pool")
	void Init();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;  
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool")
	TSubclassOf<APooledActor> PooledActorClass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool", meta = (ClampMin = "1"))
	int32 MaxPoolSize = 500;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool", meta = (ClampMin = "0"))
	int32 InitSize = 100;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool", meta = (ClampMin = "1.0"))
	float RecycleAfterSeconds = 30.f;
	
private:
	// 目前会导致大量错误......
	void RecycleActor();

	// 外部销毁回调 防止错误
	UFUNCTION()
	void OnPooledActorDestroyed(AActor* DestroyedActor);
	
	// 池
	TArray<APooledActor*> AvailableActors;

	// 已用对象
	TDoubleLinkedList<int32> UsedActorIndexList;
	TMap<APooledActor*,TDoubleLinkedList<int32>::TDoubleLinkedListNode*> UsedActorIndexListMap;

	// 可用对象
	TArray<int32> UsableActorsIndexes;
};