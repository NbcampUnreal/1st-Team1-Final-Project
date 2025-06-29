#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Character/E_Character.h"
#include "GS_Character.generated.h"

class UGS_StatComp;
class UGS_SkillComp;
class UGS_DebuffComp;
class UGS_HitReactComp;
class UGS_HPTextWidgetComp;
class UGS_PlayerInfoWidget;
class UGS_HPText;
class UGS_HPWidget;
class AGS_Weapon;
class UDecalComponent;
class UAkAudioEvent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

USTRUCT(BlueprintType)
struct FWeaponSlot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<AGS_Weapon> WeaponClass = nullptr;
	
	UPROPERTY()
	AGS_Weapon* WeaponInstance = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FName SocketName = NAME_None;
};


UCLASS()
class GAS_API AGS_Character : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGS_Character();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnDamageStart();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCanHitReact(bool CanReact);
	
	UPROPERTY(Replicated)
	bool CanHitReact = true;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 팀 ID (0: 중립, 1: 플레이어, 2: 몬스터) */
	UPROPERTY(EditAnywhere, Category="Team")
	FGenericTeamId TeamId;

	// 죽음 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	UAkAudioEvent* DeathSoundEvent;

	//variable
	float MaxSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	TObjectPtr<UGS_HPTextWidgetComp> HPTextWidgetComp;
	
	//getter
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }
	FORCEINLINE UGS_DebuffComp* GetDebuffComp() const { return DebuffComp; }
	FORCEINLINE ECharacterType GetCharacterType() const { return CharacterType; }
	
	//serverRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCMeleeAttack(AGS_Character* InDamagedCharacter);

	//character death play ragdoll
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCCharacterDeath();
	
	UFUNCTION()
	void WatchOtherPlayer();

	UFUNCTION()
	virtual void OnDeath();
	UFUNCTION()
	void DestroyAllWeapons();
	
	//HP widget
	void SetHPTextWidget(UGS_HPText* InHPTextWidget);
	void SetHPBarWidget(UGS_HPWidget* InHPBarWidget);
	void SetPlayerInfoWidget(UGS_PlayerInfoWidget* InPlayerInfoWidget);
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsEnemy(const AGS_Character* Other) const;

	//play skill montage
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCPlaySkillMontage(UAnimMontage* SkillMontage);

	UFUNCTION(NetMulticast, Reliable)
	void MulicastRPCStopCurrentSkillMontage(UAnimMontage* CurrentSkillMontage);

	// Impact VFX 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayImpactVFX(UNiagaraSystem* VFXAsset, FVector Scale);

	UFUNCTION(BlueprintCallable)
	AGS_Weapon* GetWeaponByIndex(int32 Index) const;

	UFUNCTION(Server, Reliable)
	void Server_SetCharacterSpeed(float InRatio);

	UFUNCTION(BlueprintCallable)
	virtual void SetCanUseSkill(bool bCanUse) {}
	
	UFUNCTION()
	void SetCharacterSpeed(float InRatio);

	bool IsDead() const;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeath OnDeathDelegate;
	
	UFUNCTION(Server, Reliable)
	void Server_SetCanHitReact(bool bCanReact);
protected:
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_DebuffComp> DebuffComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_StatComp> StatComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_HitReactComp> HitReactComp;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RTS")
	TObjectPtr<UDecalComponent> SelectionDecal;

	bool bIsHovered;
	
	virtual FLinearColor GetCurrentDecalColor();
	virtual void UpdateDecal();
	virtual bool ShowDecal();
	void ShowDecalWithColor(const FLinearColor& Color);
	virtual void OnHoverBegin();
	virtual void OnHoverEnd();
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterSpeed)
	float CharacterSpeed;
	float DefaultCharacterSpeed;
	
	UPROPERTY(Replicated)
	bool bIsDead;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDecalMaterial;

	UPROPERTY()
	FTimerHandle HPWidgetRotationTimer;

	UFUNCTION()
	void OnRep_CharacterSpeed();

	void SpawnAndAttachWeapons();
	
	void SetHovered(bool bHovered);

	void UpdateHPWidgetRotation();
};

