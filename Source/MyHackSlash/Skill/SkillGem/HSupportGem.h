#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillGem/HGemBase.h"
#include "HSupportGem.generated.h"

class UHMainGem;

/**
 * 메인 젬의 성능을 강화하는 보조 젬 클래스입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHSupportGem : public UHGemBase
{
	GENERATED_BODY()

public:
	/** 
	 * 디자인 문서의 장착 매칭 로직을 구현합니다.
	 * 보조 젬의 타입이 Common이거나 메인 젬의 타입과 일치해야 합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "Gem|Support")
	bool CanAttach(const UHMainGem* InMainGem) const;
};
