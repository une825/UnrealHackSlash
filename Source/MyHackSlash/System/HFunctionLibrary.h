#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HFunctionLibrary.generated.h"

/**
 * @brief 프로젝트 전반에서 사용하는 공용 정적 함수들을 모아둔 라이브러리 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief GemID를 사용하여 GameplayTag용 문자열을 생성합니다. (예: FireBall -> Data.GemID.FireBall)
	 * @param InGemID 젬의 기본 식별자
	 * @return GameplayTag 경로 문자열
	 */
	UFUNCTION(BlueprintPure, Category = "HackSlash|Utility")
	static FString MakeGemTagString(FName InGemID);

	/**
	 * @brief GemID와 Tier를 사용하여 젬의 아이콘을 가져옵니다.
	 * @param WorldContextObject 월드 컨텍스트
	 * @param InGemID 젬의 기본 식별자 (예: FireBall)
	 * @param InTier 젬의 티어 (1, 2, 3...)
	 * @return 젬의 아이콘 텍스처 (없으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "HackSlash|Gem", meta = (WorldContext = "WorldContextObject"))
	static class UTexture2D* GetGemIcon(const UObject* WorldContextObject, FName InGemID, int32 InTier);
};
