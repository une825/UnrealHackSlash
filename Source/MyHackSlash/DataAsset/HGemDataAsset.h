#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "System/HGlobalTypes.h"
#include "HGemDataAsset.generated.h"

class UTexture2D;
class UGameplayAbility;

/**
 * @brief 보조 젬이 제공하는 개별 효과를 정의하는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FHSupportEffect
{
	GENERATED_BODY()

public:
	/** @brief 적용할 파라미터 태그 (예: Data.ProjectileCount, Data.DamageMultiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Support")
	FGameplayTag SupportTag;

	/** @brief 보너스 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Support")
	float ModifierValue = 0.0f;
};

/**
 * @brief 개별 스킬 젬의 상세 데이터를 정의하는 구조체입니다. (DataTable용)
 */
USTRUCT(BlueprintType)
struct FHGemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** @brief 베이스 식별자 (예: FireBall) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FName GemID = NAME_None;

	/** @brief 젬의 티어 (1~3단계 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	int32 Tier = 1;

	/** @brief 발동할 어빌리티 태그 이름 (예: Ability.FireBall) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FName AbilityTagName = NAME_None;

	/** @brief UI 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FText Name;

	/** @brief UI 표시 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem", meta = (MultiLine = true))
	FText Description;

	/** @brief UI 표시 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	HEGemCategory GemCategory = HEGemCategory::Main;

	/** @brief 젬의 등급 (Common, Rare 등 - 기존 유지) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	HEGemType GemType = HEGemType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	HEElement GemElement = HEElement::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	HEShape GemShape = HEShape::Area;

	/** @brief 보조 젬이 제공하는 효과 리스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Support")
	TArray<FHSupportEffect> SupportEffects;
};

/**
 * @brief 전체 젬 데이터를 관리하는 DataAsset 클래스입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHGemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 실제 젬 데이터가 담긴 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GemData")
	TObjectPtr<UDataTable> GemDataTable;

	/** 
	 * @brief 실행할 어빌리티 클래스 
	 * @key FName : AbilityTagName
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GemData")
	TMap<FName, TSubclassOf<UGameplayAbility>> SkillAbilityClassMap;

	/**
	 * @brief GemID(FullID)를 기반으로 젬 데이터를 검색합니다.
	 * @param InFullID 검색할 Row 이름 (예: FireBall_T1)
	 * @param OutData 찾은 데이터 결과
	 * @return 데이터 발견 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "GemData")
	bool FindGemData(FName InFullID, FHGemData& OutData) const;

	/**
	 * @brief 모든 젬 데이터를 가져옵니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "GemData")
	void GetAllGemData(TArray<FHGemData>& OutDataList) const;

	/**
	 * @brief AbilityTagName을 사용하여 등록된 어빌리티 클래스를 찾습니다.
	 * @param InTagName 찾을 어빌리티 태그 이름 (예: Ability.FireBall)
	 * @return 찾은 어빌리티 클래스 (없으면 nullptr)
	 */
	UFUNCTION(BlueprintCallable, Category = "GemData")
	TSubclassOf<UGameplayAbility> FindAbilityClassByTagName(FName InTagName) const;

	/**
	 * @brief 현재 젬의 다음 티어 데이터를 찾습니다. (GemID + (Tier + 1) 조합 사용)
	 * @param InCurrentData 현재 젬 데이터
	 * @param OutNextData 찾은 다음 티어 데이터
	 * @return 발견 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "GemData")
	bool FindNextTierGemData(const FHGemData& InCurrentData, FHGemData& OutNextData) const;
};
