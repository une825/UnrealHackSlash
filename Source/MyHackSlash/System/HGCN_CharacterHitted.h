#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "HGCN_CharacterHitted.generated.h"

/**
 * 캐릭터 피격 시 시각적/청각적 피드백을 담당하는 GameplayCueNotify 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGCN_CharacterHitted : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UHGCN_CharacterHitted();

	/** GameplayCue가 실행될 때 호출되는 메인 함수입니다. */
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
