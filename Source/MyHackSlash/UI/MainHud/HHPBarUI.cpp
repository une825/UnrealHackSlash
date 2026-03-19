#include "UI/MainHud/HHPBarUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Unit/HBaseCharacter.h"

void UHHPBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(GetOwningPlayerPawn()))
	{
		// 델리게이트 바인딩 (HBaseCharacter에 선언되어 있으므로 모든 캐릭터 공용)
		Character->OnHPChanged.AddDynamic(this, &UHHPBarUI::UpdateHPBar);

		// 초기 값 설정
		const FUnitStatRow& CurrentStat = Character->GetCurrentStat();
		UpdateHPBar(Character->GetCurrentHP(), CurrentStat.MaxHP);
	}
}

void UHHPBarUI::UpdateHPBar(float InCurrentHP, float InMaxHP)
{
	if (HPProgressBar)
	{
		float HPPercent = (InMaxHP > 0) ? (InCurrentHP / InMaxHP) : 0.0f;
		HPProgressBar->SetPercent(HPPercent);
	}

	if (HPText)
	{
		FText HPDisplayText = FText::Format(NSLOCTEXT("UI", "HPFormat", "{0} / {1}"), FText::AsNumber(FMath::RoundToInt(InCurrentHP)), FText::AsNumber(FMath::RoundToInt(InMaxHP)));
		HPText->SetText(HPDisplayText);
	}
}
