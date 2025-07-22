#include "Fireball.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"

AFireballActor::AFireballActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    RootComponent = SphereComponent;
    SphereComponent->SetCollisionProfileName(TEXT("Projectile"));
    SphereComponent->SetNotifyRigidBodyCollision(true);

    FireballVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireballVFX"));
    FireballVFX->SetupAttachment(RootComponent);
    
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(RootComponent); 
    ProjectileMovementComponent->InitialSpeed = 1500.f; 
    ProjectileMovementComponent->MaxSpeed = 2000.f;  
    ProjectileMovementComponent->bRotationFollowsVelocity = true; 
    ProjectileMovementComponent->bShouldBounce = false; 
    ProjectileMovementComponent->ProjectileGravityScale = 0.f;

    SphereComponent->OnComponentHit.AddDynamic(this, &AFireballActor::OnHit);
    
    InitialLifeSpan = 5.0f;
}

void AFireballActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && (OtherActor != this) && (OtherActor != GetOwner()))
    {
        Destroy();
    }
}