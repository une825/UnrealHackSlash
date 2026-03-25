#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillGem/HGemBase.h"
#include "HMainGem.generated.h"

class UHSupportGem;

/**
 * 액티브 스킬을 담당하는 메인 젬 클래스입니다.
 * 보조 젬을 장착할 수 있는 슬롯 리스트를 관리합니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHMainGem : public UHGemBase
{
	GENERATED_BODY()

public:
	/** 보조 젬을 슬롯에 추가합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|Main")
	bool AddSupportGem(UHSupportGem* InSupportGem);

	/** 특정 보조 젬을 슬롯에서 제거합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|Main")
	void RemoveSupportGem(UHSupportGem* InSupportGem);

	/** 장착된 모든 보조 젬 리스트를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem|Main")
	const TArray<UHSupportGem*>& GetSupportGems() const { return SupportGems; }

protected:
	/** 장착된 보조 젬들을 저장하는 슬롯 (최대 개수는 필요 시 제한 가능) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem|Main")
	TArray<TObjectPtr<UHSupportGem>> SupportGems;
};
