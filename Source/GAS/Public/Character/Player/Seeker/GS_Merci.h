// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Character/Interface/GS_AttackInterface.h"
#include "AkGameplayTypes.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "Animation/Character/E_SeekerAnim.h"
#include "GS_Merci.generated.h"

class AGS_SeekerMerciArrow;
class UAkComponent;
class UGS_ArrowTypeWidget;
class UNiagaraSystem;
class UGS_CrossHairImage;

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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayArrowShotSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayArrowEmptySound();

	// getter
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	int32 GetMaxAxeArrows();

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	int32 GetMaxChildArrows();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> WidgetCrosshairClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	UGS_CrossHairImage* WidgetCrosshair;

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

	void SetArrowTypeWidget(UGS_ArrowTypeWidget* Widget) { ArrowTypeWidget = Widget; }

	// Auto Aiming
	void SetAutoAimTarget(AActor* Target);

	// Camera Control
	UFUNCTION(Client, Reliable)
	void Client_StartZoom();

	UFUNCTION(Client, Reliable)
	void Client_StopZoom();

	//Crosshair
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairWidget(UGS_CrossHairImage* InCrosshairWidget);

	UFUNCTION(Client, Reliable)
	void Client_ShowCrosshairHitFeedback();
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

	// 활 관련 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Bow")
	UAkAudioEvent* BowPullSound; // 활 당길 때(클릭)

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Bow")
	UAkAudioEvent* BowReleaseSound; // 활 놓을 때(릴리즈)

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Bow")
	UAkAudioEvent* ArrowShotSound; // 활 놓을 때(릴리즈)

	// 화살 타입 변경 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Arrow")
	UAkAudioEvent* ArrowTypeChangeSound; // 화살 타입 변경할 때

	// 화살 부족 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Arrow")
	UAkAudioEvent* ArrowEmptySound; // 화살이 없을 때

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	// [화살 관리]
	
private:
	UGS_ArrowTypeWidget* ArrowTypeWidget;

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
	void Client_PlaySound(UAkComponent* SoundComp);

	bool bIsFullyDrawn = false;


	// [화살 관리]
	int32 MaxAxeArrows = 5;
	int32 MaxChildArrows = 3;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentArrowType)
	EArrowType CurrentArrowType;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAxeArrows)
	int32 CurrentAxeArrows;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentChildArrows)
	int32 CurrentChildArrows;

	UFUNCTION()
	void OnRep_CurrentArrowType();

	UFUNCTION()
	void OnRep_CurrentAxeArrows();

	UFUNCTION()
	void OnRep_CurrentChildArrows();

	FTimerHandle AxeArrowRegenTimer;
	FTimerHandle ChildArrowRegenTimer;

	float RegenInterval = 5.0f; // 5초마다 충전

	UFUNCTION()
	void RegenAxeArrow();

	UFUNCTION()
	void RegenChildArrow();

	// Auto Aiming
	UPROPERTY(ReplicatedUsing = OnRep_AutoAimTarget)
	AActor* AutoAimTarget;

	UFUNCTION()
	void OnRep_AutoAimTarget();

	UFUNCTION(NetMulticast, Reliable)
	void Client_DrawDebugSphere(FVector Loc, float Radius, FColor Color, float Duration);
};
