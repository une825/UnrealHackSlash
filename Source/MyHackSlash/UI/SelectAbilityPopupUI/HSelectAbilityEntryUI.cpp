#include "UI/SelectAbilityPopupUI/HSelectAbilityEntryUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "System/HSelectAbilityManager.h"
#include <System/HUIManager.h>

void UHSelectAbilityEntryUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UHSelectAbilityEntryUI::OnSelectButtonClicked);
	}
}

void UHSelectAbilityEntryUI::NativeOnListItemObjectSet(UObject* InListItemObject)
{
	if (UHSelectAbilityData* AbilityData = Cast<UHSelectAbilityData>(InListItemObject))
	{
		CurrentData = AbilityData;
		const FHRewardOptionData& RewardData = AbilityData->RewardOptionData;

		// 텍스트 설정
		if (TitleText)
		{
			TitleText->SetText(RewardData.Title);
		}

		if (DescText)
		{
			DescText->SetText(RewardData.Description);
		}

		// 아이콘 설정 (소프트 레퍼런스 비동기 로드)
		if (IconImage)
		{
			if (!RewardData.Icon.IsNull())
			{
				IconImage->SetVisibility(ESlateVisibility::Visible);
				if (RewardData.Icon.IsValid())
				{
					IconImage->SetBrushFromTexture(RewardData.Icon.Get());
				}
				else
				{
					IconImage->SetBrushFromSoftTexture(RewardData.Icon);
				}
			}
			else
			{
				IconImage->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		// 등급별 오버레이 설정
		if (SilverGrade) SilverGrade->SetVisibility(ESlateVisibility::Collapsed);
		if (GoldGrade) GoldGrade->SetVisibility(ESlateVisibility::Collapsed);
		if (DiaGrade) DiaGrade->SetVisibility(ESlateVisibility::Collapsed);

		switch (RewardData.Grade)
		{
		case EHAbilityGrade::Silver:
			if (SilverGrade) SilverGrade->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;
		case EHAbilityGrade::Gold:
			if (GoldGrade) GoldGrade->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;
		case EHAbilityGrade::Dia:
			if (DiaGrade) DiaGrade->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;
		}
	}
}

void UHSelectAbilityEntryUI::OnSelectButtonClicked()
{
	if (!CurrentData.IsValid()) return;

	if (UHSelectAbilityManager* Manager = GetGameInstance()->GetSubsystem<UHSelectAbilityManager>())
	{
		// 1. 보상 실행
		Manager->ExecuteReward(CurrentData->RewardOptionData);

		// 2. 팝업 닫기
		if (UUserWidget* ParentPopup = GetTypedOuter<UUserWidget>())
		{
			if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
			{
				UIManager->HideWidget(ParentPopup);
			}
		}
	}
}
