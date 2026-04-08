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

	/** @brief 직접 스폰할 액터 클래스 (비어있으면 기본 AHMapProp 사용) */
	UPROPERTY(EditAnywhere, Category = "Prop")
	TSubclassOf<AActor> PropClass;

	/** @brief PropClass가 비어있을 때 사용할 메쉬 */
	UPROPERTY(EditAnywhere, Category = "Prop", meta = (EditCondition = "PropClass == nullptr"))
	TObjectPtr<class UStaticMesh> Mesh;

	/** @brief PropClass가 비어있을 때 사용할 콜리전 프로필 */
	UPROPERTY(EditAnywhere, Category = "Prop", meta = (EditCondition = "PropClass == nullptr"))
	FName CollisionProfile = TEXT("BlockAll");

	/** @brief 스폰될 확률 가중치 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, Category = "Prop")
	float SpawnWeight = 0.5f;

	/** @brief 스폰 시 추가로 적용할 Z축 오프셋 */
	UPROPERTY(EditAnywhere, Category = "Prop")
	float SpawnZOffset = 0.0f;

	/** @brief 액터의 크기를 계산하여 자동으로 바닥에 붙일지 여부 (중심점이 박스 중앙인 경우 유용) */
	UPROPERTY(EditAnywhere, Category = "Prop")
	bool bAutoGround = true;
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
