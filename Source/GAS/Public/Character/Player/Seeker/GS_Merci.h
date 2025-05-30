// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Character/Interface/GS_AttackInterface.h"
#include "AkGameplayTypes.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "GS_Merci.generated.h"

class AGS_SeekerMerciArrow;
class UAkComponent;
class UNiagaraSystem;

UCLASS()
class GAS_API AGS_Merci : public AGS_Seeker, public IGS_AttackInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Merci();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Attack Interface
	virtual void LeftClickPressed_Implementation() override;
	virtual void LeftClickRelease_Implementation() override;

	// 화살 발사 VFX
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayArrowShotVFX(FVector Location, FRotator Rotation, int32 NumArrows);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> WidgetCrosshairClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	UUserWidget* WidgetCrosshair;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* Quiver;

	UFUNCTION(BlueprintCallable)
	void DrawBow(UAnimMontage* DrawMontage);

	UFUNCTION(BlueprintCallable)
	void ReleaseArrow(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg = 0.0f, int32 NumArrows = 1);

	UFUNCTION(Server, Reliable)
	void Server_DrawBow(UAnimMontage* DrawMontage);

	UFUNCTION(Server, Reliable)
	void Server_ReleaseArrow(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg = 0.0f, int32 NumArrows = 1);

	UFUNCTION(Server, Reliable)
	void Server_FireArrow(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg = 0.0f, int32 NumArrows = 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AGS_SeekerMerciArrow> NormalArrowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AGS_SeekerMerciArrow> SmokeArrowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* ComboSkillDrawMontage;

	UFUNCTION(Server, Reliable)
	void Server_NotifyDrawMontageEnded();

	void OnDrawMontageEnded();
	
	bool GetIsFullyDrawn() { return bIsFullyDrawn; }
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DrawDebugLine(FVector Start, FVector End, FColor Color = FColor::Green);

	UFUNCTION(Server, Reliable)
	void Server_ChangeArrowType(int32 Direction);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 화살 발사 VFX 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* ArrowShotVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* MultiShotVFX;

	// VFX 위치 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack", meta = (AllowPrivateAccess = "true"))
	FVector ArrowShotVFXOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack", meta = (AllowPrivateAccess = "true"))
	FVector MultiShotVFXOffset = FVector::ZeroVector;


	// 타임라인 관련
	FTimeline ZoomTimeline;

	UPROPERTY(EditAnywhere)
	UCurveFloat* ZoomCurve;

	UFUNCTION()
	void UpdateZoom(float Alpha);

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
    class UAkAudioEvent* ArrowShotSound_C;

	// 활 관련 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Bow")
	UAkAudioEvent* BowPullSound; // 활 당길 때(클릭)

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Bow")
	UAkAudioEvent* BowReleaseSound; // 활 놓을 때(릴리즈)

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Bow")
	UAkAudioEvent* ArrowShotSound; // 활 놓을 때(릴리즈)

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	// [화살 관리]
	
private:
	bool bWidgetVisibility = false;
	USkeletalMeshComponent* Mesh;

	void PlayDrawMontage(UAnimMontage* DrawMontage);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopDrawMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDrawMontage(UAnimMontage* Montage);

	UFUNCTION(Client, Reliable)
	void Client_SetWidgetVisibility(bool bVisible);

	UFUNCTION(Client, Reliable)
	void Client_StartZoom();

	UFUNCTION(Client, Reliable)
	void Client_StopZoom();

	UFUNCTION(Client, Reliable)
	void Client_PlaySound(UAkComponent* SoundComp);

	bool bIsFullyDrawn = false;

	EArrowType CurrentArrowType;

	// [화살 관리]
	int32 MaxAxeArrows = 5;

	int32 MaxChildArrows = 3;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Arrow")
	int32 CurrentAxeArrows;
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Arrow")
	int32 CurrentChildArrows;

	FTimerHandle AxeArrowRegenTimer;
	FTimerHandle ChildArrowRegenTimer;

	float RegenInterval = 5.0f; // 5초마다 충전

	UFUNCTION()
	void RegenAxeArrow();

	UFUNCTION()
	void RegenChildArrow();
};
