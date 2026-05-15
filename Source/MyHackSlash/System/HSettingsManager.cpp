#include "System/HSettingsManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "System/HGlobalTextManager.h"
#include "System/HSoundManager.h"

void UHSettingsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSettings();
	ApplyRuntimeSettings();
}

void UHSettingsManager::ApplySettings(const FHGameSettings& InSettings, bool bSaveImmediately)
{
	CurrentSettings = SanitizeSettings(InSettings);
	ApplyRuntimeSettings();

	if (bSaveImmediately)
	{
		SaveSettings();
	}

	OnSettingsChanged.Broadcast(CurrentSettings);
}

void UHSettingsManager::ResetToDefault(bool bSaveImmediately)
{
	ApplySettings(GetDefaultSettings(), bSaveImmediately);
}

void UHSettingsManager::SaveSettings() const
{
	UHSettingsSaveGame* SaveGame = Cast<UHSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UHSettingsSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return;
	}

	SaveGame->SavedSettings = CurrentSettings;
	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, SaveUserIndex);
}

void UHSettingsManager::LoadSettings()
{
	CurrentSettings = GetDefaultSettings();

	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		return;
	}

	UHSettingsSaveGame* SaveGame = Cast<UHSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	if (!SaveGame)
	{
		return;
	}

	CurrentSettings = SanitizeSettings(SaveGame->SavedSettings);
}

void UHSettingsManager::SetLanguage(EHTextLanguage InLanguage, bool bSaveImmediately)
{
	FHGameSettings NewSettings = CurrentSettings;
	NewSettings.Language = InLanguage;
	ApplySettings(NewSettings, bSaveImmediately);
}

void UHSettingsManager::SetMasterVolume(float InVolume, bool bSaveImmediately)
{
	FHGameSettings NewSettings = CurrentSettings;
	NewSettings.MasterVolume = InVolume;
	ApplySettings(NewSettings, bSaveImmediately);
}

void UHSettingsManager::SetBGMVolume(float InVolume, bool bSaveImmediately)
{
	FHGameSettings NewSettings = CurrentSettings;
	NewSettings.BGMVolume = InVolume;
	ApplySettings(NewSettings, bSaveImmediately);
}

void UHSettingsManager::SetSFXVolume(float InVolume, bool bSaveImmediately)
{
	FHGameSettings NewSettings = CurrentSettings;
	NewSettings.SFXVolume = InVolume;
	ApplySettings(NewSettings, bSaveImmediately);
}

void UHSettingsManager::SetMute(bool bInMute, bool bSaveImmediately)
{
	FHGameSettings NewSettings = CurrentSettings;
	NewSettings.bMute = bInMute;
	ApplySettings(NewSettings, bSaveImmediately);
}

FHGameSettings UHSettingsManager::GetDefaultSettings() const
{
	FHGameSettings DefaultSettings;
	DefaultSettings.Language = EHTextLanguage::Korean;
	DefaultSettings.MasterVolume = 1.0f;
	DefaultSettings.BGMVolume = 1.0f;
	DefaultSettings.SFXVolume = 1.0f;
	DefaultSettings.bMute = false;
	return DefaultSettings;
}

FHGameSettings UHSettingsManager::SanitizeSettings(const FHGameSettings& InSettings) const
{
	FHGameSettings SanitizedSettings = InSettings;
	SanitizedSettings.MasterVolume = FMath::Clamp(SanitizedSettings.MasterVolume, 0.0f, 1.0f);
	SanitizedSettings.BGMVolume = FMath::Clamp(SanitizedSettings.BGMVolume, 0.0f, 1.0f);
	SanitizedSettings.SFXVolume = FMath::Clamp(SanitizedSettings.SFXVolume, 0.0f, 1.0f);
	return SanitizedSettings;
}

void UHSettingsManager::ApplyRuntimeSettings()
{
	if (UHGlobalTextManager* TextManager = GetGameInstance()->GetSubsystem<UHGlobalTextManager>())
	{
		TextManager->SetActiveLanguage(CurrentSettings.Language);
	}

	if (UWorld* World = GetWorld())
	{
		if (UHSoundManager* SoundManager = World->GetSubsystem<UHSoundManager>())
		{
			SoundManager->SetBGMVolume(GetEffectiveBGMVolume());
		}
	}
}

float UHSettingsManager::GetEffectiveBGMVolume() const
{
	return CurrentSettings.bMute ? 0.0f : CurrentSettings.MasterVolume * CurrentSettings.BGMVolume;
}
