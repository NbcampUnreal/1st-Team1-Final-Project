// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RTS/GS_RTSHUD.h"
#include "AI/RTS/GS_RTSController.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Blueprint/WidgetLayoutLibrary.h"

AGS_RTSHUD::AGS_RTSHUD()
{
	RTSController = nullptr;
	PointA = FVector2D::ZeroVector; 
	PointB = FVector2D::ZeroVector; 
	bIsDrawing = false;
	bIsMarquee = false;
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

	const float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
	PointA = FVector2D(MouseX * Scale, MouseY * Scale);

	bIsDrawing = true;	
}

void AGS_RTSHUD::StopSelection()
{
	bIsDrawing = false;

	if (!bIsMarquee || !RTSController)
	{
		return;
	}

	RTSController->ClearUnitSelection();
	for (AGS_Monster* Actor : SelectionOfActors)
	{
		RTSController->AddUnitToSelection(Actor);
	}
	
	bIsMarquee = false;
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

	const float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
	PointB = FVector2D(MouseX * Scale, MouseY * Scale);

	bIsMarquee = FVector2D::Distance(PointA, PointB) > 200.0f;
	if (!bIsMarquee)
	{
		return;
	}
	
	const float X = PointA.X;
	const float Y = PointA.Y;
	const float W = PointB.X - PointA.X;
	const float H = PointB.Y - PointA.Y;
		
	DrawRect(FLinearColor(0,1,1,0.15f), X, Y, W, H);
	
	SelectionOfActors.Empty();
	GetActorsInSelectionRectangle<AGS_Monster>(
		PointA, PointB,
		SelectionOfActors, 
		true,               
		false               
	);
}
