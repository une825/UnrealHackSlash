#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HSoundDataAsset.generated.h"

class USoundBase;

/**
 * @brief 게임 내에서 사용되는 BGM 및 효과음(SFX)을 통합 관리하는 데이터 에셋입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSoundDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief GameplayTag 기반의 사운드 리스트 (권장 방식) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Sound")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> SoundMap;

	/** @brief 배경음악(BGM) 리스트 - 키값으로 찾을 수 있도록 함 (레거시) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Sound")
	TMap<FName, TObjectPtr<USoundBase>> BGMMap;

	/** @brief 효과음(SFX) 리스트 (레거시) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Sound")
	TMap<FName, TObjectPtr<USoundBase>> SFXMap;

	/** @brief 지정된 태그에 해당하는 사운드를 찾습니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	USoundBase* GetSound(FGameplayTag InTag) const;

	/** @brief 지정된 키에 해당하는 BGM을 찾습니다. (레거시) */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	USoundBase* GetBGM(FName InKey) const;

	/** @brief 지정된 키에 해당하는 효과음을 찾습니다. (레거시) */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	USoundBase* GetSFX(FName InKey) const;
};
