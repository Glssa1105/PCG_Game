#pragma once
#include "ObjectpoolComponent.generated.h"

class APooledActor;

UCLASS(meta = (BlueprintSpawnableComponent))
class UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UObjectPoolComponent();

	// 从池中获取 Actor 若没有可用：1.池未满，创建填入并返回；2.若满，销毁使用时间最长的Actor，生成新Actor替代
	UFUNCTION(BlueprintCallable,Category = "Object Pool")
	APooledActor* GetPooledActor();	

	// 将不需使用的 Actor 重新放入池内
	UFUNCTION(BlueprintCallable,Category = "Object Pool")
	void ReturnPooledActor(APooledActor* ActorToReturn);

	UFUNCTION(BlueprintCallable,Category = "Object Pool")
	void Init();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;  
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	// 需要池化管理的 Actor 类
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool")
	TSubclassOf<APooledActor> PooledActorClass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool", meta = (ClampMin = "1"))
	int32 MaxPoolSize = 500;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool", meta = (ClampMin = "1"))
	int32 InitSize = 100;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Object Pool", meta = (ClampMin = "1"))
	float RecycleAfterSeconds = 30;
	
private:
	
	void RecycleActor();
	
	// 池
	UPROPERTY()
	TArray<APooledActor*> AvailableActors;

	// 已用对象下标队列，需要汰换时选用最前者
	TDoubleLinkedList<int32> UsedActorIndexList;
	TMap<APooledActor*,TDoubleLinkedList<int32>::TDoubleLinkedListNode*> UsedActorIndexListMap;

	// 可用对象下标列表
	TArray<int32> UsableActorsIndexes;
};
