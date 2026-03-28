#include "UI/MainHud/HMainHudUI.h"
#include "Components/TextBlock.h"
#include "Unit/Player/HPlayerState.h"
#include "Kismet/GameplayStatics.h"

void UHMainHudUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 플레이어 스테이트를 찾아서 골드 변경 델리게이트에 바인딩
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AHPlayerState* PS = PC->GetPlayerState<AHPlayerState>())
		{
			// 현재 골드 수치로 즉시 초기화
			RefreshGold(PS->GetCurrentGold());

			// 값이 바뀔 때마다 실행되도록 바인딩
			PS->OnGoldChanged.AddUObject(this, &UHMainHudUI::RefreshGold);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HMainHudUI: Main HUD Widget Constructed and Gold bound!"));
}

void UHMainHudUI::RefreshGold(int32 InNewGold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::Format(FText::FromString(TEXT("보유 골드 : {0}")), FText::AsNumber(InNewGold)));
	}
}
