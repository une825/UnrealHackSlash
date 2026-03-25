#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "System/HGlobalTypes.h"
#include "DataAsset/HGemDataAsset.h"
#include "HGemBase.generated.h"

/**
 * 게임 내에서 생성되는 젬 인스턴스의 최상위 베이스 클래스입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class MYHACKSLASH_API UHGemBase : public UObject
{
	GENERATED_BODY()

public:
	UHGemBase();

	/** 데이터를 기반으로 젬을 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem")
	virtual void Initialize(const FHGemData& InGemData);

	/** 젬의 카테고리를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem")
	HEGemCategory GetGemCategory() const { return GemData.GemCategory; }

	/** 젬의 타입을 반환합니다. (Melee, Projectile 등) */
	UFUNCTION(BlueprintPure, Category = "Gem")
	HEGemType GetGemType() const { return GemData.GemType; }

	/** 젬의 속성을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem")
	HEElement GetGemElement() const { return GemData.GemElement; }

	/** 젬의 상세 데이터를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem")
	const FHGemData& GetGemData() const { return GemData; }

protected:
	/** 젬의 상세 데이터 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem")
	FHGemData GemData;
};
