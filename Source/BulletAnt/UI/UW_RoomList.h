#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_RoomList.generated.h"

class UUW_RoomListItem;
class UUniformGridPanel;
class UButton;

UCLASS()
class BULLETANT_API UUW_RoomList : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION()
	void RefreshList();

protected:
	void OnUpdateRooms(bool bSuccessful);
	void UpdateList();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUW_RoomListItem> ItemClass;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ItemParent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnRefresh;

	FDelegateHandle UpdateHandle;
};
