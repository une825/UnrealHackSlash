#include "Skill/HProjectile.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/HObjectPoolManager.h"
#include "Engine/DamageEvents.h"

AHProjectile::AHProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComponent->InitSphereRadius(20.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &AHProjectile::OnHit);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AHProjectile::OnOverlap);
	
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = LifeSpan;
}

void AHProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (FlightEffect)
	{
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			UNiagaraComponent* NiagaraComp = Pool->SpawnNiagaraFromPool(FlightEffect, GetActorLocation(), GetActorRotation());
			if (NiagaraComp)
			{
				NiagaraComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}
}

void AHProjectile::OnHit(UPrimitiveComponent* InHitComp, AActor* InOtherActor, UPrimitiveComponent* InOtherComp, FVector InNormalImpulse, const FHitResult& InHit)
{
	if ((InOtherActor != nullptr) && (InOtherActor != this) && (InOtherComp != nullptr))
	{
		Explode();
	}
}

void AHProjectile::OnOverlap(UPrimitiveComponent* InOverlappedComponent, AActor* InOtherActor, UPrimitiveComponent* InOtherComp, int32 InOtherBodyIndex, bool bInFromSweep, const FHitResult& InSweepResult)
{
	if ((InOtherActor != nullptr) && (InOtherActor != this) && (InOtherComp != nullptr))
	{
		// 아군 체크
		if (AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(InOtherActor))
		{
			if (TargetCharacter->GetUnitProfileData() && TargetCharacter->GetUnitProfileData()->UnitType == OwningUnitType)
			{
				return;
			}
		}

		// 데미지 전달 로직
		FDamageEvent DamageEvent;
		InOtherActor->TakeDamage(DamageAmount, DamageEvent, GetInstigatorController(), this);

		Explode();
	}
}

void AHProjectile::Explode()
{
	if (ExplosionEffect)
	{
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			Pool->SpawnNiagaraFromPool(ExplosionEffect, GetActorLocation(), GetActorRotation());
		}
	}

	Destroy();
}
