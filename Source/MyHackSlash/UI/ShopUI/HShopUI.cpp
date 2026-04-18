// Source/MyHackSlash/UI/ShopUI/HShopUI.cpp

#include "UI/ShopUI/HShopUI.h"
#include "UI/ShopUI/HShopEntryUI.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Unit/Player/HPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "System/HUIManager.h"
#include "System/HWaveManager.h"
#include "DataAsset/HShopRow.h"

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

	// 버튼 이벤트 바인딩
	if (NextWaveButton)
	{
		NextWaveButton->OnClicked.AddDynamic(this, &UHShopUI::OnClickNextWave);
	}

	InitWaveInfo();
	PopulateShopItems();
}

void UHShopUI::RefreshCurrency(int32 InNewCurrency)
{
	if (CurrencyText)
	{
		CurrencyText->SetText(FText::AsNumber(InNewCurrency));
	}
}

void UHShopUI::PopulateShopItems()
{
	if (!ShopTileView) return;
	ShopTileView->ClearListItems();

	UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>();
	if (!WaveManager) return;

	const FHWaveData& CurrentWave = WaveManager->GetCurrentWaveData();
	UDataTable* ShopTable = CurrentWave.ShopRewardTable;
	if (!ShopTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHShopUI: ShopRewardTable is null in current wave!"));
		return;
	}

	// 테이블의 모든 행을 가져와서 배치
	TArray<FName> RowNames = ShopTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (FHShopRow* ItemData = ShopTable->FindRow<FHShopRow>(RowName, TEXT("UHShopUI::PopulateShopItems")))
		{
			UHShopEntryData* EntryData = NewObject<UHShopEntryData>(this);
			EntryData->ItemRowData = *ItemData;
			EntryData->RowName = RowName;

			ShopTileView->AddItem(EntryData);
		}
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

void UHShopUI::OnClickNextWave()
{
	// 게임 일시정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// 이 UI 숨기기
	if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIManager->HideWidget(this);
	}

	// 웨이브 매니저에게 현재 웨이브(Shop) 종료 알림
	// EndWave()가 호출되면 결과창(이자 정산 등)이 뜨고 다음 단계로 진행할 수 있습니다.
	if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
	{
		WaveManager->EndWave();
	}
}
