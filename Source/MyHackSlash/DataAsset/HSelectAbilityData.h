#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "HSelectAbilityData.generated.h"

/**
 * @brief 보상 선택지의 등급을 정의합니다.
 */
UENUM(BlueprintType)
enum class EHAbilityGrade : uint8
{
	Silver UMETA(DisplayName = "Silver"),
	Gold   UMETA(DisplayName = "Gold"),
	Dia    UMETA(DisplayName = "Dia")
};

/**
 * @brief 보상의 종류를 정의합니다.
 */
UENUM(BlueprintType)
enum class EHRewardType : uint8
{
	GetSkillGem UMETA(DisplayName = "GetSkillGem"),
	GetGold     UMETA(DisplayName = "GetGold"),
	GetReroll   UMETA(DisplayName = "GetReroll")
};

/**
 * @brief 등급별 등장 확률을 정의하기 위한 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FHGradeProbability
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	EHAbilityGrade Grade = EHAbilityGrade::Silver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Probability = 0.0f;
};

/**
 * @brief 각 보상 선택지의 상세 데이터를 정의하는 DataTable용 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FHRewardOptionData : public FTableRowBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief 보상의 아이콘을 가져옵니다. RewardType이 GetSkillGem이면 젬 아이콘을 검색하고, 아니면 설정된 아이콘을 반환합니다.
	 * @param WorldContextObject 월드 컨텍스트
	 * @return 아이콘 텍스처 (없으면 nullptr)
	 */
	class UTexture2D* GetRewardIcon(const UObject* WorldContextObject) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	EHAbilityGrade Grade = EHAbilityGrade::Silver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	EHRewardType RewardType = EHRewardType::GetSkillGem;

	/** @brief 스킬 젬 이름 등 대상의 식별자 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	FName TargetID = NAME_None;

	/** @brief UI에 표시될 제목 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	FText Title;

	/** @brief 성급 (Tier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	int32 Tier = 1;

	/** @brief 수량 (Amount) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	int32 Amount = 1;

	/** @brief UI에 표시될 설명 (선택 사항) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	FText Description;

private:
	/** @brief UI에 표시될 아이콘 (선택 사항) */
	UPROPERTY(EditAnywhere, Category = "Select Ability")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * @brief 전체 등급 확률 설정을 저장하는 DataAsset 클래스입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHSelectAbilityGradeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select Ability")
	TArray<FHGradeProbability> GradeProbabilities;

	/**
	 * @brief 현재 설정된 확률 리스트를 반환합니다.
	 * @param OutProbabilities 결과 배열
	 */
	UFUNCTION(BlueprintCallable, Category = "Select Ability")
	void GetGradeProbabilities(TArray<FHGradeProbability>& OutProbabilities) const
	{
		OutProbabilities = GradeProbabilities;
	}
};
