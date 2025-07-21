// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WizardBaseAnimInstance.h"
#include "WizardCharacterAnimInstance.generated.h"


class UCharacterMovementComponent;
class ABaseCharacter;

UCLASS()
class PCG_GAME_API UWizardCharacterAnimInstance : public UWizardBaseAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY()
	ABaseCharacter* OwningCharacter;

	UPROPERTY()
	UCharacterMovementComponent* OwningMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData")
	float GroundSpeed_X;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData")
	float GroundSpeed_Y;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData")
	float GroundSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData")
	bool bHasAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData")
	bool bIsAttacking;	
};
