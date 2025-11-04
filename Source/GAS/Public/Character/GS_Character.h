#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Character/E_Character.h"
#include "Component/GS_HitReactComp.h"
#include "CharacterDataAsset.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_Character.generated.h"

class UGS_StatComp;
class UGS_SkillComp;
class UGS_DebuffComp;
class UGS_HitReactComp;
class UGS_CameraShakeComponent;
class UGS_HPTextWidgetComp;
class UGS_PlayerInfoWidget;
class UGS_HPText;
class UGS_HPWidget;
class AGS_Weapon;
class UDecalComponent;

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
	virtual void BeginDestroy() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnDamageStart();
	
	// HitReact
	bool CanHitReact = true;
	FTimerHandle HitReactTimerHandle;
	
	void DisableHitReact(float CooldownTime);
	void DisableHitReact(bool bAllowHitReact);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 팀 ID (0: 중립, 1: 플레이어, 2: 몬스터) */
	UPROPERTY(EditAnywhere, Category="Team")
	FGenericTeamId TeamId;

	// 죽음 사운드는 각 캐릭터 타입별 오디오 컴포넌트에서 처리됨
	// 시커: GS_SeekerAudioComponent, 가디언: GS_GuardianAudioComponent, 몬스터: GS_MonsterAudioComponent

	//variable
	float MaxSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_HitReactComp> HitReactComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_CameraShakeComponent> CameraShakeComp;

	// EditAnywhere, BlueprintReadOnly, Category = "Effects|CameraShake"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|CameraShake")
	FGS_CameraShakeInfo TakeDamageShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|CameraShake")
	FGS_CameraShakeInfo AttackSuccessShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	TObjectPtr<UGS_HPTextWidgetComp> HPTextWidgetComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	UCharacterDataAsset* CharacterData;

	UFUNCTION(BlueprintCallable, Category="Data")
	UTexture2D* GetPortrait() const { return CharacterData ? CharacterData->Portrait : nullptr; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess), Replicated)
	bool bLockRotationToController = false; // Idle 상태에도 bUseControllerRotationYaw 를 true 로 두기 위한 flag.

	UFUNCTION(BlueprintCallable, Category = "State")
	bool GetIsLockedRotationToController();

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetIsLockedRotationToController(bool InputIsRotationRoController);
	
	UFUNCTION(BlueprintCallable, Category="Data")
	FText GetMonsterName() const { return CharacterData ? CharacterData->CharacterName : FText::GetEmpty(); }

	UFUNCTION(BlueprintCallable, Category="Data")
	FText GetDescription() const { return CharacterData ? CharacterData->Description : FText::GetEmpty(); }

	UFUNCTION(BlueprintCallable, Category="Data")
	FText GetTypeName() const { return CharacterData ? CharacterData->TypeName : FText::GetEmpty(); }
	
	//getter
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }
	FORCEINLINE UGS_DebuffComp* GetDebuffComp() const { return DebuffComp; }
	FORCEINLINE ECharacterType GetCharacterType() const { return CharacterType; }
	
	//serverRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCMeleeAttack(AGS_Character* InDamagedCharacter);

	//clientRPC for camera shake
	UFUNCTION(Client, Reliable)
	void Client_PlayTakeDamageShake(APlayerController* TargetPC);

	UFUNCTION(Client, Reliable)
	void Client_PlayAttackSuccessShake(APlayerController* TargetPC);

	UFUNCTION(Client, Reliable)
	void Client_PlayAttackSuccessShakeWithInfo(APlayerController* TargetPC, const FGS_CameraShakeInfo& CustomShakeInfo);

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

	UFUNCTION(BlueprintCallable)
	AGS_Weapon* GetWeaponBySocketName(FName SocketName);

	UFUNCTION(Server, Reliable)
	void Server_SetCharacterSpeed(float InRatio);

	UFUNCTION(BlueprintCallable)
	virtual void SetCanUseSkill(bool bCanUse) {}
	
	UFUNCTION()
	void SetCharacterSpeed(float InRatio);

	bool IsDead() const;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeath OnDeathDelegate;

	// HitReact
	UFUNCTION(Server, Reliable)
	void Server_SetCanHitReact(bool bCanReact);

	UFUNCTION()
	void SetCanHitReact(bool bCanReact);

	void SetInvincible(bool bEnable);

protected:
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_DebuffComp> DebuffComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_StatComp> StatComp;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;

	UPROPERTY(Replicated)
	EWeaponHandlingState WeaponHandlingState = EWeaponHandlingState::Wielding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RTS")
	TObjectPtr<UDecalComponent> SelectionDecal;

	bool bIsHovered;
	
	virtual FLinearColor GetCurrentDecalColor();
	virtual void UpdateDecal();
	virtual bool ShowDecal();
	void ShowDecalWithColor(const FLinearColor& Color);
	virtual void OnHoverBegin();
	virtual void OnHoverEnd();

public:
	UFUNCTION()
	EWeaponHandlingState GetWeaponHandlingState();
	UFUNCTION()
	void SetWeaponHandlingState(EWeaponHandlingState InputWeaponHandlingState);
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterSpeed)
	float CharacterSpeed;
	float DefaultCharacterSpeed;
	
	UPROPERTY(Replicated)
	bool bIsDead;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDecalMaterial;
	
	UFUNCTION()
	void OnRep_CharacterSpeed();

	void SpawnAndAttachWeapons();
	
	void SetHovered(bool bHovered);

	// 무적 상태
	UPROPERTY(Replicated)
	bool bIsInvincible = false;
};

