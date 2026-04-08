#include "UI/HWaveResultUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "System/HUIManager.h"
#include "System/HWaveManager.h"

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
		WaveText->SetText(FText::Format(FText::FromString(TEXT("Wave {0}")), FText::AsNumber(InWaveIndex)));
	}

	// Wave 타입 설정
	if (WaveTypeText)
	{
		FString TypeStr;
		switch (InWaveType)
		{
		case EHWaveType::Battle: TypeStr = TEXT("BATTLE"); break;
		case EHWaveType::Boss:   TypeStr = TEXT("BOSS");   break;
		case EHWaveType::Reward: TypeStr = TEXT("REWARD"); break;
		case EHWaveType::Shop:   TypeStr = TEXT("SHOP");   break;
		}
		WaveTypeText->SetText(FText::FromString(TypeStr));
	}

	// 이자 골드 설정
	if (InterestGoldText)
	{
		InterestGoldText->SetText(FText::Format(FText::FromString(TEXT("+ {0}")), FText::AsNumber(InInterestGold)));
	}

	// 총 골드 설정
	if (TotalGoldText)
	{
		TotalGoldText->SetText(FText::AsNumber(InTotalGold));
	}

	// 가이드 텍스트 수정
	if (TouchToSkipText)
	{
		TouchToSkipText->SetText(FText::FromString(TEXT("TOUCH TO START NEXT WAVE")));
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
