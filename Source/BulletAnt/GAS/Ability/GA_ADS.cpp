#include "GAS/Ability/GA_ADS.h"

#include "GAS/BAGameplayTags.h"
#include "Player/BACharacter.h"
#include "Player/BAPlayerController.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/BaseWeapon.h"
#include "Camera/CameraComponent.h"

UGA_ADS::UGA_ADS()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_ADS);

	SetAssetTags(DefaultTag);

	ActivationOwnedTags.AddTag(TAG_State_Combat_ADS);
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

}

void UGA_ADS::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ADS::StartADS()
{
	SavedTargetPoint = ADSLineTrace();
	FVector SightLoc = CachedWeapon->GetWeaponMesh()->GetSocketLocation("ADS_Sight");
	FRotator SightRot = CachedWeapon->GetWeaponMesh()->GetSocketRotation("ADS_Sight");

	FRotator IdealLookAtRot = UKismetMathLibrary::FindLookAtRotation(SightLoc, SavedTargetPoint);
	FRotator ErrorDelta = UKismetMathLibrary::NormalizedDeltaRotator(IdealLookAtRot, SightRot);

	FRotator CurrentControlRot = PC->GetControlRotation();
	FRotator NewControlRot = CurrentControlRot + ErrorDelta;

	USpringArmComponent* SpringArm = Source->GetSpringArm();
	Source->bUseControllerRotationYaw = true;
	SpringArm->bUsePawnControlRotation = false;
	Source->GetCamera()->FieldOfView = 70.f;
	Source->AimStart(1.f);

	SavedSpringArmTransform = SpringArm->GetRelativeTransform();
	SpringArm->AttachToComponent(CachedWeapon->GetWeaponMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "ADS_Sight");
	SpringArm->TargetArmLength = 0.f;
	PC->SetControlRotation(NewControlRot);

	if (IsLocallyControlled())
	{
		Source->GetMesh()->HideBoneByName(FName("head"), EPhysBodyOp::PBO_None);
	}
}

void UGA_ADS::StopADS()
{
	Source->bUseControllerRotationYaw = false;
	USpringArmComponent* SpringArm = Source->GetSpringArm();
	UCameraComponent* Camera = Source->GetCamera();
	SpringArm->bUsePawnControlRotation = true;
	Source->GetCamera()->FieldOfView = 90.f;
	PC->StopADSUI();
	Source->AimStop(1.f);

	SpringArm->AttachToComponent(Source->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	SpringArm->SetRelativeTransform(SavedSpringArmTransform);
	SpringArm->TargetArmLength = 223.f;
	FVector RestoredCamLoc = Camera->GetComponentLocation();
	FRotator NewLookAtRot = UKismetMathLibrary::FindLookAtRotation(RestoredCamLoc, SavedTargetPoint);
	PC->SetControlRotation(NewLookAtRot);

	if (IsLocallyControlled())
	{
		Source->GetMesh()->UnHideBoneByName(FName("head"));
	}
}

FVector UGA_ADS::ADSLineTrace()
{
	FVector CamLoc;
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
