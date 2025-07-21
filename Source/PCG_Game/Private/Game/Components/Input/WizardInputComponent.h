// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Game/DataAssets/Input/DataAsset_InputConfig.h"
#include "WizardInputComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_GAME_API UWizardInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template<class UserObject,typename CallbackFunc>
	void BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig,const FGameplayTag& InInputTag,ETriggerEvent TriggerEvent,UserObject* ContextObject,CallbackFunc Func);
};

template <class UserObject, typename CallbackFunc>
void UWizardInputComponent::BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig,
	const FGameplayTag& InInputTag, ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func)
{
	checkf(InInputConfig,TEXT("Input config data asset is null,can not proceed with binding"));

	if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
	{
		BindAction(FoundAction,TriggerEvent,ContextObject,Func);
	}
}
