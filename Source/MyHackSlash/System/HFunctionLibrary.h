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
};
