#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BAPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedPlayerColor, FLinearColor);

UCLASS()
class BULLETANT_API ABAPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* NewPlayerState) override;

public:
	void SetPlayerColorIdx(int32 NewIdx);
	UFUNCTION(BlueprintCallable)
	FLinearColor GetPlayerColor() const;

	FDelegateHandle BindOnChangedPlayerColor(const FOnChangedPlayerColor::FDelegate& Delegate);
	void UnbindOnChangedPlayerColor(const UObject* Object);
	void UnbindOnChangedPlayerColor(FDelegateHandle Handle);

protected:
	UFUNCTION()
	void OnRep_PlayerColorIdx();

protected:
	FOnChangedPlayerColor OnChangedPlayerColor;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColorIdx)
	int32 PlayerColorIdx = -1;

	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> PlayerColorTable;
};
