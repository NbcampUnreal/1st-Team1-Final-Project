// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Character/Interface/GS_AttackInterface.h"
#include "AkGameplayTypes.h"
#include "GS_Merci.generated.h"

class AGS_SeekerMerciArrow;
class UAkComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	UUserWidget* WidgetCrosshair;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* Quiver;

	UFUNCTION(BlueprintCallable)
	void LeftClickPressedAttack(UAnimMontage* DrawMontage);

	UFUNCTION(BlueprintCallable)
	void LeftClickReleaseAttack(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass);

	UFUNCTION(Server, Reliable)
	void Server_LeftClickPressedAttack(UAnimMontage* DrawMontage);

	UFUNCTION(Server, Reliable)
	void Server_LeftClickReleaseAttack(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass);

	void FireArrow(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 타임라인 관련
	FTimeline ZoomTimeline;

	UPROPERTY(EditAnywhere)
	UCurveFloat* ZoomCurve;

	UFUNCTION()
	void UpdateZoom(float Alpha);

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class UAkAudioEvent* ArrowShotSound_C;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* ShotSoundComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* PullSoundComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* ReleaseSoundComp;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* BowPullEvent;

	/*UFUNCTION()
	void PlayBowPullSound(UAkComponent* AkComp);*/

private:
	bool bWidgetVisibility = false;
	USkeletalMeshComponent* Mesh;

	void OnDrawMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void PlayDrawMontage(UAnimMontage* DrawMontage);

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
};
