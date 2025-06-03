// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_SkillInputHandlerComp.generated.h"

class UInputAction;
class UInputMappingContext;
class UGS_SkillComp;
class AGS_Character;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_SkillInputHandlerComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGS_SkillInputHandlerComp();
	void SetupEnhancedInput(class UInputComponent* PlayerInputComponent);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnRightClick(const struct FInputActionInstance& Instance);

	UFUNCTION()
	virtual void OnLeftClick(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void OnCtrlModifierStarted();

	UFUNCTION()
	void OnCtrlModifierEnded();

	UFUNCTION()
	virtual void OnRightClickRelease(const struct FInputActionInstance& Instance);

	UFUNCTION()
	virtual void OnLeftClickRelease(const struct FInputActionInstance& Instance);

	UFUNCTION()
	virtual void OnScroll(const struct FInputActionInstance& Instance);

	TObjectPtr<AGS_Character> OwnerCharacter;

	bool bCtrlHeld = false;

	bool bWasCtrlHeldWhenLeftClicked = false;

private:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> IA_RightClick;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_LeftClick;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ModifierCtrl;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> SkillMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Scroll;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 MappingPriority = 1;
};
