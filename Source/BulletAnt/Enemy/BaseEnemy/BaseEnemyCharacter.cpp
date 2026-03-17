// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Enemy/BaseEnemy/BaseEnemyController.h"
#include "Net/UnrealNetwork.h"
#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"
#include "Player/BAPlayerController.h"
#include "Components/SphereComponent.h"
#include "Player/BACharacter.h"
#include "Building/BaseBuilding.h"
#include "GAS/BAGameplayTags.h"
#include "Enemy/DataAsset/TribeDataAsset.h"
#include "Enemy/Spawn/TribeMaterialManagerSubsystem.h"
#include "Components/CapsuleComponent.h"


ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AIControllerClass = ABaseEnemyController::StaticClass();

	// AI 회전 설정
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (IsValid(CharacterMovementComponent))
	{
		CharacterMovementComponent->bOrientRotationToMovement = true;
		CharacterMovementComponent->bUseControllerDesiredRotation = true;
	}

	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel6);	// Enemy ObjectType

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel7);	// EnemyVision ObjectType
	DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);	// Building
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Overlap);	// Character
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel9, ECollisionResponse::ECR_Overlap);	// Core
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemyCharacter::OnDetectionSphereBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemyCharacter::OnDetectionSphereEndOverlap);

	TargetActor = nullptr;
	TargetActorPriority = ETargetPriorityType::Max;
}

USphereComponent* ABaseEnemyCharacter::GetDetectionSphere() const
{
	return DetectionSphere;
}

void ABaseEnemyCharacter::SetTargetPrioriy(ETargetPriorityType InTargetPriority)
{
	TargetActorPriority = InTargetPriority;
}

void ABaseEnemyCharacter::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (OtherActor == this)
	{
		return;
	}

	//if (!ensureMsgf(IsValid(TribeType), TEXT("ABaseEnemyCharacter OnDetectionSphereBeginOverlap : TribeType Miss")))
	//{
	//	return;
	//}
	if (!IsValid(TribeType))
	{
		return;
	}

	ETargetPriorityType Priority;
	if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel1)	// Building
	{
		Priority = TribeType->Building;
	}
	else if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel3)	// Player
	{
		Priority = TribeType->Player;
	}
	else if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel9)	// Core
	{
		Priority = TribeType->Core;
	}
	else
	{
		return;
	}

	FActorArrayWrapper& Value = NearbyActors.FindOrAdd(Priority);
	Value.Actors.Add(OtherActor);
}

void ABaseEnemyCharacter::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ensureMsgf(IsValid(TribeType), TEXT("ABaseEnemyCharacter OnDetectionSphereBeginOverlap : TribeType Miss")))
	{
		return;
	}

	ETargetPriorityType Priority;
	if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel1)	// Building
	{
		Priority = TribeType->Building;
	}
	else if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel3)	// Player
	{
		Priority = TribeType->Player;
	}
	else if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel9)	// Core
	{
		Priority = TribeType->Core;
	}
	else
	{
		return;
	}

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

void ABaseEnemyCharacter::SenseNearbyActors()
{
	if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("BaseEnemyCharacter SenseNearbyActors : DataAsset Missing")))
	{
		return;
	}
	if (NearbyActors.Num() == 0)
	{
		return;
	}

	float MinDistSquared = TNumericLimits<float>::Max();
	float SenseAngle = BaseEnemyDataAsset->SenseAngle;
	AActor* NewTarget = nullptr;
	ETargetPriorityType NewTargetPriority;
	for (uint8 i = 1; i < static_cast<uint8>(TargetActorPriority); i++)
	{
		ETargetPriorityType Key = static_cast<ETargetPriorityType>(i);
		if (FActorArrayWrapper* Value = NearbyActors.Find(Key))
		{
			for (AActor* NearbyActor : Value->Actors)
			{
				if (IsValid(NearbyActor) && IsInFieldOfView(NearbyActor, SenseAngle))
				{
					double DistSquared = FVector::DistSquared2D(GetActorLocation(), NearbyActor->GetActorLocation());
					if (MinDistSquared > DistSquared)
					{
						NewTarget = NearbyActor;
						NewTargetPriority = Key;
						MinDistSquared = DistSquared;
					}
				}
			}

			if (IsValid(NewTarget) && TargetActor != NewTarget)
			{
				//RemoveOnTargetDestroy(TargetActor);
				//BindOnTargetDestroy(NewTarget);

				TargetActor = NewTarget;
				TargetActorPriority = NewTargetPriority;
				FStateTreeEvent ToRotate(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Rotating")));
				StateTreeComponent->SendStateTreeEvent(ToRotate);
				return;
			}
		}
	}

	if (!IsValid(TargetActor))
	{
		InitTarget();
		TransitionToRotate();
	}
}

