// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Fly/BaseFlyEnemy.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "Enemy/DataAsset/FlyDataAsset.h"
#include "Enemy/DataAsset/TribeDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Enemy/BaseEnemy/BaseEnemyController.h"
#include "Components/CapsuleComponent.h"
#include "GAS/AttributeSet/MoveAttributeSet.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GameplayEffect.h"

void ABaseFlyEnemy::OnRep_TribeType()
{
	ApplyTribeMaterial();

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
		HealthAttributeSet->SetMaxHealth(BaseEnemyDataAsset->Health * TribeType->HealthMul);
		HealthAttributeSet->SetHealth(BaseEnemyDataAsset->Health * TribeType->HealthMul);

		MoveAttributeSet->SetMoveSpeed(BaseEnemyDataAsset->MoveSpeed * TribeType->SpeedMul);

		UFlyDataAsset* FlyDataAsset = Cast<UFlyDataAsset>(BaseEnemyDataAsset);
		if (!ensureMsgf(IsValid(FlyDataAsset), TEXT("ABaseFlyEnemy BeginPlay : FlyDataAsset Missing")))
		{
			return;
		}
		Deceleration = FlyDataAsset->BrakingDecelerationFlying * TribeType->SpeedMul;
		OnRep_Deceleration();
		AccelerationRate = FlyDataAsset->AccelerationRate * TribeType->SpeedMul;
		OnRep_AccelerationRate();
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

float ABaseFlyEnemy::GetFlySpeed() const
{
	if (IsValid(MoveAttributeSet))
	{
		return MoveAttributeSet->GetMoveSpeed();
	}
	else
	{
		return 700.f;
	}
}

void ABaseFlyEnemy::SetFlySpeed(float InSpeed)
{
	if (IsValid(MoveAttributeSet))
	{
		MoveAttributeSet->SetMoveSpeed(InSpeed);
	}
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
		CMC->RotationRate = FRotator(100, 100, 100);
    }

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (IsValid(Capsule))
	{
		GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel12);	// FlyEnemy ObjectType
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

	if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("ABaseFlyEnemy BeginPlay : DataAsset Missing")))
	{
		return;
	}
	UFlyDataAsset* FlyDataAsset = Cast<UFlyDataAsset>(BaseEnemyDataAsset);
	if (!ensureMsgf(IsValid(FlyDataAsset), TEXT("ABaseFlyEnemy BeginPlay : FlyDataAsset Missing")))
	{
		return;
	}
	GetCharacterMovement()->RotationRate = FlyDataAsset->FlyRotationRate;

	if (IsValid(AbilitySystemComponent) && IsValid(MoveAttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MoveAttributeSet->GetMoveSpeedMultiplierAttribute())
			.AddUObject(this, &ABaseFlyEnemy::OnMoveAttributeChange);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MoveAttributeSet->GetMoveSpeedAttribute())
			.AddUObject(this, &ABaseFlyEnemy::OnMoveAttributeChange);
	}
}

void ABaseFlyEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABaseFlyEnemy, Deceleration);
	DOREPLIFETIME(ABaseFlyEnemy, AccelerationRate);
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

void ABaseFlyEnemy::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (OtherActor == this)
	{
		return;
	}
	if (!IsValid(TribeType))
	{
		return;
	}

	ETargetPriorityType Priority = ETargetPriorityType::High;
	FActorArrayWrapper& Value = NearbyActors.FindOrAdd(Priority);
	Value.Actors.Add(OtherActor);
}

void ABaseFlyEnemy::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!ensureMsgf(IsValid(TribeType), TEXT("ABaseEnemyCharacter OnDetectionSphereBeginOverlap : TribeType Miss")))
	{
		return;
	}

	ETargetPriorityType Priority = ETargetPriorityType::High;

	if (FActorArrayWrapper* Value = NearbyActors.Find(Priority))
	{
		Value->Actors.Remove(OtherActor);
		if (TargetActor == OtherActor)
		{
			InitTarget();
			StartIntrudeAction();
			TransitionToRotate();
		}
	}
}

void ABaseFlyEnemy::OnMoveAttributeChange(const FOnAttributeChangeData& Data)
{
	if (IsValid(GetCharacterMovement()) && IsValid(MoveAttributeSet))
	{
		GetCharacterMovement()->MaxFlySpeed = MoveAttributeSet->GetMoveSpeed() * MoveAttributeSet->GetMoveSpeedMultiplier();
	}
}

void ABaseFlyEnemy::OnRep_Deceleration()
{
	if (IsValid(GetCharacterMovement()))
	{
		GetCharacterMovement()->BrakingDecelerationFlying = Deceleration;
	}
}

void ABaseFlyEnemy::OnRep_AccelerationRate()
{
	if (IsValid(GetCharacterMovement()))
	{
		GetCharacterMovement()->MaxAcceleration = AccelerationRate;
	}
}

void ABaseFlyEnemy::Multicast_UnSetDiveMode_Implementation()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (IsValid(Capsule))
	{
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);	
	}
}

void ABaseFlyEnemy::Multicast_SetDiveMode_Implementation()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (IsValid(Capsule))
	{
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);	
	}
}
