#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataAsset/HGlobalTextDataAsset.h"
#include "HGlobalTextManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHOnGlobalTextLanguageChanged, EHTextLanguage, NewLanguage);

/**
 * @brief 언어별 텍스트 테이블을 참조해 UI 표시 텍스트를 제공하는 글로벌 매니저입니다.
 */
UCLASS(Config = Game, DefaultConfig)
class MYHACKSLASH_API UHGlobalTextManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Global Text")
	void SetTextDataAsset(UHGlobalTextDataAsset* InTextDataAsset);

	UFUNCTION(BlueprintCallable, Category = "Global Text")
	bool SetActiveLanguage(EHTextLanguage InLanguage);

	UFUNCTION(BlueprintPure, Category = "Global Text")
	EHTextLanguage GetActiveLanguage() const { return ActiveLanguage; }

	UFUNCTION(BlueprintPure, Category = "Global Text")
	FText GetText(FName InTextKey) const;

	UFUNCTION(BlueprintCallable, Category = "Global Text")
	bool FindText(FName InTextKey, FText& OutText) const;

	UFUNCTION(BlueprintCallable, Category = "Global Text")
	void RefreshTextCache();

	UPROPERTY(BlueprintAssignable, Category = "Global Text")
	FHOnGlobalTextLanguageChanged OnLanguageChanged;

private:
	void BuildTextCache(EHTextLanguage InLanguage, TMap<FName, FText>& OutTextCache) const;

private:
	UPROPERTY(Config, EditAnywhere, Category = "Global Text")
	TSoftObjectPtr<UHGlobalTextDataAsset> TextDataAssetPath;

	UPROPERTY()
	TObjectPtr<UHGlobalTextDataAsset> TextDataAsset;

	UPROPERTY()
	EHTextLanguage ActiveLanguage = EHTextLanguage::Korean;

	TMap<FName, FText> ActiveTextCache;
	TMap<FName, FText> FallbackTextCache;
};
