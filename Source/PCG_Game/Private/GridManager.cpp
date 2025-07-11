// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Grids.SetNumUninitialized(Rows*Columns);
	
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

void AGridManager::GenerateGrid()
{
	InitGridStatuses();
	
	FVector StartWorldLocation = GetActorLocation();
	
	FIntPoint StartGridLocation = FIntPoint(FMath::RandRange(0, Columns-1),FMath::RandRange(0, Rows-1));
	
	// for (int32 Row = 0; Row < Rows; Row++)
	// {
	// 	for (int32 Column = 0; Column < Columns; Column++)
	// 	{
	// 		FVector Location = StartWorldLocation + FVector(Row*GridSpacing, Column*GridSpacing, 0);
	// 		FActorSpawnParameters SpawnParameters;
	// 		SpawnParameters.Owner = this;
	// 		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//
	// 		const int32 RandomIndex = FMath::RandRange(0, GridClasses.Num()-1);
	// 		UE_LOG(LogTemp,Display,TEXT("Random Index = %d , GridClassesMax = %d"),RandomIndex,GridClasses.Num()-1);
	// 		if (AGrid* NewGrid = GetWorld()->SpawnActor<AGrid>(GridClasses[RandomIndex],Location,FRotator::ZeroRotator,SpawnParameters); NewGrid != nullptr)
	// 		{
	// 			Grids[Row*Rows + Column] = NewGrid;
	// 		}
	// 	}
	// }

	
}

void AGridManager::InitGridStatuses()
{
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

