#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BAGameInstance.generated.h"

class UMapConfig;
class USoundMix;
class USoundClass;

UCLASS()
class BULLETANT_API UBAGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void OnStart() override;

	UMapConfig* GetMapConfig() { return MapConfig; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMapConfig> MapConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundMix> MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> SfxSoundClass;
};
