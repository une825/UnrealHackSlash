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
	Melee       UMETA(DisplayName = "Melee (Sweep)"),      // ���� ����
	Projectile  UMETA(DisplayName = "Projectile (Spawn)"), // ����ü ����
};

UCLASS()
class MYHACKSLASH_API UHAnimNotifyState_Attack : public UAnimNotifyState
{
	GENERATED_BODY()

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// ����ü�� ��Ʈ��ĵ �� �� ���� ��������, ƽ���� �������� ����
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bExecuteOnce = true;

	//// ���� ������ (������, ����ü Ŭ����, ���� �� ������ ��� DataAsset)
	//UPROPERTY(EditAnywhere, Category = "Attack")
	//TObjectPtr<UAttackDataAsset> AttackData;

private:
	bool bHasExecuted = false; // ���� ���� üũ��
};
