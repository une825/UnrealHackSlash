// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MYHACKSLASH_API IHCombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 노티파이에서 호출할 함수
	virtual void AttackHitCheck() = 0;
	// ANS의 NotifyBegin에서 호출

public:
	void ClearAlreadyHitActors() { AlreadyHitActors.Empty(); }
	void AddHitActor(AActor* InActor) { AlreadyHitActors.Add(InActor); }
	bool WasAlreadyHit(AActor* InActor) const { return AlreadyHitActors.Contains(InActor); }

protected:
	// 약참조(WeakPtr)를 사용하여 파괴된 액터에 대한 메모리 문제를 방지합니다.
	TArray<TWeakObjectPtr<AActor>> AlreadyHitActors;

protected:
	// 히트 박스 크기나 소켓 이름을 에디터에서 수정 가능하게 노출
	FName WeaponSocketName = TEXT("hand_r");
	
	float HitRadius = 50.0f;
};
