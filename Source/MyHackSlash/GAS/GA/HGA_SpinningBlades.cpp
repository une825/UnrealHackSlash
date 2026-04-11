#include "GAS/GA/HGA_SpinningBlades.h"
#include "Skill/HProjectile_SpinningBlade.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "System/HObjectPoolManager.h"

UHGA_SpinningBlades::UHGA_SpinningBlades()
	: BladeCount(3), OrbitRadius(200.0f), RotationSpeed(180.0f), Duration(5.0f)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_SpinningBlades::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (AvatarActor && ProjectileClass && DamageEffectClass)
	{
		// 1. 데미지 적용을 위한 SpecHandle 생성
		FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
		
		// 2. 균등한 간격으로 칼날 생성
		float AngleStep = 360.0f / BladeCount;

		for (int32 i = 0; i < BladeCount; ++i)
		{
			SpawnProjectile(i, AngleStep, DamageSpecHandle);
		}
	}

	// 3. 지속 시간 후 어빌리티 종료 처리
	FTimerHandle EndTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(EndTimerHandle, [this, Handle, ActorInfo, ActivationInfo]()
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}, Duration, false);
}

void UHGA_SpinningBlades::SpawnProjectile(int32 Index, float AngleStep, const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	if (ProjectileClass == nullptr) return;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr) return;

	UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
	if (Pool == nullptr) return;

	// 1. 초기 위치 계산 (공전 로직에 의해 틱마다 갱신되지만 초기 생성 위치 전달)
	float InitialAngle = Index * AngleStep;
	float RadAngle = FMath::DegreesToRadians(InitialAngle);
	FVector SpawnLoc = AvatarActor->GetActorLocation() + FVector(FMath::Cos(RadAngle) * OrbitRadius, FMath::Sin(RadAngle) * OrbitRadius, 0.0f);

	// 2. 풀에서 가져오기
	AActor* PooledActor = Pool->SpawnFromPool(ProjectileClass, SpawnLoc, FRotator::ZeroRotator);
	AHProjectile_SpinningBlade* Projectile = Cast<AHProjectile_SpinningBlade>(PooledActor);

	if (Projectile)
	{
		// 3. 소유자 및 인스티게이터 설정
		Projectile->SetOwner(AvatarActor);
		Projectile->SetInstigator(Cast<APawn>(AvatarActor));

		// 4. 공전 파라미터 초기화
		Projectile->Initialize(AvatarActor, OrbitRadius, RotationSpeed, InitialAngle, DamageSpecHandle);
		
		// 5. 수명 동기화 및 리셋
		Projectile->SetProjectileLifeSpan(Duration);
		Projectile->ResetProjectile(SpawnLoc, FRotator::ZeroRotator);
	}
}
