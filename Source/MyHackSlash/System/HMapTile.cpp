// Fill out your copyright notice in the Description page of Project Settings.

#include "System/HMapTile.h"
#include "System/HMapProp.h"
#include "System/HObjectPoolManager.h"
#include "Components/StaticMeshComponent.h"

AHMapTile::AHMapTile()
{
	PrimaryActorTick.bCanEverTick = false;

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	SetRootComponent(FloorMesh);
	
	// 기본 충돌 설정
	FloorMesh->SetCollisionProfileName(TEXT("BlockAll"));
	FloorMesh->SetMobility(EComponentMobility::Movable);
}

void AHMapTile::PrepareFromPool(const FIntPoint& InGridPos, float InTileSize, const TArray<FMapPropData>& InPropPool, int32 InMinPropCount, int32 InMaxPropCount)
{
	GridPos = InGridPos;
	
	// 그리드 좌표를 기반으로 월드 위치 계산 (Z축은 0으로 고정)
	FVector NewLocation = FVector(static_cast<float>(GridPos.X) * InTileSize, static_cast<float>(GridPos.Y) * InTileSize, 0.0f);
	SetActorLocation(NewLocation);
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// 이전 프롭 정리 후 새로 생성
	ClearProps();
	GenerateProps(InTileSize, InPropPool, InMinPropCount, InMaxPropCount);
}

void AHMapTile::ClearProps()
{
	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		for (AActor* Prop : SpawnedProps)
		{
			if (Prop)
			{
				PoolManager->ReturnToPool(Prop);
			}
		}
	}
	SpawnedProps.Empty();
}

void AHMapTile::GenerateProps(float InTileSize, const TArray<FMapPropData>& InPropPool, int32 InMinPropCount, int32 InMaxPropCount)
{
	if (InPropPool.Num() == 0) return;

	UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>();
	if (!PoolManager) return;

	// 결정론적 랜덤 (Deterministic Random)을 위해 그리드 좌표를 시드로 사용
	int32 Seed = (GridPos.X * 73856093) ^ (GridPos.Y * 19349663);
	FRandomStream RandomStream(Seed);

	// 가중치 총합 계산
	float TotalWeight = 0.0f;
	for (const FMapPropData& PropData : InPropPool)
	{
		TotalWeight += PropData.SpawnWeight;
	}

	if (TotalWeight <= 0.0f) return;

	// 타일당 생성할 프롭 개수 결정 (에디터 설정값 사용)
	int32 PropCount = RandomStream.RandRange(InMinPropCount, InMaxPropCount);
	float SafeZoneSize = InTileSize * 0.45f; // 타일 경계에서 약간 안쪽까지만 생성

	for (int32 i = 0; i < PropCount; ++i)
	{
		// 랜덤 위치 계산
		float RandomX = RandomStream.FRandRange(-SafeZoneSize, SafeZoneSize);
		float RandomY = RandomStream.FRandRange(-SafeZoneSize, SafeZoneSize);
		FVector RelativeLocation(RandomX, RandomY, 0.0f);
		FVector WorldLocation = GetActorLocation() + RelativeLocation;

		// 가중치 기반 랜덤 선택
		float RandomWeightValue = RandomStream.FRandRange(0.0f, TotalWeight);
		float CurrentWeightSum = 0.0f;
		const FMapPropData* SelectedProp = nullptr;

		for (const FMapPropData& PropData : InPropPool)
		{
			CurrentWeightSum += PropData.SpawnWeight;
			if (RandomWeightValue <= CurrentWeightSum)
			{
				SelectedProp = &PropData;
				break;
			}
		}

		if (!SelectedProp) SelectedProp = &InPropPool[0]; // 폴백

		// 풀에서 프롭 획득
		if (AHMapProp* NewProp = Cast<AHMapProp>(PoolManager->SpawnFromPool(AHMapProp::StaticClass(), WorldLocation, FRotator(0, RandomStream.FRandRange(0, 360), 0))))
		{
			NewProp->InitializeProp(SelectedProp->Mesh, SelectedProp->CollisionProfile);
			SpawnedProps.Add(NewProp);
		}
	}
}
