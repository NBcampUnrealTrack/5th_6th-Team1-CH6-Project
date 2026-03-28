#include "Weapon/Sniper/WeaponSniper.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Player/BACharacter.h"
#include "Framework/BAGameState.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UI/UISubsystem.h"
#include "UI/UW_Scope.h"
#include "Kismet/GameplayStatics.h"

AWeaponSniper::AWeaponSniper()
{
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(WeaponMesh, "ADS_Sight");	
}

void AWeaponSniper::StartNightVision()
{
	if (!SceneCapture || !NightVisionMaterial) return;

	if (!NightVisionMID)
	{
		NightVisionMID = UMaterialInstanceDynamic::Create(NightVisionMaterial, this);
	}

	SceneCapture->PostProcessSettings.WeightedBlendables.Array.Empty();

	SceneCapture->PostProcessSettings.WeightedBlendables.Array.Add(
		FWeightedBlendable(1.0f, NightVisionMID)
	);

	SceneCapture->PostProcessSettings.bOverride_AutoExposureMethod = true;
	SceneCapture->PostProcessSettings.bOverride_AutoExposureBias = true;

	SceneCapture->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;

	SceneCapture->PostProcessSettings.AutoExposureBias = 8.0f;

	if (NightVisionOnSound)
	{
		UGameplayStatics::PlaySound2D(GetOwner(), NightVisionOnSound);
	}
}

void AWeaponSniper::StopNightVision()
{
	if (!SceneCapture) return;

	SceneCapture->PostProcessSettings.WeightedBlendables.Array.Empty();

	SceneCapture->PostProcessSettings.bOverride_AutoExposureMethod = false;
	SceneCapture->PostProcessSettings.bOverride_AutoExposureBias = false;

	if (NightVisionOffSound)
	{
		UGameplayStatics::PlaySound2D(GetOwner(), NightVisionOffSound);
	}
}

void AWeaponSniper::SceneCaptureHideArrowMesh(ABACharacter* Player)
{
	if (!IsValid(Player)) return;
	SceneCapture->HideComponent(Player->GetArrowMesh());
}

void AWeaponSniper::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		ABAGameState* GS = Cast<ABAGameState>(World->GetGameState());
		if (GS)
		{
			const TArray<TWeakObjectPtr<ABACharacter>> CharacterArray = GS->GetActiveCharacters();

			for (const auto& Player : CharacterArray)
			{
				if (!Player.IsValid()) continue;	
				SceneCaptureHideArrowMesh(Player.Get());
				Player->OnDropDelegate.AddDynamic(this, &AWeaponSniper::SceneCaptureHideArrowMesh);
			}
		}
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		if (Character->IsLocallyControlled())
		{
			SceneCapture->TextureTarget = RT;
			SceneCapture->SetActive(false);
			SceneCapture->bCaptureEveryFrame = true;
			SceneCapture->SetComponentTickEnabled(true);

			APlayerController* FPC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
			if (!GetWorld()) return;
			ULocalPlayer* LP = FPC->GetLocalPlayer();
			if (!LP) return;

			UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
			if (IsValid(UISubsystem))
			{
				UUW_Scope* Scope = UISubsystem->ShowUI<UUW_Scope>(EUIType::Scope);
				Scope->InitScope(RT);
				UISubsystem->HideUI(EUIType::Scope);
			}		
		}
	}

	
}
