#include "VoxelizeBall.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "../../VoxelDestruction/VoxelizableComponent.h"
#include "Kismet/GameplayStatics.h"

AVoxelizeBall::AVoxelizeBall()
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
    
    SphereComponent->OnComponentHit.AddDynamic(this, &AVoxelizeBall::OnHit);

    Voxelizer = Cast<AVoxelizer>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelizer::StaticClass()));
    InitialLifeSpan = 5.0f;
}

void AVoxelizeBall::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && (OtherActor != this) && (OtherActor != GetOwner()))
    {
        for (UActorComponent* Component : OtherActor->GetComponents())
        {
            if (UVoxelizableComponent* VoxelizableComponent = OtherActor->FindComponentByClass<UVoxelizableComponent>())
            {
                if (Voxelizer == nullptr)
                {
                    UE_LOG(LogTemp, Display, TEXT("Didn't find AVoxelizer"));
                    return;
                }
                UE_LOG(LogTemp, Display, TEXT("Voxelize !"));
                
                Voxelizer->SetTarget(OtherActor);
                Voxelizer->StartVoxelize();
                OtherActor->Destroy();
                break;
            }
        }
        
        TArray<FHitResult> HitResults;
        FVector ExplosionOrigin = Hit.ImpactPoint;

        FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
        bool bHasHit = GetWorld()->SweepMultiByChannel(HitResults, ExplosionOrigin, ExplosionOrigin, FQuat::Identity, ECC_PhysicsBody, SphereShape);

        if (bHasHit)
        {
            for (auto& HitResult : HitResults)
            {
                UPrimitiveComponent* PhysicsComponent = HitResult.GetComponent();
                if (PhysicsComponent && PhysicsComponent->IsSimulatingPhysics())
                {
                    PhysicsComponent->AddRadialForce(ExplosionOrigin, ExplosionRadius, ExplosionStrength, ERadialImpulseFalloff::RIF_Constant, true);
                }
            }
        }
        
        Destroy();
    }
}


float AVoxelizeBall::GetPowerRatio()
{
    return PowerRatio;
}

void AVoxelizeBall::SetPowerRatio(float NewPowerRatio)
{
    PowerRatio = NewPowerRatio;
}

float AVoxelizeBall::PowerRatio = 220.f;


