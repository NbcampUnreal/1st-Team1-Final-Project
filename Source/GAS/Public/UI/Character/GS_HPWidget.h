#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HPWidget.generated.h"


class UGS_StatComp;
class AGS_Character;
class UProgressBar;
class UImage;

UCLASS()
class GAS_API UGS_HPWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_HPWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	void InitializeHPWidget(UGS_StatComp* InStatComp);
	
	AGS_Character* GetOwningActor()const { return OwningCharacter; }

	void SetOwningActor(AGS_Character* InOwningCharacter) { OwningCharacter = InOwningCharacter; }

	UFUNCTION()
	void OnCurrentHPBarChanged(UGS_StatComp* InStatComp);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UProgressBar> HPBarWidget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;

	float MaxHealth;
};