#pragma once

#include "CoreMinimal.h"
#include "GS_BuildManager.h"
#include "GameFramework/PlayerController.h"
#include "GS_DEController.generated.h"

struct FInputActionValue;
class UGS_DungeonEditorWidget;
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
	UInputAction* PropRotationAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ClickLMBAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ClickRMBAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ClickDeleteAction;

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
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_DungeonEditorWidget> DungeonEditorWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UGS_DungeonEditorWidget> DungeonEditorWidget;

	TObjectPtr<APawn>& GetEditorPawn() { return EditorPawn; }
	
protected:
	void BeginPlay() override;

	void CreateDEWidgets();

	UPROPERTY()
	bool Is_DEActive;

	UPROPERTY()
	TObjectPtr<APawn> EditorPawn;
	
	virtual void EnterEditorMode(AActor* SpawnPoint);
	virtual void ExitEditorMode();
	
	// 서버에 에디터 모드 진입을 요청하는 RPC
	UFUNCTION(Server, Reliable)
	void Server_RequestEnterEditorMode(AActor* SpawnPoint);
	// 서버에 에디터 모드 종료를 요청하는 RPC
	UFUNCTION(Server, Reliable)
	void Server_RequestExitEditorMode();
	// 서버가 클라이언트에게 에디터 모드 진입이 완료되었음을 알리는 RPC
	UFUNCTION(Client, Reliable)
	void Client_OnEnteredEditorMode();
	// 서버가 클라이언트에게 에디터 모드 종료가 완료되었음을 알리는 RPC
	UFUNCTION(Client, Reliable)
	void Client_OnExitedEditorMode();

	// 입력
	virtual void SetupInputComponent() override;
	
	void HandleMove(const FInputActionValue& Value);
	void HandleZoom(const FInputActionValue& Value);
	void HandlePropRotation(const FInputActionValue& Value);
	void HandleClickLMB(const FInputActionValue& Value);
	void HandleReleasedLMB(const FInputActionValue& Value);
	void HandleClickRMB(const FInputActionValue& Value);
	void HandleReleasedRMB(const FInputActionValue& Value);
	void HandleClickDelete(const FInputActionValue& Value);
};
