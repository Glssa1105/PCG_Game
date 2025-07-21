// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DataAsset_InputConfig.generated.h"


class UInputMappingContext;
class UInputAction;

USTRUCT(BlueprintType)
struct FWizardInputActionConfig
{
	GENERATED_BODY()
public :
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (Catagories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UInputAction* InputAction;
};


UCLASS()
class PCG_GAME_API UDataAsset_InputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (TitleProperty = "InputTag"))
	TArray<FWizardInputActionConfig> NativeInputActions;

	UInputAction* FindNativeInputActionByTag(const FGameplayTag& InputTag) const ;
};
