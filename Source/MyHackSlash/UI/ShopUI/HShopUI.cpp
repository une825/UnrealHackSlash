// Source/MyHackSlash/UI/ShopUI/HShopUI.cpp

#include "UI/ShopUI/HShopUI.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Unit/Player/HPlayerState.h"

void UHShopUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 보유 재화 초기화 및 바인딩
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AHPlayerState* PS = PC->GetPlayerState<AHPlayerState>())
		{
			RefreshCurrency(PS->GetCurrentGold());
			PS->OnGoldChanged.AddUObject(this, &UHShopUI::RefreshCurrency);
		}
	}

	InitWaveInfo();
}

void UHShopUI::RefreshCurrency(int32 InNewCurrency)
{
	if (CurrencyText)
	{
		CurrencyText->SetText(FText::AsNumber(InNewCurrency));
	}
}

void UHShopUI::InitWaveInfo()
{
	if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
	{
		if (WaveText)
		{
			WaveText->SetText(FText::Format(FText::FromString(TEXT("Wave {0}")), FText::AsNumber(WaveManager->GetCurrentWaveDisplayIndex())));
		}

		if (WaveTypeText)
		{
			// 웨이브 타입에 따른 설명 설정 (예: 일반, 보스 등)
			WaveTypeText->SetText(FText::FromString(TEXT("상점 준비 단계")));
		}
	}
}
