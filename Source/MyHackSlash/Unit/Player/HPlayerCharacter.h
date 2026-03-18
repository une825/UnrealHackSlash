// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/HBaseCharacter.h"
#include "HPlayerCharacter.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API AHPlayerCharacter : public AHBaseCharacter
{
	GENERATED_BODY()

public:
	AHPlayerCharacter();

public:
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

public:
	virtual void PossessedBy(AController* NewController);
	virtual void SetupPlayerInputComponent(class UInputComponent* InPlayerInputComponent) override;

protected:
	void SetupGASInputComponent();
	void GASInputPressed(const int32 InInputID);
	void GASInputReleased(const int32 InInputID);

protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TMap<int32, TSubclassOf<class UGameplayAbility>> StartInputAbilities;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};
