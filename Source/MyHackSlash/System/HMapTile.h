// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HMapTile.generated.h"

/**
 * @brief 프롭 종류와 설정을 정의하는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FMapPropData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere)
	FName CollisionProfile = TEXT("BlockAll");

	/** @brief 스폰될 확률 가중치 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere)
	float SpawnWeight = 0.5f;
};

/**
 * @brief 무한 맵 구성을 위한 개별 타일 액터입니다.
 */
UCLASS()
class MYHACKSLASH_API AHMapTile : public AActor
{
	GENERATED_BODY()
	
public:	
	AHMapTile();

	/** @brief 풀에서 꺼내질 때 호출되어 좌표와 실제 월드 위치를 초기화합니다. */
	void PrepareFromPool(const FIntPoint& InGridPos, float InTileSize, const TArray<FMapPropData>& InPropPool, int32 InMinPropCount, int32 InMaxPropCount);

	/** @brief 타일이 풀로 반납될 때 호출되어 프롭들을 정리합니다. */
	void ClearProps();

	/** @brief 현재 타일의 그리드 좌표를 반환합니다. */
	FIntPoint GetGridPos() const { return GridPos; }

protected:
	/** @brief 바닥 메쉬 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Map")
	TObjectPtr<class UStaticMeshComponent> FloorMesh;

	/** @brief 현재 타일에 생성된 프롭 액터들 */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedProps;

	/** @brief 현재 타일의 그리드 좌표 (X, Y) */
	UPROPERTY(VisibleInstanceOnly, Category = "H|Map")
	FIntPoint GridPos;

	/** @brief 랜덤하게 프롭들을 생성합니다. */
	void GenerateProps(float InTileSize, const TArray<FMapPropData>& InPropPool, int32 InMinPropCount, int32 InMaxPropCount);
};
