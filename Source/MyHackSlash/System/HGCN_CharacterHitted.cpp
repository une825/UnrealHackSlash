#include "System/HGCN_CharacterHitted.h"
#include "Unit/HBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"

UHGCN_CharacterHitted::UHGCN_CharacterHitted()
{
	// 이 클래스가 어떤 GameplayCue 태그와 연결될지 블루프린트에서 설정하거나 
	// 코드에서 기본값을 줄 수 있습니다.
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
	
	// 치명타 여부 확인 (전달받은 태그 확인)
	bool bIsCritical = Parameters.AggregatedSourceTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Effect.Critical")));
	
	TargetCharacter->ShowDamageText(Parameters.RawMagnitude, bIsCritical, DamageCauser);

	// 카메라 흔들림 (플레이어인 경우)
	if (TargetCharacter->IsPlayerControlled())
	{
		TargetCharacter->HandleCameraShake(Parameters.RawMagnitude);
	}

	return bResult;
}

