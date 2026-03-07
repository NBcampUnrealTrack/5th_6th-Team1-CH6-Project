// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAWeaponCamera.h"
#include "Camera/CameraComponent.h"

// Sets default values
ABAWeaponCamera::ABAWeaponCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

}

// Called when the game starts or when spawned
void ABAWeaponCamera::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABAWeaponCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

