#include "Skill/HProjectile.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/HObjectPoolManager.h"
#include "Engine/DamageEvents.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"

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

	// InitialLifeSpan = LifeSpan; // 생성 시 수명 관리 대신 타이머로 관리할 것이므로 주석 처리하거나 제거 가능
}

void AHProjectile::BeginPlay()
{
	Super::BeginPlay();

	bIsActive = true;

	// 생성 직후 라이프사이클 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(LifeSpanTimerHandle, this, &AHProjectile::OnLifeSpanExpired, LifeSpan, false);

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

void AHProjectile::ResetProjectile(FVector InLocation, FRotator InRotation)
{
	bIsActive = true;

	SetActorLocationAndRotation(InLocation, InRotation);
	
	// 1. 발사체 이동 컴포넌트 초기화 및 재활성화
	if (ProjectileMovement)
	{
		ProjectileMovement->Activate(true);
		ProjectileMovement->Velocity = InRotation.Vector() * ProjectileMovement->InitialSpeed;
		ProjectileMovement->UpdateComponentVelocity();
	}
	
	// 2. 라이프사이클 타이머 초기화 및 시작
	GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(LifeSpanTimerHandle, this, &AHProjectile::OnLifeSpanExpired, LifeSpan, false);

	// 3. 비행 이펙트 다시 부착
	if (FlightEffect)
	{
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			UNiagaraComponent* NiagaraComp = Pool->SpawnNiagaraFromPool(FlightEffect, InLocation, InRotation);
			if (NiagaraComp)
			{
				NiagaraComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}
}

void AHProjectile::OnLifeSpanExpired()
{
	if (!bIsActive) return;

	// 수명이 다하면 풀에 반납
	if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		bIsActive = false;
		Pool->ReturnToPool(this);
	}
	else
	{
		Destroy(); // 매니저를 못 찾으면 파괴
	}
}

void AHProjectile::OnHit(UPrimitiveComponent* InHitComp, AActor* InOtherActor, UPrimitiveComponent* InOtherComp, FVector InNormalImpulse, const FHitResult& InHit)
{
	if (bIsActive && (InOtherActor != nullptr) && (InOtherActor != this) && (InOtherComp != nullptr))
	{
		Explode();
	}
}

void AHProjectile::OnOverlap(UPrimitiveComponent* InOverlappedComponent, AActor* InOtherActor, UPrimitiveComponent* InOtherComp, int32 InOtherBodyIndex, bool bInFromSweep, const FHitResult& InSweepResult)
{
	if (bIsActive && (InOtherActor != nullptr) && (InOtherActor != this) && (InOtherComp != nullptr))
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
	if (!bIsActive) return;
	bIsActive = false;

	// 타이머 해제
	GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimerHandle);

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}

	UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();

	// 1. 시각 효과 재생 및 크기 조절
	if (ExplosionEffect && Pool)
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

	// 2. 주변 범위 데미지 처리
	TArray<FHitResult> OutHits;
	FVector ExplodeLocation = GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	bool bHasHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		ExplodeLocation,
		ExplodeLocation + FVector(0.1f, 0.f, 0.f),
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
				if (AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(HitActor))
				{
					if (TargetCharacter->GetUnitProfileData() && TargetCharacter->GetUnitProfileData()->UnitType == OwningUnitType)
					{
						continue;
					}
				}

				if (DamageEffectClass)
				{
					if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
					{
						if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator()))
						{
							FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
							ContextHandle.AddHitResult(Hit);

							FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
							if (SpecHandle.IsValid())
							{
								if (AHBaseCharacter* SourceCharacter = Cast<AHBaseCharacter>(GetInstigator()))
								{
									const float CritChance = SourceCharacter->GetCriticalRate();
									if (FMath::FRandRange(0.0f, 100.0f) <= CritChance)
									{
										SpecHandle.Data->AddDynamicAssetTag(FGameplayTag::RequestGameplayTag(TEXT("Effect.Critical")));
									}
								}
								SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
							}
						}
					}
				}
				else
				{
					FDamageEvent DamageEvent;
					HitActor->TakeDamage(DamageAmount, DamageEvent, GetInstigatorController(), this);
				}
				DamagedActors.Add(HitActor);
			}
		}
	}

	// 3. 투사체 파괴 대신 풀에 반납
	if (Pool)
	{
		Pool->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}
