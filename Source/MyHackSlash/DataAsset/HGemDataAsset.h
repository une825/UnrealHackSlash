#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HGlobalTypes.h"
#include "HGemDataAsset.generated.h"

class UTexture2D;
class UGameplayAbility;

/**
 * 개별 스킬 젬의 상세 데이터를 정의하는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FHGemData
{
	GENERATED_BODY()

public:
	/** @brief 데이터 식별자 (예: Fireball_01) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FName GemID = NAME_None;

	// --- 공통 데이터 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	FText GemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common", meta = (MultiLine = true))
	FText GemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	TObjectPtr<UTexture2D> GemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	HEGemCategory GemCategory = HEGemCategory::Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	HEGemType GemType = HEGemType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	HEElement GemElement = HEElement::Physical;

	/** @brief 젬의 티어 (1~3단계 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	int32 Tier = 1;

	/** @brief 3개가 모였을 때 업그레이드될 다음 티어 젬의 ID (없으면 NAME_None) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Common")
	FName NextTierGemID = NAME_None;

	// --- 메인 젬 전용 데이터 (Active) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Main")
	TSubclassOf<UGameplayAbility> SkillAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Main")
	HEShape GemShape = HEShape::Area;

	// --- 보조 젬 전용 데이터 (Passive) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Support")
	float EffectValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Support")
	FName SupportEffectTag = NAME_None;
};

/**
 * 모든 스킬 젬 데이터를 관리하는 컬렉션 데이터 에셋 클래스입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHGemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 등록된 모든 젬 데이터 리스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	TArray<FHGemData> GemList;

	/**
	 * @brief GemID를 이용하여 특정 젬 데이터를 찾습니다.
	 * @param InGemID 찾고자 하는 젬의 ID
	 * @param OutGemData 찾은 결과 데이터
	 * @return 데이터 존재 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Gem")
	bool FindGemData(const FName& InGemID, FHGemData& OutGemData) const
	{
		for (const FHGemData& Data : GemList)
		{
			if (Data.GemID == InGemID)
			{
				OutGemData = Data;
				return true;
			}
		}
		return false;
	}
};
