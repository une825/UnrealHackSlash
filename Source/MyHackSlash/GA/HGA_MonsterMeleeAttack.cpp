// Source/MyHackSlash/GA/HGA_MonsterMeleeAttack.cpp

#include "GA/HGA_MonsterMeleeAttack.h"
#include "Unit/HBaseCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"

UHGA_MonsterMeleeAttack::UHGA_MonsterMeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_MonsterMeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AHBaseCharacter* Character = Cast<AHBaseCharacter>(ActorInfo->AvatarActor.Get());
	if (Character == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 캐릭터 상태 업데이트 (공격 중임을 알림)
	Character->ProcessAttack();

	// 2. 몬스터 캐릭터가 소유한 공격 몽타주 가져오기
	UAnimMontage* AttackMontage = Character->GetAttackMontage();
	if (AttackMontage == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 공격 속도 배율 가져오기
	float AttackSpeedRate = Character->GetAttackSpeedRate();

	// 4. PlayMontageAndWait 태스크 생성 및 실행
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("Attack"),
		AttackMontage,
		AttackSpeedRate,
		NAME_None,
		false,
		1.0f
	);

	if (PlayMontageTask)
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &UHGA_MonsterMeleeAttack::OnMontageEnded);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UHGA_MonsterMeleeAttack::OnMontageEnded);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UHGA_MonsterMeleeAttack::OnMontageEnded);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UHGA_MonsterMeleeAttack::OnMontageEnded);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UHGA_MonsterMeleeAttack::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_MonsterMeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 어빌리티 종료 시 캐릭터에게 공격이 끝났음을 알림 (AI 태스크 종료 유도)
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(ActorInfo->AvatarActor.Get()))
		{
			Character->AttackEnd(nullptr, bWasCancelled);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
