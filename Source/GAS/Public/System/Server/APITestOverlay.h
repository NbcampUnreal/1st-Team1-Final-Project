#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "APITestOverlay.generated.h"

class UAPITestManger;

UCLASS()
class GAS_API UAPITestOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAPITestManger> APITestMangerClass;

protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY()
	TObjectPtr<UAPITestManger> APITestManger;
};
