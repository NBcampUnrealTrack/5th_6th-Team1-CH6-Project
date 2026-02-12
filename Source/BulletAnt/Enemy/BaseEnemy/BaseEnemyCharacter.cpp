// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "BaseEnemyDataAsset.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Enemy/BaseEnemy/BaseEnemyController.h"

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
		CharacterMovementComponent->bOrientRotationToMovement = false;
		CharacterMovementComponent->bUseControllerDesiredRotation = true;
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UStateTreeComponent* ABaseEnemyCharacter::GetStateTreeComponent() const
{
	return StateTreeComponent;
}

AActor* ABaseEnemyCharacter::GetTargetActor() const
{
	return TargetActor;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{		
		if (!ensureMsgf(IsValid(BaseEnemyDataAsset), TEXT("BaseEnemyCharacter BeginPlay : DataAsset Missing")))
		{
			return;
		}
		AcceptanceRadius = BaseEnemyDataAsset->AcceptanceRadius;
		GetCharacterMovement()->RotationRate = FRotator(0.f, BaseEnemyDataAsset->RotationRate, 0.f);

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
		}
		
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
			if (IsValid(SpawnManagerSubsystem))
			{
				TargetActor = SpawnManagerSubsystem->GetTargetActor();			
			}
			else
			{
				TargetActor = nullptr;
			}
		}
		
		StateTreeComponent->StartLogic();
	}
}

UDataAsset* ABaseEnemyCharacter::GetDataAsset() const
{
	return BaseEnemyDataAsset->BaseEnemyAttackDataAsset;
}

void ABaseEnemyCharacter::OnDeath()
{
	if (HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->CancelAllAbilities();
		}
	}

	Destroy();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManagerSubsystem))
		{
			SpawnManagerSubsystem->OnEnemyDie();
		}
	}
}