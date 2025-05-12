#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_DEController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class GAS_API AGS_DEController : public APlayerController
{
	GENERATED_BODY()

public:
	AGS_DEController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ZoomAction;

	// Zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float ZoomSpeed;
	float GetZoomSpeed() { return ZoomSpeed; }

protected:
	void BeginPlay() override;

private:
	TObjectPtr<APawn> EditorPawn;
};
