#include "GAS/HExecutionCalculation_Damage.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

/** @brief 속성 캡처를 위한 정적 구조체 */
struct HDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalMultiplier);

	HDamageStatics()
	{
		// 공격자의 속성(Source)을 캡처 대상으로 정의합니다.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHCharacterAttributeSet, AttackDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHCharacterAttributeSet, CriticalRate, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHCharacterAttributeSet, CriticalMultiplier, Source, false);
	}
};

static const HDamageStatics& DamageStatics()
{
	static HDamageStatics DStatics;
	return DStatics;
}

UHExecutionCalculation_Damage::UHExecutionCalculation_Damage()
{
	// 이 ExecutionCalculation에서 캡처할 속성들을 등록합니다.
	RelevantAttributesToCapture.Add(DamageStatics().AttackDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalRateDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalMultiplierDef);
}

void UHExecutionCalculation_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& ExecutionOutputs) const
{
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	if (!SourceASC || !TargetASC) return;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 1. 공격자 속성값 캡처
	float AttackDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackDamageDef, EvaluationParameters, AttackDamage);

	float CriticalRate = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalRateDef, EvaluationParameters, CriticalRate);

	float CriticalMultiplier = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalMultiplierDef, EvaluationParameters, CriticalMultiplier);

	// 2. 추가 데미지 배율 (SetByCaller 등으로부터 가져올 수 있음)
	// 예: 스킬 자체의 고유 데미지 배율이 있다면 여기서 곱해줌
	float DamageMultiplier = 1.0f;
	FGameplayTag MultiplierTag = FGameplayTag::RequestGameplayTag(TEXT("Data.DamageMultiplier"));
	if (Spec.GetSetByCallerMagnitude(MultiplierTag, false, 0.0f) > 0.0f)
	{
		DamageMultiplier = Spec.GetSetByCallerMagnitude(MultiplierTag);
	}

	// 3. 최종 데미지 산출 (기본 공격력 * 배율)
	float FinalDamage = AttackDamage * DamageMultiplier;

	// 4. 치명타 판정 (GA에서 이미 판별하여 태그를 심어주었다고 가정)
	const FGameplayTag CriticalTag = FGameplayTag::RequestGameplayTag(TEXT("Effect.Critical"));
	if (Spec.GetDynamicAssetTags().HasTag(CriticalTag) || Spec.CapturedSourceTags.GetAggregatedTags()->HasTag(CriticalTag))
	{
		FinalDamage *= CriticalMultiplier;
	}

	// 5. 계산된 최종 데미지를 'Damage' 메타 어트리뷰트로 출력
	if (FinalDamage > 0.0f)
	{
		ExecutionOutputs.AddOutputModifier(FGameplayModifierEvaluatedData(UHCharacterAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
	}
}
