#include "ObjectPoolComponent.h"
#include "PooledActor.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

void UObjectPoolComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// 可能Crash
	// RecycleActor();
}

void UObjectPoolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for(APooledActor* Actor : AvailableActors)
	{
		if(Actor) Actor->Destroy();
	}

	AvailableActors.Empty();
	UsableActorsIndexes.Empty();
	
	UsedActorIndexList.Empty();
	UsedActorIndexListMap.Empty();
}

void UObjectPoolComponent::Init()
{
	AvailableActors.Empty();
	UsableActorsIndexes.Empty();
	
	UsedActorIndexList.Empty();
	UsedActorIndexListMap.Empty();
	
	AvailableActors.Reserve(MaxPoolSize);

	for(int i = 0;i<InitSize;++i)
	{
		APooledActor* NewActor = GetWorld()->SpawnActor<APooledActor>(PooledActorClass,FVector::ZeroVector,FRotator::ZeroRotator);
		if(NewActor)
		{
			NewActor->OnReturnedToPool();
			AvailableActors.Add(NewActor);
			UsableActorsIndexes.Add(i);
		}
	}
}

void UObjectPoolComponent::RecycleActor()
{
	auto NowNode = UsedActorIndexList.GetHead();
	if(NowNode == nullptr) return ;
	auto NowActorPtr = AvailableActors[NowNode->GetValue()];
	if(NowActorPtr == nullptr) return ;

	while(GetWorld()->GetTimeSeconds() - NowActorPtr->GetLastUseTime() >= RecycleAfterSeconds)
	{
		ReturnPooledActor(NowActorPtr);
		NowNode = UsedActorIndexList.GetHead();
		if(NowNode == nullptr) return ;
		NowActorPtr = AvailableActors[NowNode->GetValue()];
		if(NowActorPtr == nullptr) return ;
	}
}

APooledActor* UObjectPoolComponent::GetPooledActor()
{
	if(!PooledActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PooledActorClass is EMPTY!!!!!!!"));
		return nullptr;
	}

	APooledActor* ActorToProvide = nullptr;

	// 有可用Actor
	if(UsableActorsIndexes.Num())
	{
		int32 Index = UsableActorsIndexes.Pop();
		ActorToProvide = AvailableActors[Index];
		UsedActorIndexList.AddTail(Index);
		UsedActorIndexListMap.Add(ActorToProvide,UsedActorIndexList.GetTail());
	}
	// 无可用Actor
	else
	{
		// 当前池内对象少于上限
		if(AvailableActors.Num() < MaxPoolSize)
		{
			ActorToProvide = GetWorld()->SpawnActor<APooledActor>(PooledActorClass,FVector::ZeroVector,FRotator::ZeroRotator);
			if(ActorToProvide == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("PoolActorClass spawn failed!!!!!!!"));
				return nullptr;
			}
			
			int32 NewIndex = AvailableActors.Add(ActorToProvide);
			UsedActorIndexList.AddTail(NewIndex);
			UsedActorIndexListMap.Add(ActorToProvide,UsedActorIndexList.GetTail());
		}
		// 当前池内对象达到上限
		else
		{
			// 移除最前者
			int32 ReuseIndex = UsedActorIndexList.GetHead()->GetValue();
			UsedActorIndexList.RemoveNode(UsedActorIndexList.GetHead());
			ActorToProvide = AvailableActors[ReuseIndex];
			if (ActorToProvide)
				ActorToProvide->OnReuse();
			else
			{
				ActorToProvide = GetWorld()->SpawnActor<APooledActor>(PooledActorClass,FVector::ZeroVector,FRotator::ZeroRotator);
			}
			
			UsedActorIndexList.AddTail(ReuseIndex);
			UsedActorIndexListMap.Add(ActorToProvide,UsedActorIndexList.GetTail());
		}
	}

	if(!ActorToProvide)
	{
		UE_LOG(LogTemp, Error, TEXT("Get actor from pool failed!!!!!!!"));
		return nullptr;
	}
	ActorToProvide->OnPulledFromPool();
	return ActorToProvide;
}

void UObjectPoolComponent::ReturnPooledActor(APooledActor* ActorToReturn)
{
	if(ActorToReturn == nullptr) return;
	TDoubleLinkedList<int32>::TDoubleLinkedListNode** NodePtr = UsedActorIndexListMap.Find(ActorToReturn);
	if(NodePtr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("尝试返回非池中的Actor: %s。该Actor将被直接销毁"), *ActorToReturn->GetName());
		ActorToReturn->Destroy();
		return ;
	}
	int32 UsableIndex = (*NodePtr)->GetValue();
	UsedActorIndexList.RemoveNode(*NodePtr);
	UsedActorIndexListMap.Remove(ActorToReturn);

	ActorToReturn->OnReturnedToPool();
	UsableActorsIndexes.Add(UsableIndex);
}


