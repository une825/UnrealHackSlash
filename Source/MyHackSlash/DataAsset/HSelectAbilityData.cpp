#include "DataAsset/HSelectAbilityData.h"
#include "System/HFunctionLibrary.h"
#include "Engine/Texture2D.h"

UTexture2D* FHRewardOptionData::GetRewardIcon(const UObject* WorldContextObject) const
{
	// 보상 종류가 스킬 젬인 경우, GemID와 Tier를 사용하여 젬 컬렉션에서 아이콘을 가져옴
	if (RewardType == EHRewardType::GetSkillGem)
	{
		return UHFunctionLibrary::GetGemIcon(WorldContextObject, TargetID, Tier);
	}

	// 그 외의 경우 (골드, 리롤 등) 혹은 젬 아이콘 검색 실패 시, 직접 설정된 Icon을 반환
	if (Icon.IsValid())
	{
		return Icon.Get();
	}

	return Icon.LoadSynchronous();
}
