#pragma once

#include "CoreMinimal.h"
#include "GS_BuildManager.h"
#include "GameFramework/PlayerController.h"
#include "GS_DEController.generated.h"

class UGS_PropWidget;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ClickLMBAction;

	// Zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float ZoomSpeed;
	float GetZoomSpeed() { return ZoomSpeed; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectRef")
	TObjectPtr<AGS_BuildManager> BuildManagerRef;
	AGS_BuildManager* GetBuildManager() { return BuildManagerRef; }

	// Widget
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_PropWidget> PropWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UGS_PropWidget> PropWidget;
	
protected:
	void BeginPlay() override;

private:
	TObjectPtr<APawn> EditorPawn;
};
