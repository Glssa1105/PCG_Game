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
	RecycleActor();
}

void UObjectPoolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	for(APooledActor* Actor : AvailableActors)
	{
		if(IsValid(Actor))
		{
            Actor->OnDestroyed.RemoveAll(this); 
			Actor->Destroy();
		}
	}
	AvailableActors.Empty();
	UsableActorsIndexes.Empty();
	UsedActorIndexList.Empty();
	UsedActorIndexListMap.Empty();
}

void UObjectPoolComponent::Init()
{
	for (APooledActor* Actor : AvailableActors)
	{
		if (IsValid(Actor))
		{
			Actor->OnDestroyed.RemoveAll(this);
			Actor->Destroy();
		}
	}
	AvailableActors.Empty();
	UsableActorsIndexes.Empty();
	UsedActorIndexList.Empty();
	UsedActorIndexListMap.Empty();
	
	if (!PooledActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PooledActorClass is not set!"));
		return;
	}

	AvailableActors.Reserve(MaxPoolSize);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = GetOwner()->GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for(int i = 0; i < InitSize; ++i)
	{
		if (AvailableActors.Num() >= MaxPoolSize) break;
		
		APooledActor* NewActor = GetWorld()->SpawnActor<APooledActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if(NewActor)
		{
            NewActor->OnDestroyed.AddDynamic(this, &UObjectPoolComponent::OnPooledActorDestroyed);
			NewActor->OnReturnedToPool();
			const int32 NewIndex = AvailableActors.Add(NewActor);
			UsableActorsIndexes.Add(NewIndex);
		}
	}
}

void UObjectPoolComponent::OnPooledActorDestroyed(AActor* DestroyedActor)
{
    APooledActor* PooledActor = Cast<APooledActor>(DestroyedActor);
    if (!PooledActor) return;
    UE_LOG(LogTemp, Log, TEXT("Pool actor '%s' was destroyed externally"), *PooledActor->GetName());
    if (UsedActorIndexListMap.Contains(PooledActor))
    {
        TDoubleLinkedList<int32>::TDoubleLinkedListNode* Node = UsedActorIndexListMap.FindAndRemoveChecked(PooledActor);
        UsedActorIndexList.RemoveNode(Node);
    }
	
    const int32 IndexInAllActors = AvailableActors.Find(PooledActor);
    if (IndexInAllActors != INDEX_NONE)
    {
        UsableActorsIndexes.Remove(IndexInAllActors);
        AvailableActors[IndexInAllActors] = nullptr;
    }
}

APooledActor* UObjectPoolComponent::GetPooledActor()
{
	if(!PooledActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT(" PooledActorClass is EMPTY!!!!!!!"));
		return nullptr;
	}

	while (UsableActorsIndexes.Num() > 0)
	{
		const int32 Index = UsableActorsIndexes.Pop();
		if (AvailableActors.IsValidIndex(Index) && AvailableActors[Index] != nullptr)
		{
			APooledActor* ActorToProvide = AvailableActors[Index];
			UsedActorIndexList.AddTail(Index);
			UsedActorIndexListMap.Add(ActorToProvide, UsedActorIndexList.GetTail());
			ActorToProvide->OnPulledFromPool();
			return ActorToProvide;
		}
	}

	if (AvailableActors.Num() < MaxPoolSize)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetOwner()->GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		APooledActor* ActorToProvide = GetWorld()->SpawnActor<APooledActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator,SpawnParams);
		if(ActorToProvide)
		{
            ActorToProvide->OnDestroyed.AddDynamic(this, &UObjectPoolComponent::OnPooledActorDestroyed);
            
			const int32 NewIndex = AvailableActors.Add(ActorToProvide);
			UsedActorIndexList.AddTail(NewIndex);
			UsedActorIndexListMap.Add(ActorToProvide, UsedActorIndexList.GetTail());
			ActorToProvide->SetActorHiddenInGame(false);
			ActorToProvide->SetActorEnableCollision(true);
			ActorToProvide->SetActorTickEnabled(true);
			return ActorToProvide;
		}
	}
	
	while (UsedActorIndexList.GetHead() != nullptr)
	{
		TDoubleLinkedList<int32>::TDoubleLinkedListNode* HeadNode = UsedActorIndexList.GetHead();
		const int32 ReuseIndex = HeadNode->GetValue();
		
		if (AvailableActors.IsValidIndex(ReuseIndex) && AvailableActors[ReuseIndex] != nullptr)
		{
			APooledActor* ActorToProvide = AvailableActors[ReuseIndex];
			
			UsedActorIndexListMap.Remove(ActorToProvide);
			UsedActorIndexList.RemoveNode(HeadNode);
			UsedActorIndexList.AddTail(ReuseIndex);
			UsedActorIndexListMap.Add(ActorToProvide, UsedActorIndexList.GetTail());

			ActorToProvide->OnPulledFromPool(); 
			return ActorToProvide;
		}
		else
		{
            UsedActorIndexList.RemoveNode(HeadNode);
		}
	}

	return nullptr;
}


void UObjectPoolComponent::ReturnPooledActor(APooledActor* ActorToReturn)
{
	if (!IsValid(ActorToReturn) || ActorToReturn->IsPendingKillPending())
	{
		return;
	}
	
	TDoubleLinkedList<int32>::TDoubleLinkedListNode** NodePtr = UsedActorIndexListMap.Find(ActorToReturn);

	if (NodePtr == nullptr)
	{
		return;
	}
	
	const int32 UsableIndex = (*NodePtr)->GetValue();
	UsedActorIndexList.RemoveNode(*NodePtr);
	UsedActorIndexListMap.Remove(ActorToReturn);
	ActorToReturn->OnReturnedToPool();
	UsableActorsIndexes.Add(UsableIndex);
}

void UObjectPoolComponent::RecycleActor()
{
	while (true)
	{
		TDoubleLinkedList<int32>::TDoubleLinkedListNode* HeadNode = UsedActorIndexList.GetHead();
		
		if (!HeadNode)
		{
			break;
		}

		const int32 OldestIndex = HeadNode->GetValue();
		
		if (!AvailableActors.IsValidIndex(OldestIndex))
		{
			UsedActorIndexList.RemoveNode(HeadNode);
			continue;
		}

		APooledActor* OldestActor = AvailableActors[OldestIndex];
		
		if (!IsValid(OldestActor))
		{

			UsedActorIndexListMap.Remove(OldestActor); 
			UsedActorIndexList.RemoveNode(HeadNode);   
			continue;
		}
		
		if (GetWorld()->GetTimeSeconds() - OldestActor->GetLastUseTime() >= RecycleAfterSeconds)
		{
			ReturnPooledActor(OldestActor);
		}
		else
		{
			break;
		}
	}
}