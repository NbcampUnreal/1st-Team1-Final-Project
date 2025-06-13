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
class UGS_HPText;
class UGS_HPWidget;
class AGS_Weapon;
class UAkAudioEvent;

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
	FORCEINLINE UGS_SkillComp* GetSkillComp() const { return SkillComp; }
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
	
	//HP widget
	void SetHPTextWidget(UGS_HPText* InHPTextWidget);
	void SetHPBarWidget(UGS_HPWidget* InHPBarWidget);
	virtual FGenericTeamId GetGenericTeamId() const override;
	//void SetClassImage(UGS_HPText* InHPTextWidget);
	
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsEnemy(const AGS_Character* Other) const;

	//play skill montage
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCPlaySkillMontage(UAnimMontage* SkillMontage);

	UFUNCTION(NetMulticast, Reliable)
	void MulicastRPCStopCurrentSkillMontage(UAnimMontage* CurrentSkillMontage);

	UFUNCTION(BlueprintCallable)
	AGS_Weapon* GetWeaponByIndex(int32 Index) const;

	UFUNCTION(Server, Reliable)
	void Server_SetCharacterSpeed(float InRatio);
	
protected:
	//component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_SkillComp> SkillComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_DebuffComp> DebuffComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_StatComp> StatComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_HitReactComp> HitReactComp;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;
private:
	void SpawnAndAttachWeapons();
	void DestroyAllWeapons();

	UPROPERTY(ReplicatedUsing = OnRep_CharacterSpeed)
	float CharacterSpeed;
	float DefaultCharacterSpeed;

	UFUNCTION()
	void OnRep_CharacterSpeed();
};

