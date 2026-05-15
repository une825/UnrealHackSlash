#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/HGlobalTextDataAsset.h"
#include "System/HSettingsManager.h"
#include "HSettingsUI.generated.h"

class UButton;
class UCheckBox;
class UComboBoxString;
class USlider;
class UTextBlock;

/**
 * @brief 언어와 사운드 설정을 변경하는 설정창 위젯입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSettingsUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnLanguageSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnMasterVolumeChanged(float InValue);

	UFUNCTION()
	void OnBGMVolumeChanged(float InValue);

	UFUNCTION()
	void OnSFXVolumeChanged(float InValue);

	UFUNCTION()
	void OnMuteChanged(bool bIsChecked);

	UFUNCTION()
	void OnApplyButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void HandleSettingsChanged(FHGameSettings NewSettings);

	void RefreshFromSettings(const FHGameSettings& InSettings);
	void RefreshLocalizedText();
	void PopulateLanguageOptions();
	UHSettingsManager* GetSettingsManager() const;
	FText GetGlobalText(FName InTextKey) const;
	FString GetLanguageOptionText(EHTextLanguage InLanguage) const;
	bool TryGetLanguageFromOption(const FString& InOption, EHTextLanguage& OutLanguage) const;

private:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> LanguageLabelText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> SoundLabelText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> MasterVolumeLabelText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> BGMVolumeLabelText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> SFXVolumeLabelText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> MuteLabelText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ApplyButtonText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ResetButtonText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> CloseButtonText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UComboBoxString> LanguageComboBox;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<USlider> BGMVolumeSlider;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<USlider> SFXVolumeSlider;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCheckBox> MuteCheckBox;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UButton> ResetButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY()
	FHGameSettings PendingSettings;

	bool bWasGamePausedBeforeOpen = false;
	bool bRefreshingUI = false;
};
