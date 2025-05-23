#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	
	void InitStat(FName RowName);

	void UpdateStat();

	float CalculateDamage(AGS_Character* InDamageCauser, AGS_Character* InDamagedCharacter, float InSkillCoefficient = 1.f, float SlopeCoefficient = 1.f);

	//getter
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	FORCEINLINE float GetCurrentHealth()const { return CurrentHealth; }
	FORCEINLINE float GetAttackPower()const { return AttackPower; }
	FORCEINLINE float GetDefense()const { return Defense; }
	FORCEINLINE float GetAgility()const { return Agility; }
	FORCEINLINE float GetAttackSpeed()const { return AttackSpeed; }

	//setter
	void SetCurrentHealth(float InHealth);
	void SetMaxHealth(float InMaxHealth);
	void SetAttackPower(float InAttackPower);
	void SetDefense(float InDefense);
	void SetAgility(float InAgility);
	void SetAttackSpeed(float InAttackSpeed);
	
	//rpc
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCPlayTakeDamageMontage();

	//OnRep Function
	UFUNCTION()
	void OnRep_CurrentHealth();

protected:

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	float AttackPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	float Defense;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	float Agility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	float AttackSpeed;

	UFUNCTION()
	void OnDamageMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	float CharacterWalkSpeed;
};
