#include "UI/SelectAbilityPopupUI/HSelectAbilityEntryUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
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
	if (UHSelectAbilityData* Data = Cast<UHSelectAbilityData>(InListItemObject))
	{
		if (TitleText) TitleText->SetText(Data->Title);
		if (DescText) DescText->SetText(Data->Description);
	}
}

void UHSelectAbilityEntryUI::OnSelectButtonClicked()
{
	// 1. 게임 시간 재개
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 2. 팝업 닫기.
	if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		if (UUserWidget* PopupUI = UIManager->GetWidgetByName(TEXT("SelectAbilityPopupUI")))
		{
			UIManager->HideWidget(PopupUI);
		}
	}
}
