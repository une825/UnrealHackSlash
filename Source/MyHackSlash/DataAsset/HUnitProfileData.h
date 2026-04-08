// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HUnitProfileData.generated.h"

// 유닛의 종류 정의
UENUM(BlueprintType)
enum class EHUnitType : uint8
{
	Player,
	Monster
};

/**
 * 유닛(플레이어/몬스터)의 전체적인 설정과 데이터를 관리하는 프로필 에셋입니다.
 */
UCLASS()
class MYHACKSLASH_API UHUnitProfileData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 이 프로필을 사용하는 유닛의 타입 (플레이어 또는 몬스터)
	UPROPERTY(EditAnywhere, Category = "Unit")
	EHUnitType UnitType = EHUnitType::Monster;

	// --- 애니메이션 설정 ---
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TMap<struct FGameplayTag, TObjectPtr<class UAnimMontage>> ActionMontageMap;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeadMontage;

	// --- 헬퍼 함수 ---
	/** @brief 특정 태그에 해당하는 몽타주를 반환합니다. 없으면 기본 AttackMontage를 반환합니다. */
	UAnimMontage* GetActionMontage(const struct FGameplayTag& InActionTag) const;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float TurnSpeed = 10.0f;

	// 사망 시 밀려나는 힘
	UPROPERTY(EditAnywhere, Category = "Effects")
	float DeathImpulseForce = 8000.0f;

	/** @brief 타격에 성공했을 때 재생할 사운드 목록 (랜덤 재생됨) */
	UPROPERTY(EditAnywhere, Category = "Effects")
	TArray<TObjectPtr<class USoundBase>> HitSounds;
};
