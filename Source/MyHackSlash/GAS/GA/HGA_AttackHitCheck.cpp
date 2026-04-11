// Source/MyHackSlash/GA/HGA_AttackHitCheck.cpp

#include "GAS/GA/HGA_AttackHitCheck.h"
#include "GAS/GA/AT/HAT_HitCheckTrace.h"
#include "GAS/GA/TA/HTA_HitCheckTrace.h"
#include "Unit/HBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

UHGA_AttackHitCheck::UHGA_AttackHitCheck()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_AttackHitCheck::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 공격자 레벨 캡처 (TriggerEventData의 Instigator로부터)
	AttackerLevel = 1.0f; // 기본값
	if (TriggerEventData && TriggerEventData->Instigator)
	{
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(TriggerEventData->Instigator.Get()));
		if (SourceASC)
		{
			const UHCharacterAttributeSet* AttributeSet = SourceASC->GetSet<UHCharacterAttributeSet>();
			if (AttributeSet)
			{
				AttackerLevel = AttributeSet->GetLevel();
			}
		}
	}

	// 트레이스 태스크 생성 및 실행
	UHAT_HitCheckTrace* HitCheckTask = UHAT_HitCheckTrace::CreateTask(this, AHTA_HitCheckTrace::StaticClass());
	if (HitCheckTask)
	{
		HitCheckTask->OnTargetDataReady.AddDynamic(this, &UHGA_AttackHitCheck::OnTraceResultCallback);
		HitCheckTask->ReadyForActivation();
	}
	else
	{
		// 태스크 생성 실패 시 즉시 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}
void UHGA_AttackHitCheck::OnTraceResultCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	// 1. 공격자(Source)의 어빌리티 시스템 및 어트리뷰트 가져오기
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	const UHCharacterAttributeSet* SourceAttribute = SourceASC ? SourceASC->GetSet<UHCharacterAttributeSet>() : nullptr;

	if (!SourceAttribute || !AttackDamageEffect)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 2. 타겟 데이터를 순회하며 각 대상에게 이펙트 적용
	for (int32 i = 0; i < TargetDataHandle.Num(); ++i)
	{
		if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetDataHandle, i))
		{
			FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, i);
			AActor* TargetActor = HitResult.GetActor();

			if (TargetActor)
			{
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
				if (TargetASC)
				{
					// 3. 진영 판별 (아군 오사 방지)
					AHBaseCharacter* SourceCharacter = Cast<AHBaseCharacter>(SourceASC->GetAvatarActor());
					AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(TargetActor);

					if (SourceCharacter && TargetCharacter)
					{
						if (SourceCharacter->GetUnitProfileData() && TargetCharacter->GetUnitProfileData())
						{
							// 동일한 진영이면 공격 건너뜀
							if (SourceCharacter->GetUnitProfileData()->UnitType == TargetCharacter->GetUnitProfileData()->UnitType)
							{
								continue;
							}
						}
					}

					// 4. 데미지 이펙트 생성 및 적용
					FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
					ContextHandle.AddHitResult(HitResult);

					FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AttackDamageEffect, GetAbilityLevel(), ContextHandle);
					if (SpecHandle.IsValid())
					{
						// 치명타 여부 판별 (데미지 배율 적용은 GE의 ExecutionCalculation에서 담당함)
						if (SourceCharacter)
						{
							const float CritChance = SourceAttribute->GetCriticalRate();
							if (FMath::FRandRange(0.0f, 100.0f) <= CritChance)
							{
								// 치명타 정보를 태그로 심어서 전달 (GameplayCue 및 ExecutionCalculation에서 인식 가능)
								SpecHandle.Data->DynamicAssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Effect.Critical")));
							}
						}

						// 최종 데미지 적용 (ExecutionCalculation이 설정된 GE를 적용)
						SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

						// 5. 명중 사운드 재생
						if (HitSound)
						{
							UGameplayStatics::PlaySoundAtLocation(this, HitSound, TargetActor->GetActorLocation());
						}
					}
				}
			}
		}
	}

	// 작업 완료 후 능력 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UHGA_AttackHitCheck::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
