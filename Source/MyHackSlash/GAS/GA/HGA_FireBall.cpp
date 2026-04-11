#include "GAS/GA/HGA_FireBall.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Skill/HProjectile.h"
#include "GameFramework/Character.h"
#include "System/HObjectPoolManager.h"

UHGA_FireBall::UHGA_FireBall()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_FireBall::ActivateAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, const FGameplayEventData* InTriggerEventData)
{
	Super::ActivateAbility(InHandle, InActorInfo, InActivationInfo, InTriggerEventData);

	if (InActorInfo == nullptr || !InActorInfo->AvatarActor.IsValid())
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, true);
		return;
	}

	AHBaseCharacter* BaseCharacter = Cast<AHBaseCharacter>(InActorInfo->AvatarActor.Get());
	if (nullptr == BaseCharacter)
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, true);
		return;
	}

	// 투사체 발사 (몽타주 재생과 함께 또는 특정 시점에)
	SpawnProjectile();

	const UHUnitProfileData* Profile = BaseCharacter->GetUnitProfileData();
	UAnimMontage* MontageToPlay = (Profile) ? Profile->GetActionMontage(MontageTag) : nullptr;

	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* PlayAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("PlayFireBall"),
			MontageToPlay,
			BaseCharacter->GetAttackSpeedRate()
		);
		if (PlayAttackTask)
		{
			PlayAttackTask->OnCompleted.AddDynamic(this, &UHGA_FireBall::OnCompleteCallback);
			PlayAttackTask->OnInterrupted.AddDynamic(this, &UHGA_FireBall::OnInterruptedCallback);
			PlayAttackTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
	}
}

void UHGA_FireBall::SpawnProjectile()
{
	if (ProjectileClass == nullptr) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar == nullptr) return;

	UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
	if (Pool == nullptr) return;

	// 1. 플레이어의 중심에서 앞쪽으로 충분히 밀어내고(50.0f)
	FVector SpawnLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 50.0f;
	FRotator SpawnRotation = Avatar->GetActorRotation();

	// 2. 풀에서 액터 가져오기
	AActor* PooledActor = Pool->SpawnFromPool(ProjectileClass, SpawnLocation, SpawnRotation);
	AHProjectile* Projectile = Cast<AHProjectile>(PooledActor);

	if (Projectile)
	{
		// 3. 소유자 및 인스티게이터 설정 (재사용 시에도 업데이트 필요)
		Projectile->SetOwner(Avatar);
		Projectile->SetInstigator(Cast<APawn>(Avatar));

		// 4. 발사체 스펙 설정
		float FinalDamage = ProjectileDamage;
		Projectile->SetElement(HEElement::Fire);

		if (AHBaseCharacter* BaseCharacter = Cast<AHBaseCharacter>(Avatar))
		{
			FinalDamage += BaseCharacter->GetAttackDamage();

			if (BaseCharacter->GetUnitProfileData())
			{
				Projectile->SetOwningUnitType(BaseCharacter->GetUnitProfileData()->UnitType);
			}
		}

		Projectile->SetDamage(FinalDamage);
		Projectile->DamageEffectClass = FireBallDamageEffect;
		
		// 5. 풀링 전용 초기화 (이동 컴포넌트 재시작 등)
		Projectile->ResetProjectile(SpawnLocation, SpawnRotation);
	}
}

void UHGA_FireBall::InputPressed(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo)
{
	Super::InputPressed(InHandle, InActorInfo, InActivationInfo);
}

void UHGA_FireBall::CancelAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateCancelAbility)
{
	Super::CancelAbility(InHandle, InActorInfo, InActivationInfo, bInReplicateCancelAbility);
}

void UHGA_FireBall::EndAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateEndAbility, bool bInWasCancelled)
{
	Super::EndAbility(InHandle, InActorInfo, InActivationInfo, bInReplicateEndAbility, bInWasCancelled);
}

void UHGA_FireBall::OnCompleteCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_FireBall::OnInterruptedCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
