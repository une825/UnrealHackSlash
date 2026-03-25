#include "UI/MainHud/HExpBarUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Unit/Player/HPlayerCharacter.h"

void UHExpBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetOwningPlayerPawn()))
	{
		// 델리게이트 바인딩
		PlayerCharacter->OnExpChanged.AddDynamic(this, &UHExpBarUI::UpdateExpBar);

		// 초기 값 설정
		UpdateExpBar(PlayerCharacter->GetLevel(), PlayerCharacter->GetCurrentExp(), PlayerCharacter->GetMaxExp());

		// 초기에는 즉시 설정
		CurrentExpPercent = TargetExpPercent;
		if (ExpProgressBar)
		{
			ExpProgressBar->SetPercent(CurrentExpPercent);
		}
	}
}

void UHExpBarUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!FMath::IsNearlyEqual(CurrentExpPercent, TargetExpPercent))
	{
		CurrentExpPercent = FMath::FInterpTo(CurrentExpPercent, TargetExpPercent, InDeltaTime, InterpSpeed);
		if (ExpProgressBar)
		{
			ExpProgressBar->SetPercent(CurrentExpPercent);
		}
	}
}

void UHExpBarUI::UpdateExpBar(const int InLevel, const float InCurrentExp, const float InMaxExp)
{
	TargetExpPercent = (InMaxExp > 0) ? (InCurrentExp / InMaxExp) : 0.0f;

	if (ExpText)
	{
		FText ExpDisplayText = FText::Format(NSLOCTEXT("UI", "ExpFormat", "{0} / {1}"), FText::AsNumber(FMath::RoundToInt(InCurrentExp)), FText::AsNumber(FMath::RoundToInt(InMaxExp)));
		ExpText->SetText(ExpDisplayText);
	}

	if (LevelText)
	{
		FText LevelDisplayText = FText::Format(NSLOCTEXT("UI", "LevelFormat", "Lv. {0}"), FText::AsNumber(InLevel));
		LevelText->SetText(LevelDisplayText);
	}
}
