// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_WeaponEquipable.h"
#include "Components/BoxComponent.h"
#include "GS_WeaponShield.generated.h"

/**
 * 방패 클래스
 */
UCLASS()
class GAS_API AGS_WeaponShield : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	AGS_WeaponShield();

	virtual void Tick(float DeltaTime) override;

	// 방어 기능 활성화/비활성화
	UFUNCTION(BlueprintCallable, Category = "Defense")
	void EnableBlock();

	UFUNCTION(BlueprintCallable, Category = "Defense")
	void DisableBlock();

	UFUNCTION(Server, Unreliable)
	void Server_SetBlockCollision(bool bEnable);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	// 방패 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* ShieldMesh;

	// 방어 콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defense")
	UBoxComponent* BlockBox;

	// 방어 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* BlockSound;

	// 방어 처리 함수
	UFUNCTION()
	void OnBlock(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// 방어 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayBlockSound();

private:
	// 방어 콜리전 설정
	void SetBlockCollision(bool bEnable);
};