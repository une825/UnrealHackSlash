#include "GAS/GA/HGA_FireBall.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Skill/HProjectile.h"
#include "GameFramework/Character.h"

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

	// 1. 플레이어의 중심에서 앞쪽으로 충분히 밀어내고(100.0f), 높이를 약간 낮춤
	FVector SpawnLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 50.0f;
	FRotator SpawnRotation = Avatar->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);
	// 무언가와 겹쳐 있더라도 일단 생성 (생성 실패 방지)
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHProjectile* Projectile = GetWorld()->SpawnActorDeferred<AHProjectile>(ProjectileClass, FTransform(SpawnRotation, SpawnLocation), Avatar, Cast<APawn>(Avatar), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
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
		
		// 모든 데이터 세팅 후 생성 완료 (이 시점에 BeginPlay 및 충돌 판정 시작)
		Projectile->FinishSpawning(FTransform(SpawnRotation, SpawnLocation));
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
