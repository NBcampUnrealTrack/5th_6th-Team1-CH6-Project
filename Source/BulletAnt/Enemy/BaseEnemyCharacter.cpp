// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "BaseEnemyDataAsset.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// DataAsset으로 초기화
	if (IsValid(BaseEnemyDataAsset))
	{
		AcceptanceRadius = BaseEnemyDataAsset->AcceptanceRadius;
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

AActor* ABaseEnemyCharacter::GetTargetActor() const
{
	return TargetActor;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	checkf(IsValid(BaseEnemyDataAsset), TEXT("BaseEnemyCharacter BeginPlay : DataAsset Missing"));
	
	if (HasAuthority())
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
			
			// 태그 부여
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
