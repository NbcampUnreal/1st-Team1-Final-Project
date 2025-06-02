// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Iris/ReplicationSystem/ReplicationSystemTypes.h"
#include "GS_Chan.generated.h"

class AGS_WeaponShield;
class AGS_WeaponAxe;
class UGS_ChanAimingSkillBar;

UCLASS()
class GAS_API AGS_Chan : public AGS_Seeker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Chan();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnComboAttack();
	
	// Combo Attack Montage
	void ComboInputOpen();
	void ComboInputClose();
	void EndMontage();

	void ComboEnd();

	// Move Skill
	void OnMoveSkill();
	void OffMoveSkill();

	// Aim Skill
	void OnReadyAimSkill();
	void OnJumpAttackSkill();
	void OffJumpAttackSkill();
	void ToIdle();
	
	
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* ComboAnimMontage;

	int32 CurrentComboIndex;
	bool CanAcceptComboInput = true;
	bool bNextCombo = false;

	UFUNCTION(Server, Reliable)
	void ServerAttackMontage();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayComboSection();

	// AnimInstnace Slot State Value 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsUpperBodySlot(bool bUpperBodySlot);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsFullBodySlot(bool bFullBodySlot);

	// Control State Value
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMoveControlValue(bool bMoveForward, bool bMoveRight);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetLookControlValue(bool bLookUp, bool bLookRight);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMustTurnInPlace(bool MustTurn);

	// Weapon
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class AGS_WeaponShield> WeaponShieldClass;

	UPROPERTY(Replicated)
	AGS_WeaponShield* WeaponShield;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponShieldName = "Shield";
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class AGS_WeaponAxe> WeaponAxeClass;

	UPROPERTY(Replicated)
	AGS_WeaponAxe* WeaponAxe;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponAxeName = "Axe";

	// ===============
	// 전용 공격 사운드
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* AxeSwingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Voice")
	UAkAudioEvent* AttackVoiceSound;

	template <typename T>
	void SpawnAndAttachWeapon(TSubclassOf<T> WeaponClass, FName SocketName, T*& OutWeapon);

	// [Widget]
	void SetChanAimingSkillBarWidget(UGS_ChanAimingSkillBar* Widget) { ChanAimingSkillBarWidget = Widget; }

	UFUNCTION(Client, Reliable)
	void Client_UpdateChanAimingSkillBar(float Stamina);

	UFUNCTION(Client, Reliable)
	void Client_ChanAimingSkillBar(bool bShow);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UGS_ChanAimingSkillBar* ChanAimingSkillBarWidget;
};

template <typename T>
void AGS_Chan::SpawnAndAttachWeapon(TSubclassOf<T> WeaponClass, FName SocketName, T*& OutWeapon)
{
	if (!WeaponClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	T* SpawnWeapon = World->SpawnActor<T>(WeaponClass);
	if (!SpawnWeapon)
	{
		return;
	}

	SpawnWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		SocketName);

	SpawnWeapon->SetOwningCharacter(this);
	OutWeapon = SpawnWeapon;
}
