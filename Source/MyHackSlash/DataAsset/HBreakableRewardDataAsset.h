#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HBreakableRewardDataAsset.generated.h"

/**
 * 드랍될 보상 정보 구조체
 */
USTRUCT(BlueprintType)
struct FHBreakableRewardEntry
{
	GENERATED_BODY()

	/** @brief 드랍될 액터 클래스 (코인, 자석 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TSubclassOf<AActor> RewardClass;

	/** @brief 드랍 가중치 (전체 가중치 합 중 비중) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float Weight = 1.0f;

	/** @brief 드랍 개수 최소값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 MinAmount = 1;

	/** @brief 드랍 개수 최대값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 MaxAmount = 1;
};

/**
 * 파괴 가능한 오브젝트의 전리품 테이블을 정의하는 데이터 에셋입니다.
 */
UCLASS()
class MYHACKSLASH_API UHBreakableRewardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 가능한 보상 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TArray<FHBreakableRewardEntry> Rewards;

	/** @brief 보상을 하나라도 드랍할 전체 확률 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TotalDropChance = 0.5f;

	/** @brief 가중치를 기반으로 랜덤 보상을 선택합니다. */
	bool GetRandomReward(FHBreakableRewardEntry& OutReward) const;
};
