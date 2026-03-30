// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UISubsystem.h"
#include "UI/UW_GameOver.h"
#include "GAS/BAGameplayTags.h"

void ABaseCore::Use_Implementation(AActor* User)
{
}

const TArray<FVector>& ABaseCore::GetAnchors() const
{
	return Anchors;
}

ABaseCore::ABaseCore()
{
    StaticMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    StaticMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel6, ECollisionResponse::ECR_Block); // Enemy
}

void ABaseCore::BeginPlay()
{
	Super::BeginPlay();

	InitializeCoreMaterial();

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (ABAGameState* GS = World->GetGameState<ABAGameState>())
			{
				GS->SetTargetCore(this);
			}
		}

        HealthSet->SetMaxHealth(5000.f);
        HealthSet->SetHealth(5000.f);

		FindAnchors();

		GetWorldTimerManager().SetTimer(
			RegenTimerHandle,
			this,
			&ThisClass::HandleRegen,
			1.0f,
			true
		);
	}

	HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetHealthAttribute()).AddUObject(this, &ABaseCore::HandleHealthChanged);
	UpdateCoreMaterialHealthRatio();
}

void ABaseCore::OnDeath()
{
	if (HasAuthority())
	{
		Multi_ShowResult();
	}
}

void ABaseCore::Multi_ShowResult_Implementation()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (UISubsystem)
	{
		UUW_GameOver* Widget = UISubsystem->ShowUI<UUW_GameOver>(EUIType::GameOver);
		UISubsystem->ApplyUIOnlyInputMode(Widget);
		Widget->InitText(false);
	}
}

void ABaseCore::FindAnchors()
{
    Anchors.Reserve(ScanCount);

    const float ScanRadius = 1300.f; 
    FVector Start = GetActorLocation();
    Start.Z = 0;

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    for (int32 i = 0; i < ScanCount; ++i)
    {
        float Angle = i * (360.f / ScanCount);
        FVector Direction = FRotator(0.f, Angle, 0.f).Vector();
        FVector End = Start + Direction * ScanRadius;
        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByObjectType(Hit, End, Start, ObjectParams))
        {
            FVector HitLocation = Hit.Location;
            HitLocation += Direction * 50;
            Anchors.Add(HitLocation);
        }
    }
}

void ABaseCore::InitializeCoreMaterial()
{
	if (!StaticMeshComp)
	{
		return;
	}

	if (CoreMaterial)
	{
		StaticMeshComp->SetMaterial(0, CoreMaterial);
	}

	CoreMID = StaticMeshComp->CreateDynamicMaterialInstance(0);

	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue(TEXT("HealthRatio"), 1.0f);
	}
}

void ABaseCore::UpdateCoreMaterialHealthRatio()
{
	if (!CoreMID || !HealthSet)
	{
		return;
	}

	const float MaxHealth = HealthSet->GetMaxHealth();
	const float CurrentHealth = HealthSet->GetHealth();
	const float Ratio = (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;

	CoreMID->SetScalarParameterValue(TEXT("HealthRatio"), Ratio);
}

void ABaseCore::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	UpdateCoreMaterialHealthRatio();
}

void ABaseCore::HandleRegen()
{
	if (bDead || !ASC || !HealthSet || !RegenHealEffect)
	{
		return;
	}

	const float MaxHealth = GetMaxHealth();
	const float CurrentHealth = GetCurrentHealth();

	if (CurrentHealth >= MaxHealth || MaxHealth <= 0.f)
	{
		return;
	}

	const float HealAmount = MaxHealth * RegenPercentPerSecond;

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(RegenHealEffect, 1.f, ASC->MakeEffectContext());

	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Heal,
		HealAmount
	);

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
