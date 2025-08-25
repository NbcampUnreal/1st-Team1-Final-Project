// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RTS/GS_RTSCamera.h"
#include "System/GameMode/GS_InGameGM.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/Box2D.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

// Sets default values
AGS_RTSCamera::AGS_RTSCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_RTSCamera::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGS_RTSCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UCameraComponent* AGS_RTSCamera::GetCameraComponent() const
{
	// 기존 블루프린트에서 추가된 카메라 컴포넌트 찾기
	return FindComponentByClass<UCameraComponent>();
}

USpringArmComponent* AGS_RTSCamera::GetSpringArmComponent() const
{
	// 기존 블루프린트에서 추가된 스프링 암 컴포넌트 찾기
	return FindComponentByClass<USpringArmComponent>();
}

FBox2D AGS_RTSCamera::GetSimpleViewBounds() const
{
	// 기존 컴포넌트들을 활용한 간단한 뷰포트 경계 계산
	UCameraComponent* CameraComp = GetCameraComponent();
	USpringArmComponent* SpringArmComp = GetSpringArmComponent();
	
	if (!CameraComp || !SpringArmComp)
	{
		// 컴포넌트가 없으면 기본값 반환
		FVector CameraLocation = GetActorLocation();
		return FBox2D(
			FVector2D(CameraLocation.X - 1000.0f, CameraLocation.Y - 1000.0f),
			FVector2D(CameraLocation.X + 1000.0f, CameraLocation.Y + 1000.0f)
		);
	}
	
	// 카메라 위치와 스프링 암 정보를 활용한 계산
	FVector CameraLocation = CameraComp->GetComponentLocation();
	float ArmLength = SpringArmComp->TargetArmLength;
	
	// 카메라 높이와 각도를 고려한 간단한 계산
	float ViewDistance = ArmLength * 1.5f; // 스프링 암 길이의 1.5배
	
	return FBox2D(
		FVector2D(CameraLocation.X - ViewDistance, CameraLocation.Y - ViewDistance),
		FVector2D(CameraLocation.X + ViewDistance, CameraLocation.Y + ViewDistance)
	);
}

