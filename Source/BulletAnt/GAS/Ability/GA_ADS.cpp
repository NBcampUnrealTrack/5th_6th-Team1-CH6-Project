#include "GAS/Ability/GA_ADS.h"

#include "GAS/BAGameplayTags.h"
#include "Player/BACharacter.h"
#include "Player/BAPlayerController.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/BaseWeapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "UI/UW_PlayerHUDWidget.h"

UGA_ADS::UGA_ADS()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_ADS);

	SetAssetTags(DefaultTag);

	ActivationOwnedTags.AddTag(TAG_State_Combat_ADS);

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Ability_Active_ADS;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggers.Add(TriggerData);

	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active_ADS);
}

void UGA_ADS::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	Source = Cast<ABACharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Source)) return;

	PC = Cast<ABAPlayerController>(Source->GetController());
	if (!IsValid(PC)) return;

	CachedWeapon = Cast<ABaseWeapon>(TriggerEventData->OptionalObject);
	if (!IsValid(CachedWeapon)) return;

	StartADS();

	UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_Combat_EndADS);
	Task->EventReceived.AddDynamic(this, &UGA_ADS::StopADS);
	Task->ReadyForActivation();
}

void UGA_ADS::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Source->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	Source->bUseControllerRotationYaw = false;
	if (IsLocallyControlled())
	{
		USpringArmComponent* SpringArm = Source->GetSpringArm();
		SpringArm->bUsePawnControlRotation = true;
		Source->GetCamera()->FieldOfView = 90.f;
		Source->EndAiming();

		SpringArm->AttachToComponent(Source->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		SpringArm->SetRelativeTransform(SavedSpringArmTransform);
		SpringArm->SocketOffset = FVector(0.f, 0.f, 0.f);

		UUW_PlayerHUDWidget* HUD = PC->GetHUD();
		HUD->SetCrossHairImage(false);
		if (UWeaponDataAsset* Data = CachedWeapon->GetWeaponData())
		{
			if (Data->WeaponType == EWeaponType::Sniper)
			{
				PC->StopADSUI();
			}
		}

		Source->GetMesh()->UnHideBoneByName(FName("head"));
	}	

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ADS::StartADS()
{
	Source->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	Source->bUseControllerRotationYaw = true;	
	if (IsLocallyControlled())
	{		
		FVector SightLoc = CachedWeapon->GetWeaponMesh()->GetSocketLocation("ADS_Sight");
		FRotator SightRot = CachedWeapon->GetWeaponMesh()->GetSocketRotation("ADS_Sight");

		USpringArmComponent* SpringArm = Source->GetSpringArm();		
		SpringArm->bUsePawnControlRotation = false;
		Source->GetCamera()->FieldOfView = 60.f;
		Source->StartAiming();

		SavedSpringArmTransform = SpringArm->GetRelativeTransform();
		SpringArm->AttachToComponent(CachedWeapon->GetWeaponMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "ADS_Sight");
		SpringArm->SocketOffset = FVector(228.f, 0.f, 0.f);

		Source->GetMesh()->HideBoneByName(FName("head"), EPhysBodyOp::PBO_None);

		UUW_PlayerHUDWidget* HUD = PC->GetHUD();
		HUD->SetCrossHairImage(true);
		
		if (UWeaponDataAsset* Data = CachedWeapon->GetWeaponData())
		{
			if (Data->WeaponType == EWeaponType::Sniper)
			{
				PC->StartADSUI();
			}
		}
	}

}

void UGA_ADS::StopADS(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

FVector UGA_ADS::ADSLineTrace()
{
	FVector CamLoc = Source->GetCamera()->GetComponentLocation();
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);
	FVector TraceEnd = CamLoc + (CamRot.Vector() * 10000.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Source);
	if (CachedWeapon)
		Params.AddIgnoredActor(CachedWeapon);
	GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC_Visibility, Params);

	return Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
}
