#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CustomCommonButton.generated.h"


UCLASS()
class GAS_API UCustomCommonButton : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ChangeLayerIconImage(int32 LayerIndex);
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ChangeText(int32 LayerIndex);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* IconTexture0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* IconTexture1;
};
