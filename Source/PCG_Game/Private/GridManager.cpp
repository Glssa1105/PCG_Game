// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "PriorityQueueUnique.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGridManager::GenerateGrid(int32 Seed)
{
	 InitGridStatuses();

	 // 可去重优先队列，选取可选情况最小进行塌陷
	 TPriorityQueueUnique<FGridStatus,int32,FGridStatus::FStatusPriorityComparator> GridStatusesPriorityQueueUnique;
	 FVector StartWorldLocation = GetActorLocation();
	 FIntPoint StartGridLocation = FIntPoint(FMath::RandRange(0 ,X_Size-1),FMath::RandRange(0, Y_Size-1));

	 // 选择初始点
	 const int32 StartArrayIndex = GetArrayIndexFromGridLocation(StartGridLocation);
	
	 FRandomStream RandomStream;
	 RandomStream.Initialize(Seed);
	
	 // 将初始状态入队
	 auto& GridStatus = GridStatuses[StartArrayIndex];
	 GridStatusesPriorityQueueUnique.Enqueue(GridStatus,GridStatus.GetValidGridWithRotationNum());
	
	 // BFS
	 while (!GridStatusesPriorityQueueUnique.IsEmpty())
	 {
	 	FGridStatus NowGridStatus;
		int32 NowValidGridNum;
	 	GridStatusesPriorityQueueUnique.Dequeue(NowGridStatus,NowValidGridNum);

	 	NowGridStatus.SetIsCompleted(true);
	 	
	 	// 权重随机算法 这里先均分概率
	 	int32 ValidCounts = NowGridStatus.GetValidGridWithRotationNum();
	 	if (ValidCounts == 0)
	 	{
	 		// 处理所有情况都被剔除时
	 		// 尽可能不出现该情况，可能塞一个空？
			UE_LOG(LogTemp, Display, TEXT("Fail To Search Next Grid"));
	 	}
	 	int32 RandomIndex = RandomStream.FRandRange(0,ValidCounts);
	 	
	 	int32 GridIndex,GridRotation;
	 	NowGridStatus.GetGridWithRotationByValidIndex(RandomIndex,GridIndex,GridRotation);
	 	FIntPoint GridLocation = NowGridStatus.GetGridLocation();
	 	
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	 	int32 ArrayIndex = GetArrayIndexFromGridLocation(GridLocation);
	 	
	 	FVector Location = StartWorldLocation + FVector(GridLocation.X*GridSpacing, GridLocation.Y*GridSpacing, 0);
	 	FRotator Rotator;
	 	NowGridStatus.GetRotatorByGridRotation(GridRotation,Rotator);

	 	AGrid* NewGrid;
	 	if (NewGrid = GetWorld()->SpawnActor<AGrid>(GridClasses[GridIndex],Location,Rotator,SpawnParameters); NewGrid != nullptr)
	 	{
	 		Grids[ArrayIndex] = NewGrid;
	 	}
	 	else
	 	{
	 		UE_LOG(LogTemp,Error,TEXT("%s"),TEXT("Failed to Spawn Grid"));
	 	}

	 	// 2D Status
	 	const TArray<TPair<int32,int32>> NextGridDelta =
	 		{
	 			{1,0},{0,1},{-1,0},{0,-1}
	 		};
	 	int DirectionIndex = 0;

	 	// 4 个方向
	 	for (auto& NextPair:NextGridDelta) {
	 		int32 DeltaX = NextPair.Key;
	 		int32 DeltaY = NextPair.Value;
	 		int32 NewX = GridLocation.X+DeltaX;
	 		int32 NewY = GridLocation.Y+DeltaY;
	 		
	 		if(NewX<0||NewX>=X_Size||NewY<0||NewY>=Y_Size)
	 			continue;
	 		int NextGridArrayIndex = GetArrayIndexFromGridLocation(FIntPoint(NewX,NewY));
	 		auto& NextGridStatus = GridStatuses[NextGridArrayIndex];
	 		if(!NextGridStatus.IsCompleted())
	 		{
	 			int32 AcceptBitmask = NewGrid->GetDirectionAcceptBitmask(GridRotation,DirectionIndex);
	 			int32 SelfBitmask = NewGrid->GetDirectionSelfBitmask(GridRotation,DirectionIndex);
	 			int32 OppositeDirectionIndex = GetOppositeDirectionIndex(DirectionIndex);

	 			// 根据新 Grid 的 SlotBitMask 更新对应位置的 GridStatus，去除不可能对象
	 			
	 			UpdateGridStatesByGridSlotBitmask(NextGridArrayIndex,OppositeDirectionIndex,AcceptBitmask,SelfBitmask);
	 			
	 			GridStatusesPriorityQueueUnique.Enqueue(NextGridStatus,NextGridStatus.GetValidGridWithRotationNum());
	 		}
	 		DirectionIndex++;
	 	}

	 	GridStatuses[ArrayIndex] = NowGridStatus; 
	}
}

void AGridManager::UpdateGridStatesByGridSlotBitmask(const int32 GridStatesIndex,const int32 DirectionIndex,const int32 AcceptBitmask,const int32 SelfBitMask)
{
	FGridStatus& GridStatus = GridStatuses[GridStatesIndex];

	// 目前可能的 Grid 对象
	TArray<int32> ValidGridTypeIndexArray;
	GridStatus.GetValidGridIndexArray(ValidGridTypeIndexArray);
	for (int32 GridTypeIndex:ValidGridTypeIndexArray)
	{
		// 目前该 Gird 对象可能的旋转情况
		TArray<int32> ValidGridRotationIndexArray;
		GridStatus.GetValidGridRotationIndexArray(GridTypeIndex,ValidGridRotationIndexArray);
		int RotationCounts = ValidGridRotationIndexArray.Num();
		
		for (int32 RotationIndex:ValidGridRotationIndexArray)
		{
			// 若该可能会发生冲突，则剔除
			if (!GridClasses[GridTypeIndex].GetDefaultObject()->CheckConnectionValid(RotationIndex,DirectionIndex,AcceptBitmask,SelfBitMask))
			{
				GridStatus.SetGridRotationValid(GridTypeIndex,RotationIndex,false);
				RotationCounts--;
			}
		}

		if (RotationCounts == 0)
			GridStatus.SetGridValid(GridTypeIndex,false);
	}
}

void AGridManager::InitGridStatuses()
{
	Grids.SetNumUninitialized(X_Size*Y_Size);
	for (int32 X = 0; X < X_Size; X++)
	{
		for (int32 Y = 0; Y < Y_Size; Y++)
		{
			const FIntPoint GridLocation = FIntPoint(X, Y);
			const int32 GridTypeNums = GridClasses.Num();
			// 2D With 4 Direction ; 3D With 6 Direction
			constexpr int32 RotationNums = 4;
			GridStatuses.Add(FGridStatus(GridLocation,GridTypeNums,RotationNums));
		}
	}
}

int32 AGridManager::GetOppositeDirectionIndex(const int32 DirectionIndex)
{
	return (DirectionIndex+2)%4;
}

int32 AGridManager::GetArrayIndexFromGridLocation(const FIntPoint GridLocation) const 
{
	return GridLocation.X * Y_Size + GridLocation.Y; 
}

