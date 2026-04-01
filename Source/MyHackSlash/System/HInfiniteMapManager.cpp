// Fill out your copyright notice in the Description page of Project Settings.

#include "System/HInfiniteMapManager.h"
#include "System/HMapTile.h"
#include "System/HObjectPoolManager.h"

void UHInfiniteMapManager::SetMapSettings(TSubclassOf<AHMapTile> InTileClass, float InTileSize, int32 InViewDistance, const TArray<FMapPropData>& InPropPool, int32 InMinPropCount, int32 InMaxPropCount)
{
	TileClass = InTileClass;
	TileSize = InTileSize;
	ViewDistance = InViewDistance;
	PropPool = InPropPool;
	MinPropCount = InMinPropCount;
	MaxPropCount = InMaxPropCount;
	
	// 설정이 변경되면 다음 업데이트 때 전체 갱신을 유도하기 위해 초기화
	LastPlayerGridPos = FIntPoint(MAX_int32, MAX_int32);
}

void UHInfiniteMapManager::UpdateMap(const FVector& InPlayerLocation)
{
	if (!TileClass) return;

	// 현재 플레이어의 그리드 좌표 계산
	int32 CurrentGridX = FMath::RoundToInt(InPlayerLocation.X / TileSize);
	int32 CurrentGridY = FMath::RoundToInt(InPlayerLocation.Y / TileSize);
	FIntPoint CurrentGridPos(CurrentGridX, CurrentGridY);

	// 플레이어가 다른 타일 영역으로 이동했거나 첫 실행인 경우에만 갱신
	if (CurrentGridPos != LastPlayerGridPos || ActiveTiles.Num() == 0)
	{
		LastPlayerGridPos = CurrentGridPos;

		// 1. 필요한 범위 내의 타일들을 스폰
		for (int32 x = -ViewDistance; x <= ViewDistance; ++x)
		{
			for (int32 y = -ViewDistance; y <= ViewDistance; ++y)
			{
				SpawnTile(CurrentGridPos + FIntPoint(x, y));
			}
		}

		// 2. 범위를 벗어난 타일들을 제거
		RemoveFarTiles(CurrentGridPos);
	}
}

void UHInfiniteMapManager::SpawnTile(const FIntPoint& InGridPos)
{
	// 이미 활성화된 좌표면 스킵
	if (ActiveTiles.Contains(InGridPos)) return;

	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		// 풀에서 타일 획득 (위치는 PrepareFromPool에서 다시 설정됨)
		if (AHMapTile* NewTile = Cast<AHMapTile>(PoolManager->SpawnFromPool(TileClass, FVector::ZeroVector, FRotator::ZeroRotator)))
		{
			NewTile->PrepareFromPool(InGridPos, TileSize, PropPool, MinPropCount, MaxPropCount);
			ActiveTiles.Add(InGridPos, NewTile);
		}
	}
}

void UHInfiniteMapManager::RemoveFarTiles(const FIntPoint& InCurrentGridPos)
{
	TArray<FIntPoint> ToRemoveKeys;
	for (auto& Pair : ActiveTiles)
	{
		FIntPoint TilePos = Pair.Key;
		// 상하좌우 ViewDistance를 벗어났는지 체크
		if (FMath::Abs(TilePos.X - InCurrentGridPos.X) > ViewDistance || 
			FMath::Abs(TilePos.Y - InCurrentGridPos.Y) > ViewDistance)
		{
			ToRemoveKeys.Add(TilePos);
		}
	}

	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		for (const FIntPoint& Key : ToRemoveKeys)
		{
			AHMapTile* Tile = ActiveTiles[Key];
			if (Tile)
			{
				PoolManager->ReturnToPool(Tile);
			}
			ActiveTiles.Remove(Key);
		}
	}
}
