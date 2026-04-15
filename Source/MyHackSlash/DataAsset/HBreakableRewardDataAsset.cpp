#include "DataAsset/HBreakableRewardDataAsset.h"

bool UHBreakableRewardDataAsset::GetRandomReward(FHBreakableRewardEntry& OutReward) const
{
	if (Rewards.Num() == 0) return false;

	// 전체 확률 체크
	if (FMath::FRand() > TotalDropChance) return false;

	// 가중치 합산
	float TotalWeight = 0.0f;
	for (const auto& Entry : Rewards)
	{
		TotalWeight += Entry.Weight;
	}

	if (TotalWeight <= 0.0f) return false;

	// 랜덤값 기반 선택
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (const auto& Entry : Rewards)
	{
		AccumulatedWeight += Entry.Weight;
		if (RandomValue <= AccumulatedWeight)
		{
			OutReward = Entry;
			return true;
		}
	}

	return false;
}
