// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_SelectionInterface.h"
#include "GameFramework/HUD.h"
#include "GS_RTSHUD.generated.h"

class AGS_Monster;
/**
 * 
 */
UCLASS()
class GAS_API AGS_RTSHUD : public AHUD, public IGS_SelectionInterface
{
	GENERATED_BODY()

public:
	AGS_RTSHUD();
	
	virtual void StartSelection() override;
	virtual void StopSelection() override;

protected:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	
private:
	FVector2D PointA; // 마우스 시작 지점
	FVector2D PointB; // 마우스 마지막 지점 
	bool bIsDrawing;
	
	TArray<AGS_Monster*> SelectionOfActors;

	UPROPERTY()
	class AGS_RTSController* RTSController;
};
