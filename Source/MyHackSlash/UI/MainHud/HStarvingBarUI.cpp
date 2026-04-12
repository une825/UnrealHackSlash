#include "UI/MainHud/HStarvingBarUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Unit/HBaseCharacter.h"

void UHStarvingBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(GetOwningPlayerPawn()))
	{
		// 배고픔 변경 델리게이트 바인딩
		Character->OnHungerChanged.AddDynamic(this, &UHStarvingBarUI::UpdateHungerBar);

		// 초기 값 설정
		UpdateHungerBar(Character->GetCurrentHunger(), Character->GetMaxHunger());
		
		// 초기에는 즉시 설정
		CurrentHungerPercent = TargetHungerPercent;
		if (HungerProgressBar)
		{
			HungerProgressBar->SetPercent(CurrentHungerPercent);
		}
	}
}

void UHStarvingBarUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 부드러운 게이지 이동을 위한 보간
	if (!FMath::IsNearlyEqual(CurrentHungerPercent, TargetHungerPercent))
	{
		CurrentHungerPercent = FMath::FInterpTo(CurrentHungerPercent, TargetHungerPercent, InDeltaTime, InterpSpeed);
		if (HungerProgressBar)
		{
			HungerProgressBar->SetPercent(CurrentHungerPercent);
		}
	}
}

void UHStarvingBarUI::UpdateHungerBar(float InCurrentHunger, float InMaxHunger)
{
	TargetHungerPercent = (InMaxHunger > 0) ? (InCurrentHunger / InMaxHunger) : 0.0f;

	if (HungerText)
	{
		// "현재 / 최대" 형식으로 텍스트 설정
		FText HungerDisplayText = FText::Format(NSLOCTEXT("UI", "HungerFormat", "{0} / {1}"), 
			FText::AsNumber(FMath::RoundToInt(InCurrentHunger)), 
			FText::AsNumber(FMath::RoundToInt(InMaxHunger)));
		HungerText->SetText(HungerDisplayText);
	}
}
