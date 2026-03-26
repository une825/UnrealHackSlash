#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "HGemDragDropOp.generated.h"

class UHGemBase;
class UUserWidget;

/**
 * 젬 드래그 앤 드롭 시 데이터를 전달하기 위한 Operation 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGemDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** @brief 드래그 중인 실제 젬 인스턴스 */
	UPROPERTY(BlueprintReadWrite, Category = "Gem|UI")
	TObjectPtr<UHGemBase> DraggedGem;

	/** @brief 드래그가 시작된 소스 위젯 */
	UPROPERTY(BlueprintReadWrite, Category = "Gem|UI")
	TObjectPtr<UUserWidget> SourceWidget;

	/** @brief 드래그가 시작된 장착 슬롯 인덱스 (인벤토리에서 시작 시 -1) */
	UPROPERTY(BlueprintReadWrite, Category = "Gem|UI")
	int32 SourceSlotIndex = -1;
};
