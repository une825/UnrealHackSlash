#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataAsset/HSelectAbilityData.h"
#include "DataAsset/HGemDataAsset.h"
#include "HSelectAbilityManager.generated.h"

/**
 * @brief 보상 선택 및 지급 시스템을 총괄하는 매니저 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSelectAbilityManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief 시스템 초기화를 위해 확률 데이터 에셋과 보상 테이블을 설정합니다.
	 * @param InGradeDataAsset 등급별 확률 데이터 에셋
	 * @param InRewardTable 보상 옵션 데이터 테이블
	 * @param InGemCollection 젬 데이터 컬렉션 에셋
	 */
	UFUNCTION(BlueprintCallable, Category = "Select Ability")
	void InitializeManager(UHSelectAbilityGradeDataAsset* InGradeDataAsset, UDataTable* InRewardTable, UHGemDataAsset* InGemCollection);

	/**
	 * @brief 무작위로 3개의 보상 선택지를 생성합니다.
	 * @param OutOptions 생성된 3개의 보상 옵션 배열
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Select Ability")
	bool GetRandomRewardOptions(TArray<FHRewardOptionData>& OutOptions);

	/**
	 * @brief 선택된 보상을 실제로 적용합니다.
	 * @param InSelectedOption 유저가 선택한 보상 데이터
	 */
	UFUNCTION(BlueprintCallable, Category = "Select Ability")
	void ExecuteReward(const FHRewardOptionData& InSelectedOption);

protected:
	/** @brief 확률 기반으로 이번 팝업의 등급을 결정합니다. */
	EHAbilityGrade RollGrade() const;

private:
	/** @brief 등급 확률 설정 데이터 에셋 */
	UPROPERTY()
	TObjectPtr<UHSelectAbilityGradeDataAsset> GradeDataAsset;

	/** @brief 보상 데이터 테이블 */
	UPROPERTY()
	TObjectPtr<UDataTable> RewardDataTable;

	/** @brief 모든 젬 데이터가 담긴 컬렉션 에셋 */
	UPROPERTY()
	TObjectPtr<UHGemDataAsset> GemCollection;
};
