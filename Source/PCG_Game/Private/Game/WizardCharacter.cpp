// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "WizardGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Input/WizardInputComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
AWizardCharacter::AWizardCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f,96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.0f;
	CameraBoom->SocketOffset = FVector(0.f,55.f,65.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f,500.0f,0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

void AWizardCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AWizardCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(InputConfigDataAsset,TEXT("InputConfigDataAsset is NULL"));
	ULocalPlayer* LocalPlayer =  GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	check(Subsystem);
	Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext,0);
	UWizardInputComponent* WizardInputComponent = CastChecked<UWizardInputComponent>(PlayerInputComponent);
	WizardInputComponent->BindNativeInputAction(InputConfigDataAsset,WizardGameplayTags::InputTag_Move,ETriggerEvent::Triggered,this,&ThisClass::Input_Move);
	WizardInputComponent->BindNativeInputAction(InputConfigDataAsset,WizardGameplayTags::InputTag_Look,ETriggerEvent::Triggered,this,&ThisClass::Input_Look);
	WizardInputComponent->BindNativeInputAction(InputConfigDataAsset,WizardGameplayTags::InputTag_Attack,ETriggerEvent::Triggered,this,&ThisClass::Input_Attack);
}

void AWizardCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Moving)
		return ;
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f,Controller->GetControlRotation().Yaw,0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardDirection,MovementVector.Y);
	}

	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection,MovementVector.X); 
	}
}

void AWizardCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Moving)
		return ;
	
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.0f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	if (LookAxisVector.Y != 0.0f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AWizardCharacter::Input_Attack(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Moving)
		return ;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		if (!AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			CharacterState = ECharacterState::Attacking;
			AnimInstance->Montage_Play(AttackMontage);
			FOnMontageEnded OnMontageEndedDelegate;
			OnMontageEndedDelegate.BindUObject(this,&AWizardCharacter::OnAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(OnMontageEndedDelegate,AttackMontage);
		}
	}
}

void AWizardCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	CharacterState = ECharacterState::Moving;
}



