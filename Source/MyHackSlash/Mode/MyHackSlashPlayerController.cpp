// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mode/MyHackSlashPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "System/HUIManager.h"
#include "Engine/LocalPlayer.h"
#include <Unit/HBaseCharacter.h>
#include <Kismet/KismetMathLibrary.h>

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMyHackSlashPlayerController::AMyHackSlashPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void AMyHackSlashPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// 인게임 입력 모드 설정 (인트로의 UIOnly 설정을 해제하기 위함)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// 마우스 커서 표시 보장
	bShowMouseCursor = true;

	// test
	if (UHUIManager* UIMgr = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIMgr->ShowWidgetByName(TEXT("MainHUD"));
	}
}

void AMyHackSlashPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Started, this, &AMyHackSlashPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Triggered, this, &AMyHackSlashPlayerController::OnDestinationTriggered);
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Completed, this, &AMyHackSlashPlayerController::OnDestinationReleased);
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Canceled, this, &AMyHackSlashPlayerController::OnDestinationReleased);

		// Setup WASD Move events
		EnhancedInputComponent->BindAction(WASDMoveAction, ETriggerEvent::Triggered, this, &AMyHackSlashPlayerController::OnWASDMove);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMyHackSlashPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AMyHackSlashPlayerController::OnDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AMyHackSlashPlayerController::OnDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

void AMyHackSlashPlayerController::OnWASDMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn != nullptr)
	{
		// 1. �̵� ����
		const FVector ForwardDirection(1.0f, 0.0f, 0.0f);
		const FVector RightDirection(0.0f, 1.0f, 0.0f);

		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);

		// 2. ���콺 ���� �ٶ󺸱� ���� �߰�
		FHitResult HitResult;
		// ECC_Visibility ä���� ����Ͽ� ���콺 �Ʒ��� ���� ��ǥ�� �����ɴϴ�.
		if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector MouseLocation = HitResult.Location;
			FVector PawnLocation = ControlledPawn->GetActorLocation();

			// ���콺 ��ġ�� ĳ���� ��ġ ������ ȸ���� ���
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PawnLocation, MouseLocation);

			// ĳ���Ͱ� ���Ʒ��� ����� �ʵ��� Pitch�� Roll�� 0���� �����ϰ� Yaw(�¿� ȸ��)�� ����մϴ�.
			ControlledPawn->SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
		}
	}
}

void AMyHackSlashPlayerController::OnAttackTriggered()
{
	AHBaseCharacter* ControlledCharacter = Cast<AHBaseCharacter>(GetPawn());
	if (ControlledCharacter != nullptr)
	{
		//ControlledCharacter->Attack();
	}
}
