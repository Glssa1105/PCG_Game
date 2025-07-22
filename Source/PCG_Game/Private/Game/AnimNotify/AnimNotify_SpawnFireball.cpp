#include "AnimNotify_SpawnFireball.h"
#include "../WizardCharacter.h"
#include "../GeneratedActor/FireBall.h"

void UAnimNotify_SpawnFireball::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!FireballClass || !MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
	UE_LOG(LogTemp,Display,TEXT("Generate Fire Ball!"));

	const FVector SpawnLocation = MeshComp->GetSocketLocation(SocketName);
	const FRotator SpawnRotation = MeshComp->GetOwner()->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MeshComp->GetOwner();
	SpawnParams.Instigator = Cast<APawn>(MeshComp->GetOwner());

	GetWorld()->SpawnActor<AFireballActor>(FireballClass, SpawnLocation, SpawnRotation, SpawnParams);
}