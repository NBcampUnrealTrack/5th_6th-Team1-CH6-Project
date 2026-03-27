// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Larvae/LarvaeCharacter.h"
#include "Components/SphereComponent.h"

ALarvaeCharacter::ALarvaeCharacter()
{
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Ignore);	// Building
}
