// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Fly/BaseFlyEnemy.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "Enemy/DataAsset/FlyDataAsset.h"
#include "Enemy/DataAsset/TribeDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Enemy/BaseEnemy/BaseEnemyController.h"
#include "Components/CapsuleComponent.h"

void ABaseFlyEnemy::ApplyTribe()
{
	Super::ApplyTribe();

	if (HasAuthority())
	{
		if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("BaseEnemyCharacter ApplyTribe : DataAsset Missing")))
		{
			return;
		}
		if (!ensureMsgf(IsValid(TribeType), TEXT("BaseEnemyCharacter ApplyTribe : TribeType Missing")))
		{
			return;
		}

		FlySpeed = BaseEnemyDataAsset->MoveSpeed * TribeType->SpeedMul;
		OnRep_FlySpeed();

		UFlyDataAsset* FlyDataAsset = Cast<UFlyDataAsset>(BaseEnemyDataAsset);
		if (!ensureMsgf(IsValid(FlyDataAsset), TEXT("ABaseFlyEnemy BeginPlay : FlyDataAsset Missing")))
		{
			return;
		}
		Deceleration = FlyDataAsset->BrakingDecelerationFlying * TribeType->SpeedMul;
		OnRep_Deceleration();
	}

	return;
}

void ABaseFlyEnemy::SetDiveMode()
{
	if (HasAuthority())
	{
		Multicast_SetDiveMode();
	}
}

void ABaseFlyEnemy::UnSetDiveMode()
{
	if (HasAuthority())
	{
		Multicast_UnSetDiveMode();
	}
}

void ABaseFlyEnemy::SetFlySpeed(float InSpeed)
{
	FlySpeed = InSpeed;
	OnRep_FlySpeed();
}

ABaseFlyEnemy::ABaseFlyEnemy()
{
    // 컨트롤러 회전 사용 안 함 (이동 방향에 맞추기 위해)
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    UCharacterMovementComponent* CMC = GetCharacterMovement();
    if (IsValid(CMC))
    {
		CMC->SetMovementMode(EMovementMode::MOVE_Flying);
		CMC->DefaultLandMovementMode = MOVE_Flying;
		CMC->bOrientRotationToMovement = false;
		CMC->bUseControllerDesiredRotation = true;
		CMC->BrakingFrictionFactor = 0.5f;
    }

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (IsValid(Capsule))
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECR_Ignore);	// Character
		Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel9, ECR_Ignore);	// Core
		Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel6, ECR_Ignore);	// Enemy
	}
}

void ABaseFlyEnemy::BeginPlay()
{
    Super::BeginPlay();

	if (HasAuthority())
	{
		if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("ABaseFlyEnemy BeginPlay : DataAsset Missing")))
		{
			return;
		}
		UFlyDataAsset* FlyDataAsset = Cast<UFlyDataAsset>(BaseEnemyDataAsset);
		if (!ensureMsgf(IsValid(FlyDataAsset), TEXT("ABaseFlyEnemy BeginPlay : FlyDataAsset Missing")))
		{
			return;
		}

		// Remove
		ApplyTribe();
	}
}

void ABaseFlyEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseFlyEnemy, FlySpeed);
	DOREPLIFETIME(ABaseFlyEnemy, Deceleration);
}

void ABaseFlyEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		ABaseEnemyController* BEController = Cast<ABaseEnemyController>(NewController);
		if (IsValid(TargetActor) && IsValid(BEController))
		{
			BEController->SetFocus(TargetActor);
		}
	}
}

void ABaseFlyEnemy::OnRep_FlySpeed()
{
	if (IsValid(GetCharacterMovement()))
	{
		GetCharacterMovement()->MaxFlySpeed = FlySpeed;
	}
}

void ABaseFlyEnemy::OnRep_Deceleration()
{
	if (IsValid(GetCharacterMovement()))
	{
		GetCharacterMovement()->BrakingDecelerationFlying = Deceleration;
	}
}

void ABaseFlyEnemy::Multicast_UnSetDiveMode_Implementation()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (IsValid(Capsule))
	{
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Block);	// Building
	}
}

void ABaseFlyEnemy::Multicast_SetDiveMode_Implementation()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (IsValid(Capsule))
	{
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Ignore);	// Building
	}
}
