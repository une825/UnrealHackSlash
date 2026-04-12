#include "GAS/GA/HGA_SpinningBlades.h"
#include "Skill/HProjectile_SpinningBlade.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "System/HObjectPoolManager.h"

UHGA_SpinningBlades::UHGA_SpinningBlades()
	: OrbitRadius(200.0f), RotationSpeed(180.0f), Duration(5.0f)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	BaseDamage = 15.0f;
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

	if (!AvatarActor || !ProjectileClass || !DamageEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());

	// 보조 젬 개수가 포함된 최종 개수를 가져옵니다.
	int32 TotalBladeCount = GetProjectileCount();
	float AngleStep = 360.0f / FMath::Max(1, TotalBladeCount);

	for (int32 i = 0; i < TotalBladeCount; ++i)
	{
		SpawnSpinningBlade(i, AngleStep, DamageSpecHandle);
	}

	// 안전한 지연 처리를 위해 AbilityTask_WaitDelay 사용
	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
	if (WaitDelayTask)
	{
		WaitDelayTask->OnFinish.AddDynamic(this, &UHGA_SpinningBlades::OnDelayFinish);
		WaitDelayTask->ReadyForActivation();
	}
	else
	{
		// 태스크 생성 실패 시 즉시 종료 (안전장치)
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UHGA_SpinningBlades::OnDelayFinish()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_SpinningBlades::SpawnSpinningBlade(int32 Index, float AngleStep, const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
	if (!AvatarActor || !Pool) return;

	float InitialAngle = Index * AngleStep;
	float RadAngle = FMath::DegreesToRadians(InitialAngle);
	FVector SpawnLoc = AvatarActor->GetActorLocation() + FVector(FMath::Cos(RadAngle) * OrbitRadius, FMath::Sin(RadAngle) * OrbitRadius, 0.0f);

	AActor* PooledActor = Pool->SpawnFromPool(ProjectileClass, SpawnLoc, FRotator::ZeroRotator);
	AHProjectile_SpinningBlade* Projectile = Cast<AHProjectile_SpinningBlade>(PooledActor);

	if (Projectile)
	{
		Projectile->SetOwner(AvatarActor);
		Projectile->SetInstigator(Cast<APawn>(AvatarActor));
		Projectile->Initialize(AvatarActor, OrbitRadius, RotationSpeed, InitialAngle, DamageSpecHandle);
		Projectile->SetProjectileLifeSpan(Duration);

		// 베이스 클래스에서 소스 오브젝트 설정 (보조 젬 효과 참조용)
		if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
		{
			Projectile->SetSourceObject(Spec->SourceObject.Get());
		}

		Projectile->ResetProjectile(SpawnLoc, FRotator::ZeroRotator);
	}
}
