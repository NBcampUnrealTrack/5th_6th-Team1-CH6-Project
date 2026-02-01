// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "BaseEnemyDataAsset.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"

void ABaseEnemyCharacter::OnDeath()
{
}

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
	return BaseEnemyAttackDataAsset;
}