bool ABaseEnemyCharacter::IsInFieldOfView(AActor* Target, float FOVAngle)
{
	FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector Forward = GetActorForwardVector();

	float DotProduct = FVector::DotProduct(Forward, DirectionToTarget);

	return DotProduct >= FMath::Cos(FMath::DegreesToRadians(FOVAngle / 2));
}

void ABaseEnemyCharacter::InitTarget()
{
	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("ABaseEnemyCharacter InitTarget : World Error")))
	{
		return;
	}
	ABAGameState* BAGameState = World->GetGameState<ABAGameState>();
	if (!ensureMsgf(IsValid(BAGameState), TEXT("ABaseEnemyCharacter InitTarget : BAGameState Error")))
	{
		return;
	}

	TargetActor = BAGameState->GetTargetCore();
	TargetActorPriority = ETargetPriorityType::Max;		
}

//void ABaseEnemyCharacter::OnTargetBuildingDestroy()
//{
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	UWorld* World = GetWorld();
//	if (!ensureMsgf(IsValid(World), TEXT("ABaseEnemyCharacter OnTargetBuildingDestroy : World Error")))
//	{
//		return;
//	}
//	ABAGameState* BAGameState = World->GetGameState<ABAGameState>();
//	if (!ensureMsgf(IsValid(BAGameState), TEXT("ABaseEnemyCharacter OnTargetBuildingDestroy : BAGameState Error")))
//	{
//		return;
//	}
//	//TargetActor = BAGameState->GetTargetCore();
//	//TargetActorPriority = ETargetPriorityType::High;
//	//FStateTreeEvent ToRotateForIntrude(FGameplayTag::RequestGameplayTag(TEXT("State.Intrude.Rotate")));
//	//StateTreeComponent->SendStateTreeEvent(ToRotateForIntrude);
//}
//
//void ABaseEnemyCharacter::BindOnTargetDestroy(AActor* Target)
//{
//	if (!IsValid(Target))
//	{
//		return;
//	}
//
//	ABaseBuilding* Building = Cast<ABaseBuilding>(Target);
//	if (IsValid(Building))
//	{
//		Building->OnDestroyed.AddUObject(this, &ABaseEnemyCharacter::OnTargetBuildingDestroy);
//	}
//}
//
//void ABaseEnemyCharacter::RemoveOnTargetDestroy(AActor* Target)
//{
//	if (!IsValid(Target))
//	{
//		return;
//	}
//
//	ABaseBuilding* BaseBuilding = Cast<ABaseBuilding>(Target);
//	if (IsValid(BaseBuilding))
//	{
//		BaseBuilding->OnDestroyed.RemoveAll(this);
//	}
//}

void ABaseEnemyCharacter::TransitionToRotate()
{
	FStateTreeEvent ToRotate(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Rotating")));
	StateTreeComponent->SendStateTreeEvent(ToRotate);
}

UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UStateTreeComponent* ABaseEnemyCharacter::GetStateTreeComponent() const
{
	return StateTreeComponent;
}

void ABaseEnemyCharacter::OnDeadEventReceived(const FGameplayEventData* Payload)
{
	if (IsValid(BaseEnemyDataAsset))
	{
		StateTreeComponent->SendStateTreeEvent(BaseEnemyDataAsset->DeathStateTag);
	}
}

UTribeDataAsset* ABaseEnemyCharacter::GetTribeType() const
{
	return TribeType;
}

void ABaseEnemyCharacter::SetTribeType(UTribeDataAsset* InTribeType)
{
	ensureMsgf(IsValid(InTribeType), TEXT("BaseEnemyCharacter SetTribeType : DataAsset NULL"));
	TribeType = InTribeType;
}

void ABaseEnemyCharacter::ApplyTribe()
{
	Multicast_ApplyTribeMaterial();
	ApplyTribePriority();
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

		WalkSpeed = BaseEnemyDataAsset->MoveSpeed * TribeType->SpeedMul;
		OnRep_WalkSpeed();
	}
}

