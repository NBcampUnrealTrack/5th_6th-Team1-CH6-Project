// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAParkourComponent.h"
#include "Player/BACharacter.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "CollisionShape.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UBAParkourComponent::UBAParkourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	TraceDistance = 150.0f;
	HighTraceHeight = 200.0f;
	bDrawDebug = true;
	bIsParkour = false;
}

// Called when the game starts
void UBAParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		MotionWarpingComp = Owner->FindComponentByClass<UMotionWarpingComponent>();
	}

}

bool UBAParkourComponent::AttemptParkour()
{
	if (DetectWall())
	{
		if (WallHeight >= 50.0f && !bIsParkour)
		{
			EParkourType TypeToPlay = EParkourType::None;

			if (WallHeight <= 150.0f) TypeToPlay = EParkourType::Vault;
			else if (WallHeight <= 250.0f) TypeToPlay = EParkourType::Climb;

			if (TypeToPlay != EParkourType::None)
			{
				ServerRPC_AttemptParkour(TypeToPlay, WarpTargetLocation, WarpTargetRotation);
				bIsParkour = true;
				return true;
			}
		}
	}

	return false;
}



// Called every frame
void UBAParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBAParkourComponent::ServerRPC_AttemptParkour_Implementation(EParkourType ParkourType, FVector TargetLocation, FRotator TargetRotation)
{
	Multicast_ExecuteParkour(ParkourType, TargetLocation, TargetRotation);
}

void UBAParkourComponent::Multicast_ExecuteParkour_Implementation(EParkourType ParkourType, FVector TargetLocation, FRotator TargetRotation)
{
	ABACharacter* Character = Cast<ABACharacter>(GetOwner());
	if (!Character || !MotionWarpingComp)
	{
		bIsParkour = false;
		return;
	}

	UAnimMontage* MontageToPlay = nullptr;
	FName WarpTargetName = NAME_None;

	switch (ParkourType)
	{
	case EParkourType::Climb:
		MontageToPlay = ClimbMontage;
		WarpTargetName = FName("ClimbTarget");
		break;
	case EParkourType::Vault:
		MontageToPlay = VaultMontage;
		WarpTargetName = FName("VaultTarget");
		break;
	}

	if (MontageToPlay)
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		/*Character->bUseControllerRotationYaw = false;
		Character->SpringArmRot(false);*/


		/*if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			FRotator NewRot = Character->GetActorRotation();
			NewRot.Pitch = PC->GetControlRotation().Pitch;
			NewRot.Roll = 0.0f;
			PC->SetControlRotation(NewRot);
			PC->SetIgnoreLookInput(true);
		}*/

		MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
			WarpTargetName,
			TargetLocation,
			TargetRotation
		);

		float Duration = Character->PlayAnimMontage(MontageToPlay);

		if (Duration > 0.f)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UBAParkourComponent::OnParkourMontageEnded);
			Character->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
		}
		else
		{
			OnParkourMontageEnded(MontageToPlay, true);
		}
	}
	else
	{
		bIsParkour = false;
	}
}

bool UBAParkourComponent::DetectWall()
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	WallHeight = 0.0f;
	WallThickness = 0.0f;
	WarpTargetLocation = FVector::ZeroVector;
	WarpTargetRotation = FRotator::ZeroRotator;
	CurrentDepth = 0.0f;
	WallHitResult.Reset();

	FVector Start = Owner->GetActorLocation();
	Start.Z += 50.f;
	FVector Forward = Owner->GetActorForwardVector();

	FVector End = Start + (Forward * TraceDistance);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHitCenter = GetWorld()->SweepSingleByChannel(
		WallHitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel5,
		SphereShape,
		Params
	);

	if (bDrawDebug)
	{
		FColor Color = bHitCenter ? FColor::Green : FColor::Red;
		DrawDebugLine(GetWorld(), Start, End, Color, false, 1.f, 0, 2.f);
	}

	if (!bHitCenter) return false;
	FVector WallInnerDir = -WallHitResult.ImpactNormal;
	WallInnerDir.Z = 0.f;
	WallInnerDir.Normalize();
	FHitResult TopHitResult;
	bool bFoundValidTop = false;


	for(int32 i = 0; i < MaxAttempts; ++i)
	{
		FVector HighStart = WallHitResult.ImpactPoint + FVector(0, 0, HighTraceHeight) + (WallInnerDir * CurrentDepth);
		FVector HighEnd = HighStart - FVector(0, 0, HighTraceHeight + 50.f);

		bool bHitTop = GetWorld()->SweepSingleByChannel(
			TopHitResult,
			HighStart,
			HighEnd,
			FQuat::Identity,
			ECC_GameTraceChannel5,
			SphereShape,
			Params
		);

		if (bDrawDebug) DrawDebugLine(GetWorld(), HighStart, HighEnd, bHitTop ? FColor::Cyan : FColor::Red, false, 2.f, 0, 1.f);

		if (bHitTop)
		{
			if (TopHitResult.ImpactNormal.Z > 0.7f)
			{
				bFoundValidTop = true;
				break;
			}
			else
			{
				CurrentDepth += DepthStep;
			}
		}
		else
		{
			break;
		}
	}

	if (bFoundValidTop)
	{
		ABACharacter* Character = Cast<ABACharacter>(Owner);
		if (Character)
		{
			float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			float FeetZ = Character->GetActorLocation().Z - CapsuleHalfHeight;

			WallHeight = TopHitResult.ImpactPoint.Z - FeetZ;
		}

		float ForwardOffset = 10.f; 

		float HeightOffset = 2.f;

		FVector NormalDir = WallHitResult.ImpactNormal;
		FVector UpDir = FVector::UpVector;

		WarpTargetLocation = TopHitResult.ImpactPoint + (NormalDir * ForwardOffset) + (UpDir * HeightOffset);
		WarpTargetRotation = FRotator(0.0f, (-NormalDir).Rotation().Yaw, 0.0f);

		if (bDrawDebug) DrawDebugSphere(GetWorld(), WarpTargetLocation, 10.0f, 12, FColor::Yellow, false, 2.0f);

		FVector LandStart = TopHitResult.ImpactPoint + (WallInnerDir * 100.0f);
		FVector LandEnd = LandStart - FVector(0, 0, WallHeight + 50.f);

		FHitResult LandHitResult;

		bool bHitLand = GetWorld()->LineTraceSingleByChannel(
			LandHitResult,
			LandStart,
			LandEnd,
			ECC_WorldStatic,
			Params
		);

		if (bDrawDebug) DrawDebugLine(GetWorld(), LandStart, LandEnd, bHitLand ? FColor::Purple : FColor::Red, false, 2.0f, 0, 2.0f);

		if (bHitLand)
		{
			WallThickness = FVector::DistXY(TopHitResult.ImpactPoint, LandHitResult.ImpactPoint);
			UE_LOG(LogTemp, Warning, TEXT("착지 가능! 두께: %f"), WallThickness);
		}
		else
		{
			WallThickness = 9999.f;
		}

		return true;
	}
	return false;
}

void UBAParkourComponent::OnParkourMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ABACharacter* Character = Cast<ABACharacter>(GetOwner());
	if (!Character) return;

	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);


	/*Character->bUseControllerRotationYaw = true;
	Character->SpringArmRot(true);*/
	/*if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		PC->SetIgnoreLookInput(false);
	}*/
	UE_LOG(LogTemp, Warning, TEXT("파쿠르 종료! 카메라 동기화 완료."));
	bIsParkour = false;
}