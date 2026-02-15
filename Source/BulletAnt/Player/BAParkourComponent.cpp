// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAParkourComponent.h"
#include "Player/BACharacter.h"
#include "MotionWarpingComponent.h"
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
		if (WallHeight >= 50.0f)
		{
			EParkourType TypeToPlay = EParkourType::None;

			if (WallHeight <= 60.0f) TypeToPlay = EParkourType::Vault;
			else if (WallHeight <= 250.0f) TypeToPlay = EParkourType::Climb;

			if (TypeToPlay != EParkourType::None)
			{
				ServerRPC_AttemptParkour(TypeToPlay, WarpTargetLocation, WarpTargetRotation);
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
	if (!Character || !MotionWarpingComp) return;

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
		Character->SetActorEnableCollision(false);
		
		MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
			WarpTargetName,
			TargetLocation,
			TargetRotation
		);
		USkeletalMeshComponent* FPSMesh = Character->GetFPSMesh();
		if (FPSMesh && FPSMesh->GetAnimInstance())
		{
			FPSMesh->GetAnimInstance()->Montage_Play(MontageToPlay);
		}
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			FRotator ViewRotation = TargetRotation;

			ViewRotation.Pitch = 0.0f;

			PC->SetControlRotation(ViewRotation);
		}

		Character->PlayAnimMontage(MontageToPlay);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UBAParkourComponent::OnParkourMontageEnded);
		Character->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
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

	WallHitResult.Reset();

	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();
	Start.Z += 10.0f;

	FVector End = Start + (Forward * TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHitCenter = GetWorld()->LineTraceSingleByChannel(
		WallHitResult,
		Start,
		End,
		ECC_WorldStatic,
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

	int32 MaxAttempts = 3;
	float CurrentDepth = 30.f;
	float DepthStep = 40.f;


	for(int32 i = 0; i < MaxAttempts; ++i)
	{
		FVector HighStart = WallHitResult.ImpactPoint + FVector(0, 0, HighTraceHeight) + (WallInnerDir * CurrentDepth);
		FVector HighEnd = HighStart - FVector(0, 0, HighTraceHeight + 50.f);

		bool bHitTop = GetWorld()->LineTraceSingleByChannel(TopHitResult, HighStart, HighEnd, ECC_WorldStatic, Params);

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
		WallHeight = TopHitResult.ImpactPoint.Z - Owner->GetActorLocation().Z;

		WarpTargetLocation = TopHitResult.ImpactPoint + (WallHitResult.ImpactNormal * 5.f);
		FRotator TempRot = (-WallHitResult.ImpactNormal).Rotation();
		WarpTargetRotation = FRotator(0.0f, TempRot.Yaw, 0.0f);

		if (bDrawDebug) DrawDebugSphere(GetWorld(), WarpTargetLocation, 10.0f, 12, FColor::Yellow, false, 2.0f);

		FVector LandStart = TopHitResult.ImpactPoint + (WallInnerDir * 100.0f);
		FVector LandEnd = LandStart - FVector(0, 0, 250.0f);

		FHitResult LandHitResult;

		bool bHitLand = GetWorld()->LineTraceSingleByChannel(
			LandHitResult, LandStart, LandEnd, ECC_WorldStatic, Params
		);

		if (bDrawDebug) DrawDebugLine(GetWorld(), LandStart, LandEnd, bHitLand ? FColor::Purple : FColor::Red, false, 2.0f, 0, 2.0f);

		if (bHitLand)
		{
			WallThickness = FVector::Dist(TopHitResult.ImpactPoint, LandHitResult.ImpactPoint);
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

void UBAParkourComponent::ExecuteParkour(EParkourType ParkourType)
{
	ABACharacter* Character = Cast<ABACharacter>(GetOwner());
	if (!Character || !MotionWarpingComp) return;

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
		Character->SetActorEnableCollision(false);

		MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
			WarpTargetName,
			WarpTargetLocation,
			WarpTargetRotation
		);

		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(MontageToPlay);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UBAParkourComponent::OnParkourMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
		}
	}
}

void UBAParkourComponent::OnParkourMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	Character->SetActorEnableCollision(true);

	UE_LOG(LogTemp, Warning, TEXT("파쿠르 종료! 다시 걷기 모드."));
}