// Fill out your copyright notice in the Description page of Project Settings.

#include "System/HMapProp.h"
#include "Components/StaticMeshComponent.h"

AHMapProp::AHMapProp()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	
	// 기본 설정: 풀링을 위해 모빌리티는 Movable로 설정
	MeshComponent->SetMobility(EComponentMobility::Movable);
}

void AHMapProp::InitializeProp(UStaticMesh* InMesh, FName InCollisionProfile)
{
	if (InMesh)
	{
		MeshComponent->SetStaticMesh(InMesh);
	}
	
	MeshComponent->SetCollisionProfileName(InCollisionProfile);
	
	// 가시성 및 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(InCollisionProfile != TEXT("NoCollision"));
}
