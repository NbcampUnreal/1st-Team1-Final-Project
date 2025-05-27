#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "GS_Character.generated.h"

class UGS_StatComp;
class UGS_SkillComp;
class UGS_DebuffComp;
class UGS_HPTextWidgetComp;
class UGS_HPText;
class UGS_HPWidget;

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Ares,
	Chan,
	Merci,
	Reina,
	Drakhar,
	SmallClaw,
	NeedleFang,
	IronFang,
	ShadowFang,
	StoneClaw
};


UCLASS()
class GAS_API AGS_Character : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGS_Character();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 팀 ID (0: 중립, 1: 플레이어, 2: 몬스터) */
	UPROPERTY(EditAnywhere, Category="Team")
	FGenericTeamId TeamId;

	//variable
	float MaxSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType;

	ECharacterType GetCharacterType() const { return CharacterType; }

	//getter
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }
	FORCEINLINE UGS_SkillComp* GetSkillComp() const { return SkillComp; }
	FORCEINLINE UGS_DebuffComp* GetDebuffComp() const { return DebuffComp; }

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

	//play skill montage
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCPlaySkillMontage(UAnimMontage* SkillMontage);

	UFUNCTION(NetMulticast, Reliable)
	void MulicastRPCStopCurrentSkillMontage(UAnimMontage* CurrentSkillMontage);

	
protected:
	//component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_SkillComp> SkillComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_DebuffComp> DebuffComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_StatComp> StatComp;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	TObjectPtr<UGS_HPTextWidgetComp> HPTextWidgetComp;
};