#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataAsset/HGlobalTextDataAsset.h"
#include "HSettingsManager.generated.h"

/**
 * @brief 플레이어 설정 값을 저장하고 런타임 시스템에 적용하기 위한 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FHGameSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EHTextLanguage Language = EHTextLanguage::Korean;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BGMVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bMute = false;
};

/**
 * @brief 플레이어 설정 저장 파일입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FHGameSettings SavedSettings;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHOnSettingsChanged, FHGameSettings, NewSettings);

/**
 * @brief 언어와 사운드 설정을 관리하고 저장하는 GameInstanceSubsystem입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSettingsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void ApplySettings(const FHGameSettings& InSettings, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void ResetToDefault(bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void SaveSettings() const;

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void LoadSettings();

	UFUNCTION(BlueprintPure, Category = "H|Settings")
	const FHGameSettings& GetSettings() const { return CurrentSettings; }

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void SetLanguage(EHTextLanguage InLanguage, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void SetMasterVolume(float InVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void SetBGMVolume(float InVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void SetSFXVolume(float InVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "H|Settings")
	void SetMute(bool bInMute, bool bSaveImmediately = true);

	UPROPERTY(BlueprintAssignable, Category = "H|Settings")
	FHOnSettingsChanged OnSettingsChanged;

private:
	FHGameSettings GetDefaultSettings() const;
	FHGameSettings SanitizeSettings(const FHGameSettings& InSettings) const;
	void ApplyRuntimeSettings();
	float GetEffectiveBGMVolume() const;

private:
	UPROPERTY()
	FHGameSettings CurrentSettings;

	UPROPERTY()
	FString SaveSlotName = TEXT("PlayerSettings");

	UPROPERTY()
	int32 SaveUserIndex = 0;
};
