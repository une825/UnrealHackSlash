#include "UI/MainHud/HMainHudUI.h"
#include "UI/MainHud/HEquipGemSlotUI.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "Unit/Player/HPlayerState.h"
#include "System/HWaveManager.h"
#include "System/HPinkFogManager.h"
#include "System/HGlobalTextManager.h"
#include "System/HUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Mode/HGameState.h"

void UHMainHudUI::NativeConstruct()
{
	Super::NativeConstruct();

	// [추가] 각 슬롯 위젯의 인덱스 명시적 초기화
	if (EquipGemSlot_0) EquipGemSlot_0->SetSlotIndex(0);
	if (EquipGemSlot_1) EquipGemSlot_1->SetSlotIndex(1);
	if (EquipGemSlot_2) EquipGemSlot_2->SetSlotIndex(2);
	if (EquipGemSlot_3) EquipGemSlot_3->SetSlotIndex(3);

	// [추가] 핑크 안개 매니저 바인딩
	if (UHPinkFogManager* PinkFogManager = GetWorld()->GetSubsystem<UHPinkFogManager>())
	{
		PinkFogManager->OnPinkFogStateChanged.AddDynamic(this, &UHMainHudUI::HandlePinkFogStateChanged);
		HandlePinkFogStateChanged(PinkFogManager->GetCurrentState());
	}

	// 초기 상태에서는 이미지를 숨깁니다 (애니메이션에서 제어하지만 안전을 위해)
	if (BloodEffectImage)
	{
		BloodEffectImage->SetRenderOpacity(0.0f);
	}

	if (SettingButton)
	{
		SettingButton->OnClicked.AddDynamic(this, &UHMainHudUI::OnSettingButtonClicked);
	}

	// 1. 플레이어 스테이트를 찾아서 골드 변경 델리게이트에 바인딩
	if (!TryBindGoldChanged())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(GoldBindRetryTimerHandle, this, &UHMainHudUI::RetryBindGoldChanged, 0.1f, true);
		}
	}

	// 2. 복제된 GameState 웨이브 상태 바인딩
	bool bBoundReplicatedWaveState = false;
	if (AHGameState* HGameState = GetWorld()->GetGameState<AHGameState>())
	{
		auto ApplyReplicatedWaveState = [this](const FHReplicatedWaveState& InWaveState)
		{
			if (InWaveState.WaveIndex <= 0)
			{
				return;
			}

			if (InWaveState.WaveState == EHWaveState::Completed)
			{
				OnWaveCompleted(InWaveState.WaveIndex);
				return;
			}

			UpdateWaveInfo(InWaveState.WaveIndex, InWaveState.WaveType, InWaveState.ClearType);
			UpdateWaveProgress(InWaveState.ProgressPercent, InWaveState.CurrentValue, InWaveState.TargetValue);
		};

		HGameState->OnReplicatedWaveStateChanged.AddWeakLambda(this, ApplyReplicatedWaveState);
		ApplyReplicatedWaveState(HGameState->GetReplicatedWaveState());
		bBoundReplicatedWaveState = true;
	}

	// GameState를 아직 얻지 못한 초기 타이밍에서는 기존 서버 로컬 델리게이트로 보완합니다.
	if (!bBoundReplicatedWaveState)
	{
		if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
		{
			WaveManager->OnWaveStarted.AddDynamic(this, &UHMainHudUI::UpdateWaveInfo);
			WaveManager->OnWaveProgressUpdated.AddDynamic(this, &UHMainHudUI::UpdateWaveProgress);
			WaveManager->OnWaveCompleted.AddDynamic(this, &UHMainHudUI::OnWaveCompleted);

			if (WaveManager->IsWaveInProgress())
			{
				UpdateWaveInfo(WaveManager->GetCurrentWaveDisplayIndex(), WaveManager->GetCurrentWaveType(), WaveManager->GetCurrentWaveClearType());

				float Cur, Tar;
				WaveManager->GetCurrentWaveProgress(Cur, Tar);
				float Percent = Tar > 0 ? Cur / Tar : 0.0f;
				UpdateWaveProgress(Percent, Cur, Tar);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HMainHudUI: Main HUD Widget Constructed and Wave Manager bound!"));
}

void UHMainHudUI::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GoldBindRetryTimerHandle);
	}

	if (AHPlayerState* PS = BoundGoldPlayerState.Get())
	{
		PS->OnGoldChanged.RemoveAll(this);
	}
	BoundGoldPlayerState.Reset();

	Super::NativeDestruct();
}

void UHMainHudUI::RefreshGold(int32 InNewGold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::Format(GetGlobalText(TEXT("UI.MainHud.GoldOwnedFormat")), FText::AsNumber(InNewGold)));
	}
}

bool UHMainHudUI::TryBindGoldChanged()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return false;
	}

	AHPlayerState* PS = PC->GetPlayerState<AHPlayerState>();
	if (!PS)
	{
		return false;
	}

	if (BoundGoldPlayerState.Get() != PS)
	{
		if (AHPlayerState* OldPS = BoundGoldPlayerState.Get())
		{
			OldPS->OnGoldChanged.RemoveAll(this);
		}

		BoundGoldPlayerState = PS;
		PS->OnGoldChanged.RemoveAll(this);
		PS->OnGoldChanged.AddUObject(this, &UHMainHudUI::RefreshGold);
	}

	RefreshGold(PS->GetCurrentGold());
	return true;
}

