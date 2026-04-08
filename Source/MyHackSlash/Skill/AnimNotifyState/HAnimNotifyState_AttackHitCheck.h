// Source/MyHackSlash/Skill/AnimNotifyState/HAnimNotifyState_AttackHitCheck.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "HAnimNotifyState_AttackHitCheck.generated.h"

/**
 * 특정 시점에 게임플레이 이벤트를 전달하는 Notify 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHAnimNotifyState_AttackHitCheck : public UAnimNotify
{
	GENERATED_BODY()

public:
	UHAnimNotifyState_AttackHitCheck();

	virtual FString GetNotifyName_Implementation() const override;

	// 특정 시점에 한 번 호출됩니다.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// 전달할 게임플레이 태그 (에디터에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FGameplayTag TriggerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName PlaySoundName;
};
