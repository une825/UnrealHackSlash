#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HGlobalTextDataAsset.generated.h"

/**
 * @brief 글로벌 텍스트 시스템에서 지원하는 언어를 정의합니다.
 */
UENUM(BlueprintType)
enum class EHTextLanguage : uint8
{
	Korean UMETA(DisplayName = "Korean"),
	English UMETA(DisplayName = "English"),
	Japanese UMETA(DisplayName = "Japanese"),
	ChineseSimplified UMETA(DisplayName = "Chinese Simplified"),
	ChineseTraditional UMETA(DisplayName = "Chinese Traditional"),
	French UMETA(DisplayName = "French"),
	German UMETA(DisplayName = "German"),
	Spanish UMETA(DisplayName = "Spanish")
};

/**
 * @brief 언어별 텍스트 DataTable의 한 행을 정의합니다.
 */
USTRUCT(BlueprintType)
struct FHGlobalTextRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** @brief 현재 언어에서 표시할 텍스트입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Text", meta = (MultiLine = true))
	FText Text;
};

/**
 * @brief 언어와 DataTable 매핑 정보입니다.
 */
USTRUCT(BlueprintType)
struct FHLanguageTextTable
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Text")
	EHTextLanguage Language = EHTextLanguage::Korean;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Text")
	TSoftObjectPtr<UDataTable> TextTable;
};

/**
 * @brief 글로벌 텍스트 시스템의 정적 설정을 보관하는 데이터 에셋입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHGlobalTextDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Text")
	EHTextLanguage DefaultLanguage = EHTextLanguage::Korean;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Text")
	EHTextLanguage FallbackLanguage = EHTextLanguage::English;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Text")
	TArray<FHLanguageTextTable> LanguageTextTables;

	UFUNCTION(BlueprintCallable, Category = "Global Text")
	UDataTable* GetTextTable(EHTextLanguage InLanguage) const;
};
