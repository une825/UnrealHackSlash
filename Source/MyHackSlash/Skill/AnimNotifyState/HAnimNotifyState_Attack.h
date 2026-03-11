// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HAnimNotifyState_Attack.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Melee       UMETA(DisplayName = "Melee (Sweep)"),      // 근접 스윕
	Projectile  UMETA(DisplayName = "Projectile (Spawn)"), // 투사체 생성
};

UCLASS()
class MYHACKSLASH_API UHAnimNotifyState_Attack : public UAnimNotifyState
{
	GENERATED_BODY()

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	// 투사체나 히트스캔 시 한 번만 실행할지, 틱마다 실행할지 결정
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bExecuteOnce = true;

	//// 공격 데이터 (데미지, 투사체 클래스, 범위 등 정보가 담긴 DataAsset)
	//UPROPERTY(EditAnywhere, Category = "Attack")
	//TObjectPtr<UAttackDataAsset> AttackData;

private:
	bool bHasExecuted = false; // 실행 여부 체크용
};
