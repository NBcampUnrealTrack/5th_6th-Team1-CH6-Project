#include "Player/BAAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/BAParkourComponent.h"
#include "GAS/BAGameplayTags.h"
#include "Weapon/BaseRangedWeapon.h"

void UBAAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Owner = TryGetPawnOwner();

	if (Owner)
	{
		Character = Cast<ABACharacter>(Owner);

		if (Character)
		{
			Movement = Character->GetCharacterMovement();
			ParkourComp = Character->FindComponentByClass<UBAParkourComponent>();
		}
	}

}

void UBAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	//캐릭터 없으면 nullptr 반환
	if (Character == nullptr || Movement == nullptr) return;

	CurrentEquipmentType = Character->CurrentEquipmentType;
	AActor* OwningActor = GetOwningActor();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);

	if (ASC)
	{
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged))
			CurrentEquipmentType = EEquipmentType::Ranged;
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Mining))
			CurrentEquipmentType = EEquipmentType::Mining;
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Melee))
			CurrentEquipmentType = EEquipmentType::Melee;
	}

	//UCharacterMovementComponent에서 Velocity 변수 가져오기
	FVector Velocity = Movement->Velocity;

	GroundSpeed = Velocity.Size2D();

	FRotator Rotation = Character->GetActorRotation();

	Direction = (GroundSpeed > 3.f)
		? UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation)
		: 0.f;

	float FinalPitch = Character->SyncAimPitch;
	float FinalYaw = Character->SyncAimYaw;

	FRotator AimRot = FRotator(FinalPitch, FinalYaw, 0.f);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, Rotation);
	if (bIsAiming)
	{
		DeltaRot = CameraTargetOffset();
	}
	TargetPitch = (FMath::Abs(DeltaRot.Pitch) > 90.0f) ? 0.0f : DeltaRot.Pitch;
	TargetYaw = (FMath::Abs(DeltaRot.Yaw) > 90.0f) ? 0.0f : DeltaRot.Yaw;

	AOPitch = FMath::FInterpTo(AOPitch, TargetPitch, DeltaSeconds, 15.0f);
    AOYaw = FMath::FInterpTo(AOYaw, TargetYaw, DeltaSeconds, 15.0f);
	//if(ParkourComp&&!ParkourComp->bisParkour
	HandIKAlpha = GetCurveValue(FName("HandIK_Alpha"));
	if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged))
	{
		if (Character->EquippedWeapon)
		{
			USkeletalMeshComponent* WeaponMesh = Character->EquippedWeapon->GetWeaponMesh();
			LeftHandIKLoc = Character->EquippedWeapon->GetWeaponMesh()->GetSocketLocation(FName("LeftHandSocket"));
		}
	}
	FVector RightDir = OwningActor->GetActorRightVector();

	float ShoulderWidth = 30.f;
	LeftTargetLocation = ParkourComp->WarpTargetLocation - (RightDir * ShoulderWidth);
	RightTargetLocation = ParkourComp->WarpTargetLocation + (RightDir * ShoulderWidth);
	bIsAiming = Character->bIsAiming;
	bIsTurning = Character->bIsTurning;
	
	bIsFalling = Movement->IsFalling();
	bIsCrouch = Character->bIsCrouched;
	bIsRunning = Character->bIsRunning;
	VerticalVelocity = Velocity.Z;

	RootYawOffset = Character->RootYawOffset * -1;

	//Test
	

}

void UBAAnimInstance::SetIsFiring(bool InFiring)
{
	bIsFiring = InFiring;
}

FRotator UBAAnimInstance::CameraTargetOffset()
{
	if (!Character || !Character->GetController())
		return FRotator::ZeroRotator;
	FVector CamLoc; FRotator CamRot;
	Character->GetController()->GetPlayerViewPoint(CamLoc, CamRot);

	FVector TraceEnd = CamLoc + (CamRot.Vector() * 5000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC_Visibility, Params);

	FVector TargetLocation = bHit ? Hit.ImpactPoint : TraceEnd;

	FVector StartLocation = Character->GetMesh()->GetSocketLocation("head");

	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

	FRotator ActorRot = Character->GetActorRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRot, ActorRot);

	return DeltaRot;
}
