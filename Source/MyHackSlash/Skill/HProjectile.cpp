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

		// 충돌 시 즉시 폭발 (폭발 로직에서 범위 데미지 처리)
		Explode();
	}
}

void AHProjectile::Explode()
{
	// 1. 시각 효과 재생 및 크기 조절
	if (ExplosionEffect)
	{
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			UNiagaraComponent* NiagaraComp = Pool->SpawnNiagaraFromPool(ExplosionEffect, GetActorLocation(), GetActorRotation());
			if (NiagaraComp)
			{
				// 에셋의 기본 반지름을 100.0f라고 가정하여 스케일 계산
				float EffectScale = ExplosionRadius / 10.0f;
				
				// 소유자 액터(ANiagaraActor)의 스케일을 조절하여 전체 크기 변경
				if (AActor* NiagaraOwner = NiagaraComp->GetOwner())
				{
					NiagaraOwner->SetActorScale3D(FVector(EffectScale));
				}

				// 컴포넌트 자체의 스케일도 설정 (안전장치)
				NiagaraComp->SetWorldScale3D(FVector(EffectScale));

				// 나이아가라 시스템 내부 변수 제어 시도 (에셋에 따라 작동 여부가 다름)
				NiagaraComp->SetFloatParameter(TEXT("User.Scale"), EffectScale);
				NiagaraComp->SetFloatParameter(TEXT("User.Radius"), ExplosionRadius);
			}
		}
	}

	// 2. 주변 범위 데미지 처리
	TArray<FHitResult> OutHits;
	FVector ExplodeLocation = GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);

	// 대상 필터링을 위한 콜리전 쿼리 설정
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	// 범위 내의 모든 대상을 검출 (Pawn 채널 기준)
	bool bHasHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		ExplodeLocation,
		ExplodeLocation + FVector(0.1f, 0.f, 0.f), // 아주 미세한 이동으로 Sweep 수행
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	if (bHasHit)
	{
		TArray<AActor*> DamagedActors;
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !DamagedActors.Contains(HitActor))
			{
				// 아군 여부 재확인 (OnOverlap과 동일 로직)
				if (AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(HitActor))
				{
					if (TargetCharacter->GetUnitProfileData() && TargetCharacter->GetUnitProfileData()->UnitType == OwningUnitType)
					{
						continue;
					}
				}

				// 데미지 적용
				FDamageEvent DamageEvent;
				HitActor->TakeDamage(DamageAmount, DamageEvent, GetInstigatorController(), this);
				DamagedActors.Add(HitActor);
			}
		}
	}

	// 3. 투사체 파괴
	Destroy();
}
