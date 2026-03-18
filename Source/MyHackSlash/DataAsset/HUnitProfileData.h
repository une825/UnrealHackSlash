// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset/HMonsterStatRow.h"
#include "HUnitProfileData.generated.h"

/**
 * 유닛(플레이어/몬스터)의 전체적인 설정과 데이터를 관리하는 프로필 에셋입니다.
 */
UCLASS()
class MYHACKSLASH_API UHUnitProfileData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- 스탯 설정 ---
	// 이 유닛이 사용할 레벨별 스탯 테이블
	UPROPERTY(EditAnywhere, Category = "Stats")
	TObjectPtr<class UDataTable> StatTable;

	// --- 애니메이션 설정 ---
	// 기본 공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

	// 사망 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeadMontage;

	// --- AI 설정 ---
	// AI 순찰 반경
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolRadius = 800.0f;

	// AI 탐지 반경
	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectRadius = 600.0f;

	// AI 회전 속도
	UPROPERTY(EditAnywhere, Category = "AI")
	float TurnSpeed = 10.0f;

	// 사망 시 밀려나는 힘
	UPROPERTY(EditAnywhere, Category = "Effects")
	float DeathImpulseForce = 500.0f;

	// --- 헬퍼 함수 ---
	// 특정 레벨의 스탯 정보를 반환 (DataTable의 Row Name을 레벨 숫자로 가정)
	FMonsterStatRow* GetStatRowByLevel(int32 Level) const
	{
		if (!StatTable) return nullptr;

		FName RowName = FName(*FString::FromInt(Level));
		return StatTable->FindRow<FMonsterStatRow>(RowName, TEXT("Unit Profile Stat Context"));
	}
};
