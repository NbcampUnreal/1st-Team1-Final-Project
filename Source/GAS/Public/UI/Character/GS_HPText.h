#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HPText.generated.h"

class UGS_StatComp;
class UTextBlock;
class UProgressBar;

UCLASS()
class GAS_API UGS_HPText : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_HPText(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	void InitializeHPTextWidget(UGS_StatComp* InStatComp);

	AActor* GetOwningActor()const { return OwningActor; }

	void SetOwningActor(AActor* InOwningActor) { OwningActor = InOwningActor; }

	UFUNCTION()
	void OnCurrentHPChanged(UGS_StatComp* InStatComp);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentHPText;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> HPBar;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor>OwningActor;
};
