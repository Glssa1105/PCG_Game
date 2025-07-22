#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnFireball.generated.h"

class AFireballActor;

UCLASS()
class PCG_GAME_API UAnimNotify_SpawnFireball : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AFireballActor> FireballClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FName SocketName = "Muzzle_01";
};