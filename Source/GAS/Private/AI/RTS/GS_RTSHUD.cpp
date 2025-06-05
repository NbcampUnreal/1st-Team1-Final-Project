// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RTS/GS_RTSHUD.h"
#include "AI/RTS/GS_RTSController.h"
#include "Character/Player/Monster/GS_Monster.h"

AGS_RTSHUD::AGS_RTSHUD()
{
	RTSController = nullptr;
	PointA = FVector2D::ZeroVector; 
	PointB = FVector2D::ZeroVector; 
	bIsDrawing = false;
}

void AGS_RTSHUD::BeginPlay()
{
	Super::BeginPlay();

	RTSController = Cast<AGS_RTSController>(GetOwningPlayerController());
}

void AGS_RTSHUD::StartSelection()
{
	float MouseX, MouseY;
	if (!RTSController->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	PointA = FVector2D(MouseX, MouseY);
	bIsDrawing = true;
	SelectionOfActors.Empty();
}

void AGS_RTSHUD::StopSelection()
{
	if (!bIsDrawing)
	{
		return;
	}
	
	bIsDrawing = false;

	if (!RTSController)
	{
		return;
	}

	// 단일 클릭
	FVector2D DragVec = PointB - PointA;
	if (DragVec.SizeSquared() < 16.f)
	{
		FHitResult Hit;
		if (RTSController->GetHitResultAtScreenPosition(PointA, ECC_GameTraceChannel1, true, Hit))
		{
			if (AGS_Monster* Unit = Cast<AGS_Monster>(Hit.GetActor()))
			{
				RTSController->ClearUnitSelection();
				RTSController->AddUnitToSelection(Unit);
			}
		}
		return;  
	}

	// 드래그 선택
	if (SelectionOfActors.Num() > 0)
	{
		RTSController->ClearUnitSelection();
		for (AGS_Monster* Actor : SelectionOfActors)
		{
			RTSController->AddUnitToSelection(Actor);
		}
	}	
}

void AGS_RTSHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bIsDrawing)
	{
		return;
	}

	float MouseX, MouseY;
	if (!RTSController->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	PointB = FVector2D(MouseX, MouseY);
	
	const float X = PointA.X;
	const float Y = PointA.Y;
	const float W = PointB.X - PointA.X;
	const float H = PointB.Y - PointA.Y;
	DrawRect(FLinearColor(0,1,1,0.15f), X, Y, W, H);

	SelectionOfActors.Empty();
	GetActorsInSelectionRectangle<AGS_Monster>(
		PointA, PointB,
		SelectionOfActors, 
		false,               
		false               
	);
}
