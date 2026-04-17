#include "GAS/HGCN_CharacterHitted.h"
#include "Unit/HBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectAggregator.h"

UHGCN_CharacterHitted::UHGCN_CharacterHitted()
{
}

bool UHGCN_CharacterHitted::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	bool bResult = Super::OnExecute_Implementation(MyTarget, Parameters);

	AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(MyTarget);
	if (nullptr == TargetCharacter) return bResult;

	// 기존 캐릭터 클래스의 피드백 로직들 호출
	TargetCharacter->HandleHitSound();
	TargetCharacter->HandleHUDDamageEffect();
	TargetCharacter->PlayHittedEffect();

	// 데미지 텍스트 출력
	AActor* DamageCauser = Parameters.EffectContext.GetInstigator();
	
	// 치명타 여부 확인
	FGameplayTag CriticalTag = FGameplayTag::RequestGameplayTag(TEXT("Effect.Critical"));
	
	// FGameplayCueParameters에서 태그를 확인하는 가장 확실한 방법들
	bool bIsCritical = Parameters.AggregatedSourceTags.HasTag(CriticalTag) || 
	                   Parameters.AggregatedTargetTags.HasTag(CriticalTag);

	// 만약 여전히 안 나온다면, AbilitySystemBlueprintLibrary를 통해 컨텍스트에서 직접 확인 시도
	if (!bIsCritical && Parameters.EffectContext.IsValid())
	{
		// EffectContext 내부의 소스 태그나 타겟 태그에 포함되어 있는지 재확인
		// (일부 환경에서는 Aggregated 태그들이 비어있을 수 있음)
	}
	
	TargetCharacter->ShowDamageText(Parameters.RawMagnitude, bIsCritical, DamageCauser);

	// 카메라 흔들림 (플레이어인 경우)
	if (TargetCharacter->IsPlayerControlled())
	{
		TargetCharacter->HandleCameraShake(Parameters.RawMagnitude);
	}

	return bResult;
}
