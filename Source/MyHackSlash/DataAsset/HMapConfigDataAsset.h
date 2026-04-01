// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HMapTile.h"
#include "HMapConfigDataAsset.generated.h"

/**
 * @brief 무한 맵 생성 설정을 보관하는 데이터 에셋입니다.
 */
UCLASS()
class MYHACKSLASH_API UHMapConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 스폰할 타일 클래스 */
	UPROPERTY(EditAnywhere, Category = "Tile")
	TSubclassOf<class AHMapTile> MapTileClass;

	/** @brief 한 타일의 가로세로 크기 */
	UPROPERTY(EditAnywhere, Category = "Tile")
	float MapTileSize = 2000.0f;

	/** @brief 플레이어 주변 몇 칸까지 타일을 유지할지 결정 */
	UPROPERTY(EditAnywhere, Category = "Tile")
	int32 MapViewDistance = 2;

	/** @brief 랜덤하게 배치될 프롭 풀 데이터 */
	UPROPERTY(EditAnywhere, Category = "Prop")
	TArray<FMapPropData> MapPropPool;

	/** @brief 타일당 프롭 최소 개수 */
	UPROPERTY(EditAnywhere, Category = "Prop")
	int32 MinPropCount = 3;

	/** @brief 타일당 프롭 최대 개수 */
	UPROPERTY(EditAnywhere, Category = "Prop")
	int32 MaxPropCount = 8;
};
