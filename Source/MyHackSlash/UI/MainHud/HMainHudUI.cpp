#include "UI/MainHud/HMainHudUI.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Unit/Player/HPlayerState.h"
#include "System/HWaveManager.h"
#include "Kismet/GameplayStatics.h"

void UHMainHudUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 상태에서는 이미지를 숨깁니다 (애니메이션에서 제어하지만 안전을 위해)
	if (BloodEffectImage)
	{
		BloodEffectImage->SetRenderOpacity(0.0f);
	}

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

	// 2. 웨이브 매니저 델리게이트 바인딩
	if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
	{
		WaveManager->OnWaveStarted.AddDynamic(this, &UHMainHudUI::UpdateWaveInfo);
		WaveManager->OnWaveProgressUpdated.AddDynamic(this, &UHMainHudUI::UpdateWaveProgress);
		WaveManager->OnWaveCompleted.AddDynamic(this, &UHMainHudUI::OnWaveCompleted);

		// [추가] 이미 웨이브가 진행 중인 경우를 위해 현재 상태로 초기화
		if (WaveManager->IsWaveInProgress())
		{
			UpdateWaveInfo(WaveManager->GetCurrentWaveDisplayIndex(), WaveManager->GetCurrentWaveType(), WaveManager->GetCurrentWaveClearType());
			
			float Cur, Tar;
			WaveManager->GetCurrentWaveProgress(Cur, Tar);
			float Percent = Tar > 0 ? Cur / Tar : 0.0f;
			UpdateWaveProgress(Percent, Cur, Tar);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HMainHudUI: Main HUD Widget Constructed and Wave Manager bound!"));
}

void UHMainHudUI::RefreshGold(int32 InNewGold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::Format(FText::FromString(TEXT("보유 골드 : {0}")), FText::AsNumber(InNewGold)));
	}
}

void UHMainHudUI::UpdateWaveInfo(int32 InWaveIndex, EHWaveType InWaveType, EHWaveClearType InClearType)
{
	if (WaveText)
	{
		WaveText->SetText(FText::Format(FText::FromString(TEXT("Wave {0}")), FText::AsNumber(InWaveIndex)));
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
		RemainTimeText->SetText(FText::Format(FText::FromString(TEXT("{0}s")), FText::AsNumber(RemainSeconds)));
	}

	// 2. 처치 진행률 표시 (KillCount 타입인 경우)
	if (KillProgressText && KillProgressText->GetVisibility() != ESlateVisibility::Collapsed)
	{
		KillProgressText->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(FMath::FloorToInt(InCurrentValue)), FText::AsNumber(FMath::FloorToInt(InTargetValue))));
	}
}

void UHMainHudUI::OnWaveCompleted(int32 InWaveIndex)
{
	if (WaveText)
	{
		WaveText->SetText(FText::Format(FText::FromString(TEXT("Wave {0} Completed!")), FText::AsNumber(InWaveIndex)));
	}
}

void UHMainHudUI::PlayDamageEffectAnim()
{
	if (PlayDamageEffect)
	{
		PlayAnimation(PlayDamageEffect);
	}
}
