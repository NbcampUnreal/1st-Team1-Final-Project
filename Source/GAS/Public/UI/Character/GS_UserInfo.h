#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_UserInfo.generated.h"

class UImage;
class UTextBlock;
class AGS_PlayerState;

UCLASS()
class GAS_API UGS_UserInfo : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_UserID;
	UPROPERTY(meta = (BindWidget))
	UImage* SteamAvatar;
	
	void SetupWidget(AGS_PlayerState* PlayerState);
};
