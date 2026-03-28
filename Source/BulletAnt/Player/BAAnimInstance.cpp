#include "Player/BAAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/BAParkourComponent.h"
#include "GAS/BAGameplayTags.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Player/BAPlayerState.h"

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
			GrabLeftHand = 1.f;
			OwningActor = GetOwningActor();
			
		}
	}

}

void UBAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	//캐릭터 없으면 nullptr 반환
	if (Character == nullptr || Movement == nullptr) return;

	float ShoulderWidth = 30.f;
	bIsAiming = Character->bIsAiming;
	bIsTurning = Character->bIsTurning;
	bIsParkour = ParkourComp->bIsParkour;
	bIsFalling = Movement->IsFalling();
	bIsCrouch = Character->bIsCrouched;
	bIsRunning = Character->bIsRunning;
	LowerYaw = Character->RootYawOffset;
	bIsJetPack = Character->bIsJetPack;
	if (!ASC)
	{
		ABAPlayerState* PS = Cast<ABAPlayerState>(Character->GetPlayerState());
		if (PS)
		{
			ASC = PS->GetAbilitySystemComponent();
		}
	}

	CurrentEquipmentType = Character->CurrentEquipmentType;

	if (ASC)
	{
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged))
			CurrentEquipmentType = EEquipmentType::Ranged;
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Mining))
			CurrentEquipmentType = EEquipmentType::Mining;
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Melee))
			CurrentEquipmentType = EEquipmentType::Melee;
		if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Jetpack))
			CurrentEquipmentType = EEquipmentType::Jetpack;
	}

	//UCharacterMovementComponent에서 Velocity 변수 가져오기
	FVector Velocity = Movement->Velocity;

	GroundSpeed = Velocity.Size2D();

	FRotator Rotation = Character->GetActorRotation();
	FRotator DeltaRot = FRotator::ZeroRotator;
	Direction = (GroundSpeed > 3.f)
		? UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation)
		: 0.f;

	

	
	/*FVector AimDir = FRotator(FinalPitch, FinalYaw, 0.f).Vector();
	FVector LocalAimDir = Rotation.Quaternion().Inverse().RotateVector(AimDir);
	FRotator DeltaRot = LocalAimDir.Rotation();*/
	if (Character->IsLocallyControlled())
	{
		if(ASC && !ASC->HasMatchingGameplayTag(TAG_State_Combat_ADS))
		{
			if (bIsAiming || bIsFiring)
				DeltaRot = CameraTargetOffset();
			else
			{
				FRotator ControlRot = Character->GetControlRotation();
				DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, Rotation);
			}
		}
		else
		{
			FRotator ControlRot = Character->GetControlRotation();
			DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, Rotation);
		}
	}
	else
	{
		float FinalPitch = Character->SyncAimPitch;
		float FinalYaw = Character->SyncAimYaw;

		FRotator AimRot = FRotator(FinalPitch, FinalYaw, 0.f);
		DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, Rotation);
	}
	if (bIsAiming||bIsFiring)
	{
		DeltaRot.Pitch *= -1;
		AOPitch = FMath::FInterpTo(AOPitch, DeltaRot.Pitch, DeltaSeconds, 0.f);
		AOYaw = FMath::FInterpTo(AOYaw, DeltaRot.Yaw, DeltaSeconds, 0.f);
	}
	else
	{
		AOPitch = FMath::FInterpTo(AOPitch, DeltaRot.Pitch, DeltaSeconds, 15.f);
		AOYaw = FMath::FInterpTo(AOYaw, DeltaRot.Yaw, DeltaSeconds, 15.f);
	}
	//if(ParkourComp&&!ParkourComp->bisParkour
	if (Character->EquippedWeapon && Character->EquippedWeapon->GetWeaponMesh()->DoesSocketExist("LeftHandSocket"))
	{
		FTransform GunLeftHandSocket = Character->EquippedWeapon->GetWeaponMesh()->GetSocketTransform("LeftHandSocket");
		FTransform Righthand = Character->GetMesh()->GetSocketTransform("hand_r");
		LeftHandIK_Transform = GunLeftHandSocket.GetRelativeTransform(Righthand);
	}
	FVector RightDir = OwningActor->GetActorRightVector();
	LeftTargetLocation = ParkourComp->WarpTargetLocation - (RightDir * ShoulderWidth);
	RightTargetLocation = ParkourComp->WarpTargetLocation + (RightDir * ShoulderWidth);
	
	VerticalVelocity = Velocity.Z;

	IsGrabLeftHand(DeltaSeconds);
	//Test
	

}

void UBAAnimInstance::SetIsFiring(bool InFiring)
{
	bIsFiring = InFiring;
}

FRotator UBAAnimInstance::CameraTargetOffset()
{
	if (!Character)
		return FRotator::ZeroRotator;
	if (Character->IsLocallyControlled() && Character->GetController())
	{
		FVector CamLoc;
		FRotator CamRot;
		Character->GetController()->GetPlayerViewPoint(CamLoc, CamRot);

		FVector TraceEnd = CamLoc + (CamRot.Vector() * 10000.0f);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);
		if (Character->EquippedWeapon)
			Params.AddIgnoredActor(Character->EquippedWeapon);

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC_GameTraceChannel11, Params);

		FVector TargetLocation = bHit ? Hit.ImpactPoint : TraceEnd;

		FVector StartLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, Character->BaseEyeHeight);

		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

		FRotator ActorRot = Character->GetActorRotation();
		return UKismetMathLibrary::NormalizedDeltaRotator(LookAtRot, ActorRot);
	}
	else
	{
		FVector TargetLocation = Character->ReplicatedAimTarget;

		FVector StartLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, Character->BaseEyeHeight);

		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

		FRotator ActorRot = Character->GetActorRotation();
		return UKismetMathLibrary::NormalizedDeltaRotator(LookAtRot, ActorRot);
	}
}

void UBAAnimInstance::IsGrabLeftHand(float DeltaSeconds)
{
	if (!ASC) return;
	float TargetAlpha = (bIsParkour|| ASC->HasMatchingGameplayTag(TAG_State_Combat_Dead)) ? 0.f : 1.f;

	GrabLeftHand = FMath::FInterpTo(GrabLeftHand, TargetAlpha, DeltaSeconds, 15.f);
}
