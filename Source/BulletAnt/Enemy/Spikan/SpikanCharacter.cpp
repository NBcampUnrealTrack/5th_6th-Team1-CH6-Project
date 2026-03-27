// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spikan/SpikanCharacter.h"
#include "Components/SphereComponent.h"

ASpikanCharacter::ASpikanCharacter()
{
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Ignore);	// Building
}
