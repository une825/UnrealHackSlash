// Source/MyHackSlash/UI/ShopUI/HShopEntryUI.cpp

#include "UI/ShopUI/HShopEntryUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Mode/MyHackSlashPlayerController.h"
#include "Unit/Player/HPlayerState.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HGemInventoryComponent.h"
#include "DataAsset/HGemDataAsset.h"
#include "Mode/MyHackSlashGameMode.h"
#include "AbilitySystemComponent.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "UI/CommonUI/HGemIconUI.h"

void UHShopEntryUI::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	CachedData = Cast<UHShopEntryData>(ListItemObject);
	if (!CachedData.IsValid()) return;

	const FHShopRow& Row = CachedData->ItemRowData;

	if (TitleText) TitleText->SetText(Row.Title);
	if (DescText) DescText->SetText(Row.Description);
	if (CurrencyText) CurrencyText->SetText(FText::AsNumber(Row.Price));

	// 아이콘 설정
	if (Row.ItemType == EHShopItemType::SkillGem)
	{
		if (GemIconUI)
		{
			// ItemTag의 마지막 부분을 GemID로 추출 (예: Gem.Skill.FireBall -> FireBall)
			FString TagName = Row.ItemTag.ToString();
			int32 LastDotIndex;
			TagName.FindLastChar('.', LastDotIndex);
			FString GemIDStr = (LastDotIndex != INDEX_NONE) ? TagName.RightChop(LastDotIndex + 1) : TagName;

			GemIconUI->SetGemInfo(FName(*GemIDStr), 1); // 상점표는 기본 1티어
			GemIconUI->SetVisibility(ESlateVisibility::Visible);
		}
		if (IconImage)
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (GemIconUI)
		{
			GemIconUI->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (IconImage)
		{
			IconImage->SetVisibility(ESlateVisibility::Visible);
			IconImage->SetBrushFromSoftTexture(Row.Icon);
		}
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.Clear();
		SelectButton->OnClicked.AddDynamic(this, &UHShopEntryUI::OnSelectButtonClicked);
	}
}

void UHShopEntryUI::OnSelectButtonClicked()
{
	if (!CachedData.IsValid()) return;

	AMyHackSlashPlayerController* PC = Cast<AMyHackSlashPlayerController>(GetOwningPlayer());
	if (!PC) return;

	// TODO: 구매 완료 사운드 및 이펙트 호출
	PC->RequestPurchaseShopItem(CachedData->RowName);
}
