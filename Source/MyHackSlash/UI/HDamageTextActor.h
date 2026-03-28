#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HDamageTextActor.generated.h"

class UWidgetComponent;

UCLASS()
class MYHACKSLASH_API AHDamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AHDamageTextActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 데미지 수치 및 크리티컬 여부를 설정하고 재생을 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "DamageText")
	void InitializeDamageText(float InDamage, bool bInIsCritical);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> DamageWidgetComponent;

	// 데미지 텍스트가 위로 올라가는 속도
	UPROPERTY(EditAnywhere, Category = "DamageText")
	float FloatingSpeed = 100.0f;

	// 데미지 텍스트가 유지되는 시간
	UPROPERTY(EditAnywhere, Category = "DamageText")
	float LifeTime = 1.0f;

	float ElapsedTime = 0.0f;
};
