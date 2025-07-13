#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/Map.h"

template<typename T,typename PriorityType,typename Compare = std::less<PriorityType>>
class TPriorityQueueUnique
{
	struct FHeapElement
	{
		T Element;
		PriorityType Priority;
		
		FHeapElement(const T& InElement,const PriorityType& InPriority)
			:Element(InElement),Priority(InPriority)
		{
			
		}
	};

	TArray<FHeapElement> Heap;
	TMap<T,int32> ElementToIndexMap;
	Compare Comparator;

public:
	TPriorityQueueUnique(const Compare& InComparator = Compare())
		:Comparator(InComparator)
	{}

	void Enqueue(const T& Element,const PriorityType& Priority)
	{
		if (ElementToIndexMap.Contains(Element))
		{
			UpdatePriority(Element,Priority);
		}
		else
		{
			AddNewElement(Element,Priority);
		}
	}

	bool Dequeue(T& OutElement,PriorityType& OutPriority)
	{
		if (Heap.Num() == 0)
			return false;

		const FHeapElement& Top = Heap[0];
		OutElement = Top.Element;
		OutPriority = Top.Priority;
		RemoveAt(0);
		return true;		
	}

	bool Peek(T& OutElement,PriorityType& OutPriority) const
	{
		if (Heap.Num() == 0)
			return false;

		OutElement = Heap[0].Element;
		OutPriority = Heap[0].Priority;
		return true;
	}

	bool IsEmpty() const
	{
		return Heap.IsEmpty();
	}

	int32 Num() const
	{
		return Heap.Num();
	}

	bool Contains(const T& Element) const
	{
		return ElementToIndexMap.Contains(Element);
	}

	bool UpdatePriority(const T& Element,const PriorityType& NewPriority)
	{
		if (const int32* IndexPtr = ElementToIndexMap.Find(Element))
		{
			UpdatePriorityInternel(*IndexPtr,NewPriority);
			return true;
		}
		return false;
	}

private:
	void AddNewElement(const T& Element,PriorityType Priority)
	{
		Heap.Add(FHeapElement(Element,Priority));
		int32 NewIndex = Heap.Num() -1;
		ElementToIndexMap.Add(Element,NewIndex);

		BubbleUp(NewIndex);
	}

	void UpdatePriorityInternel(int32 Index,PriorityType NewPriority)
	{
		const PriorityType OldPriority = Heap[Index].Priority;
		Heap[Index].Priority = NewPriority;

		if (Comparator(NewPriority,OldPriority))
		{
			BubbleUp(Index);
		}
		else
		{
			BubbleDown(Index);
		}
	}

	void RemoveAt(int32 Index)
	{
		if (Index < 0 || Index >= Heap.Num())
			return ;

		const T RemovedElement = Heap[Index].Element;
		const int32 LastIndex = Heap.Num()-1;
		if (Index != LastIndex)
		{
			SwapElements(Index,LastIndex);
		}

		Heap.RemoveAt(LastIndex);
		ElementToIndexMap.Remove(RemovedElement);

		if (Index < Heap.Num())
		{
			if (Index > 0 && Comparator(Heap[Index].Priority,Heap[Parent(Index)].Priority))
			{
				BubbleUp(Index);
			}
			else
			{
				BubbleDown(Index);
			}
		}
	}

	
	void SwapElements(int32 IndexA,int32 IndexB)
	{
		FHeapElement Temp = Heap[IndexA];
		Heap[IndexA] = Heap[IndexB];
		Heap[IndexB] = Temp;

		ElementToIndexMap[Heap[IndexA].Element] = IndexA;
		ElementToIndexMap[Heap[IndexB].Element] = IndexB;
	}
	
	void BubbleUp(int32 Index)
	{
		while (Index>0)
		{
			int32 ParentIndex = Parent(Index);

			if (Comparator(Heap[Index].Priority,Heap[ParentIndex].Priority))
			{
				SwapElements(Index,ParentIndex);
				Index = ParentIndex;
			}
			else
			{
				break;
			}
		}
	}

