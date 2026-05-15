#include "UI/Settings/HSettingsUI.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "System/HGlobalTextManager.h"
#include "System/HUIManager.h"
#include "Kismet/GameplayStatics.h"

void UHSettingsUI::NativeConstruct()
{
	Super::NativeConstruct();

	bWasGamePausedBeforeOpen = UGameplayStatics::IsGamePaused(GetWorld());
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	PopulateLanguageOptions();

	if (LanguageComboBox)
	{
		LanguageComboBox->OnSelectionChanged.AddDynamic(this, &UHSettingsUI::OnLanguageSelectionChanged);
	}
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UHSettingsUI::OnMasterVolumeChanged);
	}
	if (BGMVolumeSlider)
	{
		BGMVolumeSlider->OnValueChanged.AddDynamic(this, &UHSettingsUI::OnBGMVolumeChanged);
	}
	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UHSettingsUI::OnSFXVolumeChanged);
	}
	if (MuteCheckBox)
	{
		MuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UHSettingsUI::OnMuteChanged);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &UHSettingsUI::OnApplyButtonClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UHSettingsUI::OnResetButtonClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UHSettingsUI::OnCloseButtonClicked);
	}

	if (UHSettingsManager* SettingsManager = GetSettingsManager())
	{
		SettingsManager->OnSettingsChanged.AddDynamic(this, &UHSettingsUI::HandleSettingsChanged);
		RefreshFromSettings(SettingsManager->GetSettings());
	}

	RefreshLocalizedText();
}

void UHSettingsUI::NativeDestruct()
{
	UGameplayStatics::SetGamePaused(GetWorld(), bWasGamePausedBeforeOpen);

	if (UHSettingsManager* SettingsManager = GetSettingsManager())
	{
		SettingsManager->OnSettingsChanged.RemoveDynamic(this, &UHSettingsUI::HandleSettingsChanged);
	}

	Super::NativeDestruct();
}

void UHSettingsUI::OnLanguageSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bRefreshingUI)
	{
		return;
	}

	EHTextLanguage SelectedLanguage = PendingSettings.Language;
	if (TryGetLanguageFromOption(SelectedItem, SelectedLanguage))
	{
		PendingSettings.Language = SelectedLanguage;
	}
}

void UHSettingsUI::OnMasterVolumeChanged(float InValue)
{
	if (!bRefreshingUI)
	{
		PendingSettings.MasterVolume = InValue;
	}
}

void UHSettingsUI::OnBGMVolumeChanged(float InValue)
{
	if (!bRefreshingUI)
	{
		PendingSettings.BGMVolume = InValue;
	}
}

void UHSettingsUI::OnSFXVolumeChanged(float InValue)
{
	if (!bRefreshingUI)
	{
		PendingSettings.SFXVolume = InValue;
	}
}

void UHSettingsUI::OnMuteChanged(bool bIsChecked)
{
	if (!bRefreshingUI)
	{
		PendingSettings.bMute = bIsChecked;
	}
}

void UHSettingsUI::OnApplyButtonClicked()
{
	if (UHSettingsManager* SettingsManager = GetSettingsManager())
	{
		SettingsManager->ApplySettings(PendingSettings, true);
	}
}

void UHSettingsUI::OnResetButtonClicked()
{
	if (UHSettingsManager* SettingsManager = GetSettingsManager())
	{
		SettingsManager->ResetToDefault(true);
	}
}

void UHSettingsUI::OnCloseButtonClicked()
{
	if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIManager->HideWidget(this);
		return;
	}

	RemoveFromParent();
}

void UHSettingsUI::HandleSettingsChanged(FHGameSettings NewSettings)
{
	RefreshFromSettings(NewSettings);
	RefreshLocalizedText();
}

void UHSettingsUI::RefreshFromSettings(const FHGameSettings& InSettings)
{
	PendingSettings = InSettings;
	bRefreshingUI = true;

	if (LanguageComboBox)
	{
		LanguageComboBox->SetSelectedOption(GetLanguageOptionText(PendingSettings.Language));
	}
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(PendingSettings.MasterVolume);
	}
	if (BGMVolumeSlider)
	{
		BGMVolumeSlider->SetValue(PendingSettings.BGMVolume);
	}
	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->SetValue(PendingSettings.SFXVolume);
	}
	if (MuteCheckBox)
	{
		MuteCheckBox->SetIsChecked(PendingSettings.bMute);
	}

	bRefreshingUI = false;
}

void UHSettingsUI::RefreshLocalizedText()
{
	if (TitleText)
	{
		TitleText->SetText(GetGlobalText(TEXT("UI.Settings.Title")));
	}
	if (LanguageLabelText)
	{
		LanguageLabelText->SetText(GetGlobalText(TEXT("UI.Settings.Language")));
	}
	if (SoundLabelText)
	{
		SoundLabelText->SetText(GetGlobalText(TEXT("UI.Settings.Sound")));
	}
	if (MasterVolumeLabelText)
	{
		MasterVolumeLabelText->SetText(GetGlobalText(TEXT("UI.Settings.MasterVolume")));
	}
	if (BGMVolumeLabelText)
	{
		BGMVolumeLabelText->SetText(GetGlobalText(TEXT("UI.Settings.BGMVolume")));
	}
	if (SFXVolumeLabelText)
	{
		SFXVolumeLabelText->SetText(GetGlobalText(TEXT("UI.Settings.SFXVolume")));
	}
	if (MuteLabelText)
	{
		MuteLabelText->SetText(GetGlobalText(TEXT("UI.Settings.Mute")));
	}
	if (ApplyButtonText)
	{
		ApplyButtonText->SetText(GetGlobalText(TEXT("UI.Settings.Apply")));
	}
	if (ResetButtonText)
	{
		ResetButtonText->SetText(GetGlobalText(TEXT("UI.Settings.Reset")));
	}
	if (CloseButtonText)
	{
		CloseButtonText->SetText(GetGlobalText(TEXT("UI.Settings.Close")));
	}
}

void UHSettingsUI::PopulateLanguageOptions()
{
	if (!LanguageComboBox)
	{
		return;
	}

	LanguageComboBox->ClearOptions();
	LanguageComboBox->AddOption(GetLanguageOptionText(EHTextLanguage::Korean));
	LanguageComboBox->AddOption(GetLanguageOptionText(EHTextLanguage::English));
}

UHSettingsManager* UHSettingsUI::GetSettingsManager() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UHSettingsManager>();
	}

	return nullptr;
}

FText UHSettingsUI::GetGlobalText(FName InTextKey) const
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

FString UHSettingsUI::GetLanguageOptionText(EHTextLanguage InLanguage) const
{
	switch (InLanguage)
	{
	case EHTextLanguage::English:
		return TEXT("English");

	case EHTextLanguage::Korean:
	default:
		return TEXT("Korean");
	}
}

bool UHSettingsUI::TryGetLanguageFromOption(const FString& InOption, EHTextLanguage& OutLanguage) const
{
	if (InOption == TEXT("English"))
	{
		OutLanguage = EHTextLanguage::English;
		return true;
	}
	if (InOption == TEXT("Korean"))
	{
		OutLanguage = EHTextLanguage::Korean;
		return true;
	}

	return false;
}
