#pragma once

#include "CoreMinimal.h"
#include "Character/GS_TpsController.h"
#include "GS_GuardianController.generated.h"

UCLASS()
class GAS_API AGS_GuardianController : public AGS_TpsController
{
	GENERATED_BODY()
	
public:
	AGS_GuardianController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* CtrlInputAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LeftMouseInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RightMouseInputAction;
	
	UFUNCTION()
	void CtrlInput(const FInputActionValue& InputValue);

	UFUNCTION()
	void CtrlStop(const FInputActionValue& InputValue);

	UFUNCTION()
	void LeftMouseInput(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void RightMouseInput(const FInputActionValue& InputValue);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
