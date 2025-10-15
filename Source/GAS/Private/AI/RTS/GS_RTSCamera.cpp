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

bool AGS_RTSCamera::HasCameraChanged() const
{
	UCameraComponent* CameraComp = GetCameraComponent();
	USpringArmComponent* SpringArmComp = GetSpringArmComponent();

	if (!CameraComp || !SpringArmComp)
	{
		return true;
	}

	FVector CurrentLocation = CameraComp->GetComponentLocation();
	FRotator CurrentRotation = CameraComp->GetComponentRotation();
	float CurrentArmLength = SpringArmComp->TargetArmLength;

	// 위치/회전/줌이 변경되었는지 체크 (오차 허용)
	const float LocationTolerance = 1.0f; // 1cm
	const float RotationTolerance = 0.1f; // 0.1도
	const float ArmLengthTolerance = 1.0f; // 1cm

	bool bLocationChanged = !CurrentLocation.Equals(LastCameraLocation, LocationTolerance);
	bool bRotationChanged = !CurrentRotation.Equals(LastCameraRotation, RotationTolerance);
	bool bArmLengthChanged = FMath::Abs(CurrentArmLength - LastArmLength) > ArmLengthTolerance;

	return bLocationChanged || bRotationChanged || bArmLengthChanged;
}

FBox2D AGS_RTSCamera::GetSimpleViewBounds() const
{
	// 캐시가 유효하고 카메라가 변경되지 않았으면 캐시 반환
	if (bViewBoundsCacheValid && !HasCameraChanged())
	{
		return CachedViewBounds;
	}

	// 카메라 컴포넌트 가져오기
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

	// 실제 카메라 투영을 고려한 정확한 계산
	FVector CameraLocation = CameraComp->GetComponentLocation();
	FRotator CameraRotation = CameraComp->GetComponentRotation();

	// FOV와 종횡비 가져오기
	float FOV = CameraComp->FieldOfView;
	float AspectRatio = CameraComp->AspectRatio > 0.0f ? CameraComp->AspectRatio : 16.0f / 9.0f;

	// 카메라 높이 (Z축)
	float CameraHeight = CameraLocation.Z;

	// 카메라 피치 각도 (아래를 보는 각도)
	float PitchRadians = FMath::DegreesToRadians(FMath::Abs(CameraRotation.Pitch));

	// 지면까지의 거리 계산 (삼각함수 사용)
	float GroundDistance = CameraHeight / FMath::Tan(PitchRadians);

	// 수평 FOV 계산 (세로 FOV를 종횡비로 변환)
	float HorizontalFOV = 2.0f * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(FOV) * 0.5f) * AspectRatio);

	// 화면 중앙에서 좌우 끝까지의 거리
	float HalfWidth = GroundDistance * FMath::Tan(HorizontalFOV * 0.5f);

	// 화면 중앙에서 상하 끝까지의 거리
	float HalfHeight = GroundDistance * FMath::Tan(FMath::DegreesToRadians(FOV) * 0.5f);

	// 카메라 회전(Yaw)을 고려한 방향 벡터
	FVector ForwardVector = CameraRotation.Vector();
	FVector RightVector = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);

	// 화면 중앙 지점 (지면에 투영)
	FVector GroundCenter = CameraLocation + ForwardVector * GroundDistance;
	GroundCenter.Z = 0.0f; // 지면으로 투영

	// 2D 경계 계산 (회전 고려)
	FVector2D Center2D(GroundCenter.X, GroundCenter.Y);
	FVector2D Right2D(RightVector.X, RightVector.Y);
	Right2D.Normalize();
	FVector2D Forward2D(ForwardVector.X, ForwardVector.Y);
	Forward2D.Normalize();

	// 4개 코너 계산
	FVector2D TopLeft = Center2D + Forward2D * HalfHeight - Right2D * HalfWidth;
	FVector2D TopRight = Center2D + Forward2D * HalfHeight + Right2D * HalfWidth;
	FVector2D BottomLeft = Center2D - Forward2D * HalfHeight - Right2D * HalfWidth;
	FVector2D BottomRight = Center2D - Forward2D * HalfHeight + Right2D * HalfWidth;

	// AABB (Axis-Aligned Bounding Box) 계산
	float MinX = FMath::Min(FMath::Min(TopLeft.X, TopRight.X), FMath::Min(BottomLeft.X, BottomRight.X));
	float MaxX = FMath::Max(FMath::Max(TopLeft.X, TopRight.X), FMath::Max(BottomLeft.X, BottomRight.X));
	float MinY = FMath::Min(FMath::Min(TopLeft.Y, TopRight.Y), FMath::Min(BottomLeft.Y, BottomRight.Y));
	float MaxY = FMath::Max(FMath::Max(TopLeft.Y, TopRight.Y), FMath::Max(BottomLeft.Y, BottomRight.Y));

	FBox2D ResultBounds = FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));

	// 캐시 업데이트
	CachedViewBounds = ResultBounds;
	LastCameraLocation = CameraLocation;
	LastCameraRotation = CameraRotation;
	LastArmLength = SpringArmComp->TargetArmLength;
	bViewBoundsCacheValid = true;

	return ResultBounds;
}

