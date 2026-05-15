#include "UI/HWaveResultUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "System/HUIManager.h"
#include "System/HWaveManager.h"
#include "System/HGlobalTextManager.h"

void UHWaveResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackgroundButton)
	{
		BackgroundButton->OnClicked.AddDynamic(this, &UHWaveResultUI::OnClickBackground);
	}
}

void UHWaveResultUI::SetResultData(int32 InWaveIndex, EHWaveType InWaveType, int32 InInterestGold, int32 InTotalGold)
{
	// Wave 번호 설정 (예: Wave 1)
	if (WaveText)
	{
		WaveText->SetText(FText::Format(GetGlobalText(TEXT("UI.Common.WaveFormat")), FText::AsNumber(InWaveIndex)));
	}

	// Wave 타입 설정
	if (WaveTypeText)
	{
		FName TypeTextKey = NAME_None;
		switch (InWaveType)
		{
		case EHWaveType::Battle: TypeTextKey = TEXT("UI.WaveResult.Type.Battle"); break;
		case EHWaveType::Boss:   TypeTextKey = TEXT("UI.WaveResult.Type.Boss");   break;
		case EHWaveType::Reward: TypeTextKey = TEXT("UI.WaveResult.Type.Reward"); break;
		case EHWaveType::Shop:   TypeTextKey = TEXT("UI.WaveResult.Type.Shop");   break;
		}
		WaveTypeText->SetText(GetGlobalText(TypeTextKey));
	}

	// 이자 골드 설정
	if (InterestGoldText)
	{
		InterestGoldText->SetText(FText::Format(GetGlobalText(TEXT("UI.WaveResult.InterestGoldFormat")), FText::AsNumber(InInterestGold)));
	}

	// 총 골드 설정
	if (TotalGoldText)
	{
		TotalGoldText->SetText(FText::AsNumber(InTotalGold));
	}

	// 가이드 텍스트 수정
	if (TouchToSkipText)
	{
		TouchToSkipText->SetText(GetGlobalText(TEXT("UI.WaveResult.StartNextWaveGuide")));
	}
}

void UHWaveResultUI::OnClickBackground()
{
	// 게임 일시정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// 이 UI 숨기기
	if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIManager->HideWidget(this);
	}

	// 다음 Wave 시작 로직
	if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
	{
		WaveManager->PrepareNextWave();
		WaveManager->StartWave();
	}
}

FText UHWaveResultUI::GetGlobalText(FName InTextKey) const
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
