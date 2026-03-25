#include "GA/HGA_FireBall.h"
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
			BaseCharacter->GetCurrentStat().AttackSpeedRate
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

	FVector SpawnLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 100.0f;
	FRotator SpawnRotation = Avatar->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);

	AHProjectile* Projectile = GetWorld()->SpawnActor<AHProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->SetDamage(ProjectileDamage);
		Projectile->SetElement(HEElement::Fire);

		if (AHBaseCharacter* BaseCharacter = Cast<AHBaseCharacter>(Avatar))
		{
			if (BaseCharacter->GetUnitProfileData())
			{
				Projectile->SetOwningUnitType(BaseCharacter->GetUnitProfileData()->UnitType);
			}
		}
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
