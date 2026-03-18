// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HMonsterAIInerface.generated.h"

UINTERFACE(MinimalAPI)
class UHMonsterAIInerface : public UInterface
{
	GENERATED_BODY()
};

DECLARE_DELEGATE(FAIMonsterAttackFinished);

/**
 * 몬스터 AI 제어를 위한 최소한의 인터페이스입니다.
 * 데이터는 이제 유닛 프로필이나 스탯에서 직접 가져옵니다.
 */
class MYHACKSLASH_API IHMonsterAIInerface
{
	GENERATED_BODY()

public:
	virtual void SetAIAttackDelegate(const FAIMonsterAttackFinished& InOnAttackFinished) = 0;
	virtual void AttackByAI() = 0;
};
