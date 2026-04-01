// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HMapProp.generated.h"

/**
 * @brief 맵 타일 위에 랜덤하게 배치될 프롭(돌, 풀 등) 액터입니다.
 */
UCLASS()
class MYHACKSLASH_API AHMapProp : public AActor
{
	GENERATED_BODY()
	
public:	
	AHMapProp();

	/** @brief 메쉬와 콜리전 설정을 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Map")
	void InitializeProp(class UStaticMesh* InMesh, FName InCollisionProfile);

protected:
	/** @brief 프롭의 외형을 담당하는 메쉬 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Map")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;
};
