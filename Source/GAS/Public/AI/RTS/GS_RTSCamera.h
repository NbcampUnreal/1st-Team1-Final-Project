// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/Box2D.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "GS_RTSCamera.generated.h"

UCLASS()
class GAS_API AGS_RTSCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGS_RTSCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void HideWallAndCeiling();
	
	// 기존 블루프린트 컴포넌트들에 접근하기 위한 헬퍼 함수들
	UFUNCTION(BlueprintCallable, Category = "Camera")
	UCameraComponent* GetCameraComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "Camera")
	USpringArmComponent* GetSpringArmComponent() const;
	
	// 간단한 뷰포트 경계 계산 (기존 컴포넌트 활용)
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FBox2D GetSimpleViewBounds() const;
};