void UHMainHudUI::RetryBindGoldChanged()
{
	if (TryBindGoldChanged())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(GoldBindRetryTimerHandle);
		}
	}
}

void UHMainHudUI::UpdateWaveInfo(int32 InWaveIndex, EHWaveType InWaveType, EHWaveClearType InClearType)
{
	if (WaveText)
	{
		WaveText->SetText(FText::Format(GetGlobalText(TEXT("UI.Common.WaveFormat")), FText::AsNumber(InWaveIndex)));
	}

	// 클리어 조건에 따라 UI 요소 가시성 조절
	if (RemainTimeText)
	{
		RemainTimeText->SetVisibility(InClearType == EHWaveClearType::TimeSurvival ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		RemainTimeText->SetText(FText::GetEmpty());
	}

	if (KillProgressText)
	{
		KillProgressText->SetVisibility(InClearType == EHWaveClearType::KillCount ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		KillProgressText->SetText(FText::GetEmpty());
	}

	// 프로그레스 바 초기화 (Reward/Shop 타입이면 숨김, Battle/Boss면 표시)
	if (WaveProgressBar)
	{
		bool bShowProgress = (InClearType != EHWaveClearType::ManualUI);
		WaveProgressBar->SetVisibility(bShowProgress ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		WaveProgressBar->SetPercent(0.0f);
	}
}

void UHMainHudUI::UpdateWaveProgress(float InProgressPercent, float InCurrentValue, float InTargetValue)
{
	// 0. 프로그레스 바 업데이트
	if (WaveProgressBar && WaveProgressBar->GetVisibility() != ESlateVisibility::Collapsed)
	{
		WaveProgressBar->SetPercent(InProgressPercent);
	}

	// 1. 남은 시간 표시 (TimeSurvival 타입인 경우)
	if (RemainTimeText && RemainTimeText->GetVisibility() != ESlateVisibility::Collapsed)
	{
		int32 RemainSeconds = FMath::Max(0, FMath::FloorToInt(InTargetValue - InCurrentValue));
		RemainTimeText->SetText(FText::Format(GetGlobalText(TEXT("UI.MainHud.RemainTimeSecondsFormat")), FText::AsNumber(RemainSeconds)));
	}

	// 2. 처치 진행률 표시 (KillCount 타입인 경우)
	if (KillProgressText && KillProgressText->GetVisibility() != ESlateVisibility::Collapsed)
	{
		KillProgressText->SetText(FText::Format(GetGlobalText(TEXT("UI.Common.ValuePairFormat")), FText::AsNumber(FMath::FloorToInt(InCurrentValue)), FText::AsNumber(FMath::FloorToInt(InTargetValue))));
	}
}

void UHMainHudUI::OnWaveCompleted(int32 InWaveIndex)
{
	if (WaveText)
	{
		WaveText->SetText(FText::Format(GetGlobalText(TEXT("UI.MainHud.WaveCompletedFormat")), FText::AsNumber(InWaveIndex)));
	}
}

void UHMainHudUI::PlayDamageEffectAnim()
{
	if (PlayDamageEffect)
	{
		PlayAnimation(PlayDamageEffect);
	}
}

void UHMainHudUI::ShowNotifyText(const FText& InText, const FLinearColor& InColor, bool bPlayAnim)
{
	if (NotifyText)
	{
		NotifyText->SetText(InText);
		NotifyText->SetColorAndOpacity(InColor);
		NotifyText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (NotifyTextOverlay)
	{
		NotifyTextOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (bPlayAnim && NotifyAnim)
	{
		PlayAnimation(NotifyAnim);
	}
}

void UHMainHudUI::HideNotifyText()
{
	if (NotifyTextOverlay)
	{
		NotifyTextOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (NotifyText)
	{
		NotifyText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHMainHudUI::HandlePinkFogStateChanged(EHPinkFogState NewState)
{
	switch (NewState)
	{
	case EHPinkFogState::FadingIn:
		ShowNotifyText(GetGlobalText(TEXT("UI.MainHud.PinkFog.FadingIn")), FLinearColor::White, true);
		break;

	case EHPinkFogState::Active:
		ShowNotifyText(GetGlobalText(TEXT("UI.MainHud.PinkFog.Active")), FLinearColor::Red, false);
		break;

	case EHPinkFogState::FadingOut:
		ShowNotifyText(GetGlobalText(TEXT("UI.MainHud.PinkFog.FadingOut")), FLinearColor::White, false);
		break;

	case EHPinkFogState::Inactive:
		HideNotifyText();
		break;
	}
}

void UHMainHudUI::OnSettingButtonClicked()
{
	if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIManager->ShowWidgetByName(TEXT("SettingsUI"), 100);
	}
}

FText UHMainHudUI::GetGlobalText(FName InTextKey) const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHGlobalTextManager* TextManager = GameInstance->GetSubsystem<UHGlobalTextManager>())
		{
			return TextManager->GetText(InTextKey);
		}
	}

	return FText::FromName(InTextKey);
}
