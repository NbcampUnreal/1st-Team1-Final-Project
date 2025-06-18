#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_MainMenuPC.generated.h"

UCLASS()
class GAS_API AGS_MainMenuPC : public APlayerController
{
	GENERATED_BODY()

public:
	AGS_MainMenuPC();

	void HandleCustomGameRequest();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY()
	UUserWidget* MainMenuWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreenWidgetInstance;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenuUI();
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLoadingScreen();
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideLoadingScreen();
};