	void BubbleDown(int32 Index)
	{
		const int32 NumElements = Heap.Num();
		while (true)
		{
			int32 SmallestChild = Index;
			int32 LeftChild = Left(Index);
			int32 RightChild = Right(Index);

			if (LeftChild < NumElements && Comparator(Heap[LeftChild].Priority,Heap[SmallestChild].Priority))
			{
				SmallestChild = LeftChild;
			}

			if (RightChild < NumElements && Comparator(Heap[RightChild].Priority,Heap[SmallestChild].Priority))
			{
				SmallestChild = RightChild;
			}

			if (SmallestChild != Index)
			{
				SwapElements(Index,SmallestChild);
				Index = SmallestChild;
			}
			else
			{
				break;
			}
		}
		
	}

	int32 Parent(int32 Index) const {return (Index-1)/2;}
	int32 Left(int32 Index) const {return 2*Index+1;}
	int32 Right(int32 Index) const {return 2*Index+2;}


	static void Test()
	{
		TPriorityQueueUnique<FString, int32> BasicOperationTestQueue;
		TArray<TPair<FString, int32>> TestData = {
			{"Apple", 5},
			{"Banana", 3},
			{"Cherry", 7}
		};

		for (const auto& Data : TestData)
		{
			BasicOperationTestQueue.Enqueue(Data.Key, Data.Value);
		}

		if (BasicOperationTestQueue.Num() != 3)
		{
			UE_LOG(LogTemp, Warning, TEXT("BasicOperationTest 1 Failed"));
			return;
		}

		if(!BasicOperationTestQueue.Contains("Banana") ||
			!BasicOperationTestQueue.Contains("Apple") ||
			!BasicOperationTestQueue.Contains("Cherry"))
		{
			UE_LOG(LogTemp, Warning, TEXT("BasicOperationTest 2 Failed"));
			return;
		}


		FString Item;
		int32 Priority;
		BasicOperationTestQueue.Dequeue(Item, Priority);
		if (Item != "Banana" || Priority != 3)
		{
			UE_LOG(LogTemp, Warning, TEXT("BasicOperationTest 3 Failed"));
			return;
		}

		BasicOperationTestQueue.Dequeue(Item, Priority);
		if (Item != "Apple" || Priority != 5)
		{
			UE_LOG(LogTemp, Warning, TEXT("BasicOperationTest 4 Failed"));
			return;
		}

		BasicOperationTestQueue.Dequeue(Item, Priority);
		if (Item != "Cherry" || Priority != 7)
		{
			UE_LOG(LogTemp, Warning, TEXT("BasicOperationTest 5 Failed"));
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("BasicOperationTestSuccess"));


		TPriorityQueueUnique<FString, int32> UpdateTestQueue;

		UpdateTestQueue.Enqueue("A", 20);
		UpdateTestQueue.Enqueue("B", 10);
		UpdateTestQueue.Enqueue("C", 30);
		UpdateTestQueue.Enqueue("C", 25);

		UpdateTestQueue.UpdatePriority("B", 5);

		UpdateTestQueue.Dequeue(Item, Priority);
		if (Item != "B" || Priority != 5)
		{
			UE_LOG(LogTemp, Warning, TEXT("UpdateTest 1 Failed"));
			return;
		} 

		UpdateTestQueue.UpdatePriority("A", 35);
		UpdateTestQueue.Dequeue(Item, Priority);
		if (Item != "C" || Priority != 25)
		{
			UE_LOG(LogTemp, Warning, TEXT("UpdateTest 2 Failed"));
			return;
		}

		UpdateTestQueue.Dequeue(Item, Priority);
		if (Item != "A" || Priority != 35 || !UpdateTestQueue.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("UpdateTest 3 Failed"));
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("UpdateTestSuccess"));
	}
};