void ABaseEnemyCharacter::Multicast_ApplyTribeMaterial_Implementation()
{
	if (!ensureMsgf(IsValid(TribeType), TEXT("BaseEnemyCharacter Multicast_ApplyTribeMaterial : TribeDataAsset Missing")))
	{
		return;
	}
	if (!ensureMsgf(IsValid(GetMesh()), TEXT("BaseEnemyCharacter Multicast_ApplyTribeMaterial : Mesh Missing")))
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTribeMaterialManagerSubsystem* TribeMaterialManagerSubsystem = GameInstance->GetSubsystem<UTribeMaterialManagerSubsystem>();
		if (!ensureMsgf(IsValid(TribeMaterialManagerSubsystem), TEXT("BaseEnemyCharacter Multicast_ApplyTribeMaterial : TribeMaterialManagerSubsystem Error")))
		{
			return;
		}

		UMaterialInterface* BaseMat = GetMesh()->GetMaterial(0);
		if (!IsValid(BaseMat))
		{
			return;
		}

		UMaterialInstanceDynamic* SharedMID = TribeMaterialManagerSubsystem->GetTribeMaterial(BaseMat, TribeType->TribeColor);
		if (SharedMID)
		{
			GetMesh()->SetMaterial(0, SharedMID);
		}
	}
}

void ABaseEnemyCharacter::ApplyTribePriority()
{
	if (HasAuthority())
	{
		if (!ensureMsgf(IsValid(TribeType), TEXT("BaseEnemyCharacter ApplyTribeMaterial : TribeDataAsset Missing")))
		{
			return;
		}
		if (!ensureMsgf(IsValid(DetectionSphere), TEXT("BaseEnemyCharacter ApplyTribePriority : DetectionSphere Missing")))
		{
			return;
		}
		
		if (TribeType->Building == ETargetPriorityType::Ignore)
		{
			DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);	// Building
		}
		if (TribeType->Player == ETargetPriorityType::Ignore)
		{
			DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Ignore);	// Character
		}		
	}
}

UAnimMontage* ABaseEnemyCharacter::GetDieAnimMontage() const
{
	if (HasAuthority())
	{
		if (IsValid(BaseEnemyDataAsset))
		{
			return BaseEnemyDataAsset->DieAnimMontage;
		}
	}

	return nullptr;
}

float ABaseEnemyCharacter::GetWalkSpeed() const
{
	return WalkSpeed;
}

void ABaseEnemyCharacter::SetWalkSpeed(float InWalkSpeed)
{
	WalkSpeed = InWalkSpeed;
}

void ABaseEnemyCharacter::OnRep_WalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABaseEnemyCharacter::StartIntrudeAction()
{
	TargetActorPriority = ETargetPriorityType::High;

	if (!ensureMsgf(IsValid(AbilitySystemComponent), TEXT("BaseEnemyCharacter StartIntrudeAction : AbilitySystemComponent Missing")))
	{
		return;
	}
	if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("BaseEnemyCharacter StartIntrudeAction : DataAsset Missing")))
	{
		return;
	}
	if (!ensureMsgf(IsValid(BaseEnemyDataAsset->IntrudeEffect), TEXT("BaseEnemyCharacter StartIntrudeAction : IntrudeEffect Missing")))
	{
		return;
	}
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(BaseEnemyDataAsset->IntrudeEffect, 1.0f, EffectContext);

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Intrude")), BaseEnemyDataAsset->IntrudeTime);
		GEIntrudeHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		AbilitySystemComponent->OnGameplayEffectRemoved_InfoDelegate(GEIntrudeHandle)->AddUObject(this, &ABaseEnemyCharacter::FinishIntrudeAction);
	}
}

