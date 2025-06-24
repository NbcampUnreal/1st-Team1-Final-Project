#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_StatRow.h"
#include "GS_StatComp.generated.h"

class AGS_Character;
class UAkAudioEvent;
class UGS_StatComp;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentHPChangedDelegate, UGS_StatComp*);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAS_API UGS_StatComp : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnCurrentHPChangedDelegate OnCurrentHPChanged;

	UGS_StatComp();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat")
	TObjectPtr<UDataTable> StatDataTable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UAnimMontage*> TakeDamageMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* HitSoundEvent;
	
	// 히트 사운드 쿨다운 시간
	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitSoundCooldownTime = 1.75f;
	
	UPROPERTY(Replicated)
	bool bIsInvincible = false;
	
	// 쿨다운 체크 함수
	bool CanPlayHitSound() const;

	void InitStat(FName RowName);

	//[Change Stats when use buff skills]
	void ChangeStat(const FGS_StatRow& InChangeStat);
	void ResetStat(const FGS_StatRow& InChangeStat);
	
	UFUNCTION(Server, Reliable)
	void UpdateStat(const FGS_StatRow& RuneStats);

	float CalculateDamage(AGS_Character* InDamageCauser, AGS_Character* InDamagedCharacter, float InSkillCoefficient = 1.f, float SlopeCoefficient = 1.f);

	//getter
	UFUNCTION(BlueprintCallable, Category = "Stats")
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	FORCEINLINE float GetCurrentHealth()const { return CurrentHealth; }
	FORCEINLINE float GetAttackPower()const { return AttackPower; }
	FORCEINLINE float GetDefense()const { return Defense; }
	FORCEINLINE float GetAgility()const { return Agility; }
	FORCEINLINE float GetAttackSpeed()const { return AttackSpeed; }

	//setter
	void SetCurrentHealth(float InHealth, bool bIsHealing);
	void SetMaxHealth(float InMaxHealth);
	void SetAttackPower(float InAttackPower);
	void SetDefense(float InDefense);
	void SetAgility(float InAgility);
	void SetAttackSpeed(float InAttackSpeed);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCPlayTakeDamageMontage();

	UFUNCTION()
	void OnRep_CurrentHealth();

	// heal system
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRPCHeal(float InHealAmount);
	
protected:
	float CharacterWalkSpeed;

private:
	//stat
	UPROPERTY(VisibleAnywhere)
	float MaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;
	UPROPERTY(EditDefaultsOnly)
	float AttackPower;
	UPROPERTY(EditDefaultsOnly)
	float Defense;
	UPROPERTY(EditDefaultsOnly)
	float Agility;
	UPROPERTY(EditDefaultsOnly)
	float AttackSpeed;
	
	// 마지막 히트 사운드 재생 시간
	float LastHitSoundTime = 0.0f;

	UFUNCTION()
	void OnDamageMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	//Enum 통일 전 임시
	UFUNCTION()
	ECharacterClass MapCharacterTypeToCharacterClass(ECharacterType CharacterType);
};
