// Copyright Epic Games, Inc. All Rights Reserved.

#include "PCG_GameGameMode.h"
#include "PCG_GamePlayerController.h"
#include "PCG_GameCharacter.h"
#include "UObject/ConstructorHelpers.h"

APCG_GameGameMode::APCG_GameGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = APCG_GamePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}