#include "UI/CommonUI/HGemIconUI.h"
#include "Components/Image.h"
#include "DataAsset/HGemDataAsset.h"
#include "Mode/MyHackSlashGameMode.h"

void UHGemIconUI::SetGemInfo(FName GemID, int32 Tier)
{
	// 1. 젬 데이터 에셋 찾기
	UHGemDataAsset* GemCollection = nullptr;
	if (AMyHackSlashGameMode* GM = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GemCollection = GM->GetGemCollectionDataAsset();
	}

	if (GemCollection)
	{
		// FullID 생성 (예: FireBall_T1)
		FName FullID = FName(*FString::Printf(TEXT("%s_T%d"), *GemID.ToString(), Tier));
		FHGemData GemData;

		if (GemCollection->FindGemData(FullID, GemData))
		{
			// 아이콘 설정
			if (GemIconImage)
			{
				GemIconImage->SetBrushFromSoftTexture(GemData.Icon);
			}
		}
	}

	// 2. 티어 별 표시 설정 (Tier 1: 1개, Tier 2: 2개, Tier 3: 3개)
	if (TierStarImage_0) TierStarImage_0->SetVisibility(Tier >= 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (TierStarImage_1) TierStarImage_1->SetVisibility(Tier >= 2 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (TierStarImage_2) TierStarImage_2->SetVisibility(Tier >= 3 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
