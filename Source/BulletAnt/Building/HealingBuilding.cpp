#include "Building/HealingBuilding.h"

#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "Player/BACharacter.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GAS/BAGameplayTags.h"
#include "TimerManager.h"

AHealingBuilding::AHealingBuilding()
{
	HealZone = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HealZone"));
	HealZone->SetupAttachment(StaticMeshComp);

	HealZone->InitCapsuleSize(120.f, 220.f);

	HealZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HealZone->SetCollisionObjectType(ECC_WorldDynamic);
	HealZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	HealZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HealZone->SetGenerateOverlapEvents(true);
	HealZone->SetCanEverAffectNavigation(false);
}

void AHealingBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (HealZone)
	{
		HealZone->OnComponentBeginOverlap.AddDynamic(this, &AHealingBuilding::OnHealZoneBeginOverlap);
		HealZone->OnComponentEndOverlap.AddDynamic(this, &AHealingBuilding::OnHealZoneEndOverlap);
	}

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			HealTimerHandle,
			this,
			&AHealingBuilding::TickHealPlayers,
			HealTickInterval,
			true
		);
	}
}

void AHealingBuilding::OnRep_Dead()
{
	Super::OnRep_Dead();

	if (HealZone)
	{
		HealZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HealZone->SetGenerateOverlapEvents(false);
	}

	OverlappingPlayers.Empty();
}

void AHealingBuilding::OnHealZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	ABACharacter* Character = Cast<ABACharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	OverlappingPlayers.Add(Character);
}

void AHealingBuilding::OnHealZoneEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (!HasAuthority())
	{
		return;
	}

	ABACharacter* Character = Cast<ABACharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	OverlappingPlayers.Remove(Character);
}

void AHealingBuilding::TickHealPlayers()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	TArray<TWeakObjectPtr<ABACharacter>> InvalidPlayers;

	for (const TWeakObjectPtr<ABACharacter>& PlayerPtr : OverlappingPlayers)
	{
		ABACharacter* Character = PlayerPtr.Get();
		if (!IsValid(Character))
		{
			InvalidPlayers.Add(PlayerPtr);
			continue;
		}

		if (!CanHealCharacter(Character))
		{
			continue;
		}

		HealCharacter(Character);
	}

	for (const TWeakObjectPtr<ABACharacter>& InvalidPlayer : InvalidPlayers)
	{
		OverlappingPlayers.Remove(InvalidPlayer);
	}
}

bool AHealingBuilding::CanHealCharacter(ABACharacter* Character) const
{
	if (!IsValid(Character))
	{
		return false;
	}

	UAbilitySystemComponent* PlayerASC = Character->GetAbilitySystemComponent();
	if (!PlayerASC)
	{
		return false;
	}

	if (PlayerASC->HasMatchingGameplayTag(TAG_State_Combat_Dead))
	{
		return false;
	}

	const UHealthAttributeSet* CharacterHealthSet = PlayerASC->GetSet<UHealthAttributeSet>();
	if (!CharacterHealthSet)
	{
		return false;
	}

	return CharacterHealthSet->GetHealth() < CharacterHealthSet->GetMaxHealth();
}

void AHealingBuilding::HealCharacter(ABACharacter* Character)
{
	if (!IsValid(Character))
	{
		return;
	}

	UAbilitySystemComponent* PlayerASC = Character->GetAbilitySystemComponent();
	if (!PlayerASC)
	{
		return;
	}

	const UHealthAttributeSet* PlayerHealthSet = PlayerASC->GetSet<UHealthAttributeSet>();
	if (!PlayerHealthSet)
	{
		return;
	}

	const float MaxHealth = PlayerHealthSet->GetMaxHealth();
	if (MaxHealth <= 0.f || !HealEffect)
	{
		return;
	}

	const float HealAmount = MaxHealth * HealPercentPerSecond;

	FGameplayEffectSpecHandle SpecHandle = PlayerASC->MakeOutgoingSpec(HealEffect, 1.f, PlayerASC->MakeEffectContext());

	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Heal,
		HealAmount
	);

	PlayerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}