void ABaseEnemyCharacter::FinishIntrudeAction(const FGameplayEffectRemovalInfo& InGERemovalInfo)
{
	TargetActorPriority = ETargetPriorityType::Max;
}

//void ABaseEnemyCharacter::Die()
//{
//	if (HasAuthority())
//	{
//		if (AbilitySystemComponent)
//		{
//			if (DeathGEHandle.IsValid())
//			{
//				AbilitySystemComponent->RemoveActiveGameplayEffect(DeathGEHandle);
//				DeathGEHandle.Invalidate();
//			}
//
//			AbilitySystemComponent->CancelAllAbilities();
//		}
//
//		Destroy();
//
//		UWorld* World = GetWorld();
//		if (IsValid(World))
//		{
//			USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
//			if (IsValid(SpawnManagerSubsystem))
//			{
//				SpawnManagerSubsystem->OnEnemyDie();
//			}
//		}
//	}
//}

AActor* ABaseEnemyCharacter::GetTargetActor() const
{
	return TargetActor;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitGAS();

	if (HasAuthority())
	{		
		if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("BaseEnemyCharacter BeginPlay : DataAsset Missing")))
		{
			return;
		}
		AcceptanceRadius = BaseEnemyDataAsset->AcceptanceRadius;
		GetCharacterMovement()->RotationRate = FRotator(0.f, BaseEnemyDataAsset->RotationRate, 0.f);
		RotateThreshold = BaseEnemyDataAsset->RotateThreshold;
		DetectionSphere->SetSphereRadius(BaseEnemyDataAsset->SenseRadius);

		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			GetWorldTimerManager().SetTimer(SensingTimerHandle, this, &ABaseEnemyCharacter::SenseNearbyActors, 0.2f, true);
		}
	}

	if (IsValid(BaseEnemyDataAsset->SpawnEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			BaseEnemyDataAsset->SpawnEffect,
			GetActorLocation()
		);
	}
}

void ABaseEnemyCharacter::InitGAS()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		for (const TSubclassOf<UGameplayAbility>& AbilityClass : BaseEnemyDataAsset->DefaultAbilities)
		{
			if (AbilityClass)
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1);
				AbilitySystemComponent->GiveAbility(Spec);
			}
		}

		for (const TSubclassOf<UGameplayEffect>& EffectClass : BaseEnemyDataAsset->DefaultEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
				EffectContext.AddSourceObject(this);

				// GE의 Spec(인스턴스 같은 것)을 생성
				FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);

				if (SpecHandle.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}

		DeadEventHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(TAG_Event_Combat_Dead)
			.AddUObject(this, &ABaseEnemyCharacter::OnDeadEventReceived);
	}

}

void ABaseEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (IsValid(StateTreeComponent))
		{
			InitTarget();
			StateTreeComponent->StartLogic();
		}
	}
}

void ABaseEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseEnemyCharacter, bIsTurning);
	DOREPLIFETIME(ABaseEnemyCharacter, bIsTurningLeft);
	DOREPLIFETIME(ABaseEnemyCharacter, WalkSpeed);
	DOREPLIFETIME(ABaseEnemyCharacter, TribeType);
}

void ABaseEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GetWorld()))
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}

	if (IsValid(AbilitySystemComponent) && DeadEventHandle.IsValid())
	{
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(TAG_Event_Combat_Dead).Remove(DeadEventHandle);
	}

	Super::EndPlay(EndPlayReason);
}

UDataAsset* ABaseEnemyCharacter::GetDataAsset() const
{
	return BaseEnemyDataAsset->BaseEnemyAttackDataAssetArray[0].AttackDataAsset;
}

bool ABaseEnemyCharacter::ShouldCallAfterAttack()
{
	return false;
}

void ABaseEnemyCharacter::AfterAttack()
{
}

void ABaseEnemyCharacter::Multicast_SetNoCollision_Implementation()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UCapsuleComponent* Capsule =  GetCapsuleComponent())
	{
		Capsule->SetSimulatePhysics(false);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	if (USkeletalMeshComponent* EnemyMesh = GetMesh())
	{
		EnemyMesh->SetSimulatePhysics(false);
		EnemyMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	if (DetectionSphere)
	{
		DetectionSphere->SetSimulatePhysics(false);
		DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}
