#include "UI/MainHud/HExpBarUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHExpBarUI::NativeConstruct()
{
	Super::NativeConstruct();

}

void UHExpBarUI::UpdateExpBar(float CurrentExp, float MaxExp)
{
	if (ExpProgressBar)
	{
		float ExpPercent = (MaxExp > 0) ? (CurrentExp / MaxExp) : 0.0f;
		ExpProgressBar->SetPercent(ExpPercent);
	}

	if (ExpText)
	{
		FText ExpDisplayText = FText::Format(NSLOCTEXT("UI", "ExpFormat", "{0} / {1}"), FText::AsNumber(FMath::RoundToInt(CurrentExp)), FText::AsNumber(FMath::RoundToInt(MaxExp)));
		ExpText->SetText(ExpDisplayText);
	}
}
