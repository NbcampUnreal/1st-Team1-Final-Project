// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "BehaviorTree/BlackboardData.h"
#include "AkGameplayStatics.h"
#include "MonsterDataAsset.h"
#include "Sound/GS_MonsterAudioComponent.h"
#include "GS_Monster.generated.h"

class UWidgetComponent;
class UGS_MonsterSkillComp;
class UGS_MonsterAnimInstance;
class UGS_DebuffVFXComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDead, AGS_Monster*, DeadUnit);

UCLASS()
class GAS_API AGS_Monster : public AGS_Character
{
	GENERATED_BODY()

public:
	AGS_Monster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AvoidanceRadius;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="RTS")
	bool bCommandLocked;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="RTS")
	bool bSelectionLocked;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BTAsset;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	UBlackboardData* BBAsset;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* ClickSoundEvent;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* MoveSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	UMonsterDataAsset* MonsterData;

	UPROPERTY(BlueprintAssignable, Category="Dead")
	FOnMonsterDead OnMonsterDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UWidgetComponent> SkillCooldownWidgetComp;
	
	// 전투 음악 관련 (BGM 이벤트만 유지, 트리거는 제거)
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAkAudioEvent* CombatMusicEvent;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAkAudioEvent* CombatMusicStopEvent;
	
	// 몬스터 오디오 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	class UGS_MonsterAudioComponent* MonsterAudioComponent;
	
	// 디버프 VFX 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UGS_DebuffVFXComponent* DebuffVFXComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

	FORCEINLINE bool IsCommandable() const { return !bCommandLocked; }
	FORCEINLINE bool IsSelectable() const { return !bSelectionLocked; }
	
	void SetSelected(bool bIsSelected, bool bPlaySound = true);
	
	virtual void SetCanUseSkill(bool bCanUse) override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void Attack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage();

	UFUNCTION(BlueprintCallable, Category="Data")
	UTexture2D* GetPortrait() const { return MonsterData ? MonsterData->Portrait : nullptr; }
	
	UFUNCTION(BlueprintCallable, Category="Data")
	FText GetMonsterName() const { return MonsterData ? MonsterData->MonsterName : FText::GetEmpty(); }

	UFUNCTION(BlueprintCallable, Category="Data")
	FText GetDescription() const { return MonsterData ? MonsterData->Description : FText::GetEmpty(); }

	UFUNCTION(BlueprintCallable, Category="Data")
	FText GetTypeName() const { return MonsterData ? MonsterData->TypeName : FText::GetEmpty(); }

	FORCEINLINE UGS_MonsterSkillComp* GetMonsterSkillComp() const { return MonsterSkillComp; }

	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void UseSkill(); 
	
protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UGS_MonsterSkillComp> MonsterSkillComp;
	
	UPROPERTY()
	TObjectPtr<UGS_MonsterAnimInstance> MonsterAnim;
	
	UPROPERTY(VisibleAnywhere)
	UDecalComponent* SelectionDecal;

	UPROPERTY(VisibleAnywhere)
	UAkComponent* AkComponent;

	void HandleDelayedDestroy();
	virtual void OnDeath() override;

	UFUNCTION()
	void HandleSkillCooldownChanged(float InCurrentCoolTime, float InMaxCoolTime);

private:
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDecalMaterial;

	bool bIsRTSSelected = false;
	bool bIsRTSHovered = false;
    
	void UpdateDecalColor();
	void SetHovered(bool bIsHovered);
}; 