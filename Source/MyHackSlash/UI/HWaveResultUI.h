#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/HGlobalTypes.h"
#include "HWaveResultUI.generated.h"

class UTextBlock;
class UButton;

/**
 * @brief Wave 종료 후 결과를 표시하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHWaveResultUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 
	 * @brief 웨이브 결과 데이터를 UI에 설정합니다.
	 * @param InWaveIndex 현재 웨이브 번호
	 * @param InWaveType 웨이브 타입 (Battle, Boss 등)
	 * @param InInterestGold 이번 웨이브에서 획득한 이자 골드
	 * @param InTotalGold 현재 보유 중인 총 골드
	 */
	UFUNCTION(BlueprintCallable, Category = "H|UI")
	void SetResultData(int32 InWaveIndex, EHWaveType InWaveType, int32 InInterestGold, int32 InTotalGold);

protected:
	virtual void NativeConstruct() override;

	/** @brief 배경 또는 화면 클릭 시 호출될 함수 (일시정지 해제 및 다음 단계 진행) */
	UFUNCTION()
	void OnClickBackground();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveTypeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InterestGoldText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalGoldText;

	/** @brief 하단 가이드 텍스트 (클릭 유도) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TouchToSkipText;

	/** @brief 전체 화면 클릭을 감지하기 위한 투명 버튼 (Optional) */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UButton> BackgroundButton;
};
