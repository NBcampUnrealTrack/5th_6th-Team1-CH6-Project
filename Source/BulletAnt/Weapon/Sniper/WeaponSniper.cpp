#include "Weapon/Sniper/WeaponSniper.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Player/BACharacter.h"
#include "Framework/BAGameState.h"

AWeaponSniper::AWeaponSniper()
{
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(WeaponMesh, "ADS_Sight");

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

			for (auto Character : CharacterArray)
			{
				SceneCapture->HideComponent(Character->GetArrowMesh());
			}
		}
	}
}
