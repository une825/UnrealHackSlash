// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "System/HMapTile.h"
#include "HInfiniteMapManager.generated.h"

class AHMapTile;

/**
 * @brief 플레이어 주변에 무한히 타일을 생성/제거하는 매니저 서브시스템입니다.
 */
UCLASS()
class MYHACKSLASH_API UHInfiniteMapManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** @brief 매 프레임 또는 주기적으로 호출되어 플레이어 주변의 맵을 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Map")
	void UpdateMap(const FVector& InPlayerLocation);

	/** @brief 맵 생성을 위한 기본 설정을 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Map")
	void SetMapSettings(TSubclassOf<AHMapTile> InTileClass, float InTileSize, int32 InViewDistance, const TArray<FMapPropData>& InPropPool, int32 InMinPropCount, int32 InMaxPropCount);

private:
	/** @brief 현재 활성화된 타일 맵 (그리드 좌표 -> 타일 액터 포인터) */
	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<AHMapTile>> ActiveTiles;

	/** @brief 스폰할 타일 클래스 */
	UPROPERTY()
	TSubclassOf<AHMapTile> TileClass;

	/** @brief 랜덤하게 배치될 프롭 풀 데이터 */
	UPROPERTY()
	TArray<FMapPropData> PropPool;

	int32 MinPropCount = 3;
	int32 MaxPropCount = 8;

	/** @brief 한 타일의 가로세로 크기 */
	float TileSize = 2000.0f;

	/** @brief 플레이어 주변 몇 칸까지 타일을 유지할지 결정 (ViewDistance=1 이면 3x3) */
	int32 ViewDistance = 2;

	/** @brief 마지막으로 업데이트했을 때의 플레이어 그리드 위치 */
	FIntPoint LastPlayerGridPos = FIntPoint(MAX_int32, MAX_int32);

	/** @brief 특정 좌표에 타일을 스폰(또는 풀에서 획득)합니다. */
	void SpawnTile(const FIntPoint& InGridPos);

	/** @brief 플레이어와 너무 멀어진 타일들을 제거(풀로 반납)합니다. */
	void RemoveFarTiles(const FIntPoint& InCurrentGridPos);
};
