// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grid.h"
#include "PriorityQueueUnique.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UCLASS()
class AGridManager : public AActor
{
	GENERATED_BODY()

	struct FGridStatus
	{
	private:
		FIntPoint m_GridLocation;
		int32 m_RotationNums;
		int32 m_GridTypeNums;

		bool m_IsComplete = false;
		
		TBitArray<> m_ValidGridList;
		TBitArray<> m_ValidGridRotationList;
		
	public:
		FGridStatus(FIntPoint GridLocation,int32 GridTypeNums,int32 RotationNums)
			:m_GridLocation(GridLocation),m_RotationNums(RotationNums),m_GridTypeNums(GridTypeNums)
		{
			m_ValidGridList.Init(true,m_GridTypeNums);
			m_ValidGridRotationList.Init(true,m_RotationNums * m_GridTypeNums);
		}

		bool CheckComplete() const
		{
			return m_IsComplete;
		}

		void SetIsComplete()
		{
			m_IsComplete = true;
		}
		
		bool operator==(const FGridStatus& Other) const
		{
			return Other.GetGridLocation() == GetGridLocation();
		}

		friend uint32 GetTypeHash(const FGridStatus& Status)
		{
			uint32 Hash = GetTypeHash(Status.m_GridLocation);
			return Hash;
		}
		
		bool IsValidGrid(const int32 GridTypeIndex)
		{
			check(GridTypeIndex < m_GridTypeNums)
			return m_ValidGridList[GridTypeIndex];
		}

		bool IsValidGridDirection(const int32 GridTypeIndex,const int32 DirectionIndex)
		{
			check(GridTypeIndex < m_GridTypeNums);
			check(DirectionIndex < m_RotationNums);
			return m_ValidGridRotationList[GridTypeIndex * m_RotationNums + DirectionIndex];
		}

		void SetGridValid(const int32 GridTypeIndex,const bool IsValid)
		{
			check(GridTypeIndex < m_GridTypeNums);
			m_ValidGridList[GridTypeIndex] = IsValid;
		}

		void SetGridDirectionValid(const int32 GridTypeIndex,const int32 DirectionIndex,const bool IsValid)
		{
			check(GridTypeIndex < m_GridTypeNums);
			check(DirectionIndex < m_RotationNums);
			const int32 DirectionBitIndex = GetDirectionBitIndex(GridTypeIndex,DirectionIndex);
			m_ValidGridRotationList[DirectionBitIndex] = IsValid; 
		}

		FIntPoint GetGridLocation() const 
		{
			return m_GridLocation;
		}
		
		struct FStatusPriorityComparator
		{
			bool operator()(int32 A, int32 B) const 
			{
				return A<B;
			}
		};
		
	private:
		uint32 GetDirectionBitIndex(const int32 GridTypeIndex,const int32 DirectionIndex) const
		{
			return GridTypeIndex * m_RotationNums + DirectionIndex;
		}
	};
	
public:	
	// Sets default values for this actor's properties
	AGridManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable,Category = "Grid")
	void GenerateGrid();

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category = "Grid Settings")
	int32 Rows = 5;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category = "Grid Settings")
	int32 Columns = 5;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Settings")
	float GridSpacing = 100.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Settings")
	TArray<TSubclassOf<AGrid>> GridClasses;

private:
	void InitGridStatuses();
	
private:
	TArray<AGrid*> Grids;
	TArray<FGridStatus> GridStatuses;

	TPriorityQueueUnique<FGridStatus,int32,FGridStatus::FStatusPriorityComparator> GridStatusesPriorityQueueUnique;
};
