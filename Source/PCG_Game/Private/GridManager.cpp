// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "PriorityQueueUnique.h"

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
	 FIntPoint StartGridLocation = FIntPoint(FMath::RandRange(0, Columns-1),FMath::RandRange(0, Rows-1));

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
	 	int32 RandomCounts = NowGridStatus.GetValidGridWithRotationNum();
		int32 RandomIndex = RandomStream.FRandRange(0,RandomCounts);

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
	 	if (AGrid* NewGrid = GetWorld()->SpawnActor<AGrid>(GridClasses[GridIndex],Location,Rotator,SpawnParameters); NewGrid != nullptr)
	 	{
	 		Grids[ArrayIndex] = NewGrid;
	 	}

	 	// 2D Status
	 	for (int32 DeltaX = -1; DeltaX <= 1; DeltaX++) {
	 		for (int32 DeltaY = -1; DeltaY <= 1; DeltaY++) {
	 			if (DeltaX == 0 && DeltaY == 0)
	 				continue;
	 			int32 NewX = GridLocation.X+DeltaX;
	 			int32 NewY = GridLocation.Y+DeltaY;
	 			
	 			if(NewX<0||NewX>=Columns||NewY<0||NewY>=Rows)
	 				continue;
	 			auto& NextGridStatus = GridStatuses[GetArrayIndexFromGridLocation(FIntPoint(NewX,NewY))];
	 			if(!NextGridStatus.IsCompleted())
	 			{
					// 根据新Grid规则进行更新 GridStatus
	 				


	 				
	 				GridStatusesPriorityQueueUnique.Enqueue(NextGridStatus,NextGridStatus.GetValidGridWithRotationNum());
	 			}
	 		}
	 	}

	 	GridStatuses[ArrayIndex] = NowGridStatus; 
	}
}

void AGridManager::InitGridStatuses()
{
	Grids.SetNumUninitialized(Rows*Columns);
	for (int32 Row = 0; Row < Rows; Row++)
	{
		for (int32 Column = 0; Column < Columns; Column++)
		{
			const FIntPoint GridLocation = FIntPoint(Column, Row);
			const int32 GridTypeNums = GridClasses.Num();
			// 2D With 4 Rotation ; 3D With 6 Rotation
			constexpr int32 RotationNums = 4;
			GridStatuses.Add(FGridStatus(GridLocation,GridTypeNums,RotationNums));
		}
	}
}

int32 AGridManager::GetArrayIndexFromGridLocation(const FIntPoint GridLocation) const 
{
	return GridLocation.Y * Columns + GridLocation.X; 
}

