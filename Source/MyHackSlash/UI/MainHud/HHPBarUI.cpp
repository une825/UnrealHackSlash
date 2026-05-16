#include "UI/MainHud/HHPBarUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Unit/HBaseCharacter.h"
#include "System/HGlobalTextManager.h"

namespace
{
FText GetHPBarGlobalText(const UUserWidget* InWidget, FName InTextKey)
{
	if (InWidget)
	{
		if (const UGameInstance* GameInstance = InWidget->GetGameInstance())
		{
			if (UHGlobalTextManager* TextManager = GameInstance->GetSubsystem<UHGlobalTextManager>())
			{
				return TextManager->GetText(InTextKey);
			}
		}
	}

	return FText::FromName(InTextKey);
}
}

void UHHPBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(GetOwningPlayerPawn()))
	{
		// 델리게이트 바인딩 (HBaseCharacter에 선언되어 있으므로 모든 캐릭터 공용)
		Character->OnHPChanged.AddDynamic(this, &UHHPBarUI::UpdateHPBar);

		// 초기 값 설정
		UpdateHPBar(Character->GetCurrentHP(), Character->GetMaxHP());
		
		// 초기에는 즉시 설정
		CurrentHPPercent = TargetHPPercent;
		if (HPProgressBar)
		{
			HPProgressBar->SetPercent(CurrentHPPercent);
		}
	}
}

void UHHPBarUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!FMath::IsNearlyEqual(CurrentHPPercent, TargetHPPercent))
	{
		CurrentHPPercent = FMath::FInterpTo(CurrentHPPercent, TargetHPPercent, InDeltaTime, InterpSpeed);
		if (HPProgressBar)
		{
			HPProgressBar->SetPercent(CurrentHPPercent);
		}
	}
}

void UHHPBarUI::UpdateHPBar(float InCurrentHP, float InMaxHP)
{
	TargetHPPercent = (InMaxHP > 0) ? (InCurrentHP / InMaxHP) : 0.0f;

	if (HPText)
	{
		FText HPDisplayText = FText::Format(GetHPBarGlobalText(this, TEXT("UI.Common.ValuePairFormat")), FText::AsNumber(FMath::RoundToInt(InCurrentHP)), FText::AsNumber(FMath::RoundToInt(InMaxHP)));
		HPText->SetText(HPDisplayText);
	}
}
