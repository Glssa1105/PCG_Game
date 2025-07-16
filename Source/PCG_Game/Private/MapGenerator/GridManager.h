// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grid.h"
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
		FGridStatus()
			:m_GridLocation(FIntPoint()),m_RotationNums(0),m_GridTypeNums(0)
		{}
		
		
		FGridStatus(FIntPoint GridLocation,int32 GridTypeNums,int32 RotationNums)
			:m_GridLocation(GridLocation),m_RotationNums(RotationNums),m_GridTypeNums(GridTypeNums)
		{
			m_ValidGridList.Init(true,m_GridTypeNums);
			m_ValidGridRotationList.Init(true,m_RotationNums * m_GridTypeNums);
		}

		bool IsCompleted() const
		{
			return m_IsComplete;
		}

		void SetIsCompleted(bool IsCompleted)
		{
			m_IsComplete = IsCompleted;
		}
		
		bool operator==(const FGridStatus& Other) const
		{
			return Other.GetGridLocation() == GetGridLocation();
		}

		friend uint32 GetTypeHash(const FGridStatus& Status)
		{
			return GetTypeHash(Status.m_GridLocation);

		}
		
		bool IsValidGrid(const int32 GridTypeIndex)
		{
			check(GridTypeIndex < m_GridTypeNums)
			return m_ValidGridList[GridTypeIndex];
		}

		bool IsValidGridRotation(const int32 GridTypeIndex,const int32 RotationIndex)
		{
			check(GridTypeIndex < m_GridTypeNums);
			check(RotationIndex < m_RotationNums);
			return m_ValidGridRotationList[GridTypeIndex * m_RotationNums + RotationIndex];
		}

		void SetGridValid(const int32 GridTypeIndex,const bool IsValid)
		{
			check(GridTypeIndex < m_GridTypeNums);
			m_ValidGridList[GridTypeIndex] = IsValid;
		}

		void SetGridRotationValid(const int32 GridTypeIndex,const int32 RotationIndex,const bool IsValid)
		{
			check(GridTypeIndex < m_GridTypeNums);
			check(RotationIndex < m_RotationNums);
			const int32 RotationBitIndex = GetRotationBitIndex(GridTypeIndex,RotationIndex);
			m_ValidGridRotationList[RotationBitIndex] = IsValid; 
		}

		FIntPoint GetGridLocation() const 
		{
			return m_GridLocation;
		}

		int32 GetValidGridNum() const
		{
			return m_ValidGridList.CountSetBits();
		}

		int32 GetValidGridWithRotationNum() const
		{
			return m_ValidGridRotationList.CountSetBits();
		}

		void GetValidGridIndexArray(TArray<int32>& ValidIndexArray) const
		{
			for (TConstSetBitIterator<> It(m_ValidGridList);It;++It)
			{
				ValidIndexArray.Add(It.GetIndex());
			}
		}

		void GetValidGridRotationIndexArray(int32 GridIndex,TArray<int32>& ValidRotationIndexArray) const
		{
			const int32 StartIndex = GridIndex * m_RotationNums;
			for (int32 i = 0 ;i<m_RotationNums;++i)
			{
				if (m_ValidGridRotationList[StartIndex + i])
				{
					ValidRotationIndexArray.Add(i);
				}
			}
		}

		void GetRotatorByGridRotation(int32 GridRotation,FRotator& OutRotator) const
		{
			check(GridRotation >= 0 && GridRotation < m_RotationNums)
				
			switch(GridRotation)
			{
				case 0: OutRotator = FRotator(0, 0, 0);
					break;
				case 1: OutRotator = FRotator(0, 90, 0);
					break;
				case 2: OutRotator = FRotator(0, 180, 0);
					break;
				case 3: OutRotator = FRotator(0, 270, 0);
					break;
				default: OutRotator = FRotator::ZeroRotator;	
			}
		}

		bool GetGridWithRotationByValidIndex(int32 Index,int32& GridIndex,int32& GridRotation) const
		{
			if(Index < 0 || Index >= GetValidGridWithRotationNum())
			{
				return false;
			}
			int32 TargetIndex = 0;
			for(TConstSetBitIterator<> It(m_ValidGridRotationList);It;++It)
			{
				if(Index==0)
				{
					TargetIndex = It.GetIndex();
					break;
				}
				Index--;
			}

			GridIndex = TargetIndex/m_RotationNums;
			GridRotation = TargetIndex-GridIndex*m_RotationNums;
			return true;
		}
		
		
		struct FStatusPriorityComparator
		{
			bool operator()(int32 A, int32 B) const 
			{
				return A<B;
			}
		};
		
	private:
		uint32 GetRotationBitIndex(const int32 GridTypeIndex,const int32 RotationIndex) const
		{
			return GridTypeIndex * m_RotationNums + RotationIndex;
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

	UFUNCTION(BlueprintCallable,Category = "Grid Manager")
	void GenerateGrid(int32 Seed);

	// 根据已有 Grid 信息，更新指定 GridStatus 对象，剔除不可生成对象
	UFUNCTION(BlueprintCallable,Category = "Grid Manager")
	void UpdateGridStatesByGridSlotBitmask(const int32 GridStatesIndex,const int32 DirectionIndex,const int32 AcceptBitmask,const int32 SelfBitMask);

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Map Settings")
	int32 X_Size = 5;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Map Settings")
	int32 Y_Size = 5;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Map Settings")
	float GridSpacing = 100.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grid Map Settings")
	TArray<TSubclassOf<AGrid>> GridClasses;

private:
	void InitGridStatuses();
	static int32 GetOppositeDirectionIndex(const int32 DirectionIndex);
	int32 GetArrayIndexFromGridLocation(const FIntPoint GridLocation) const;
	
private:
	TArray<AGrid*> Grids;
	TArray<FGridStatus> GridStatuses;
};
