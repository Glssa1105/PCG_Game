// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardCharacterAnimInstance.h"

#include "Game/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UWizardCharacterAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());

	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void UWizardCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();
	GroundSpeed_X = OwningCharacter->GetActorForwardVector() | OwningCharacter->GetVelocity() ;
	GroundSpeed_Y = OwningCharacter->GetActorRightVector() | OwningCharacter->GetVelocity();
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D()>0.0f;
	bIsAttacking = OwningCharacter->IsAttacking();
}
