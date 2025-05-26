// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "BehaviorTree/BlackboardData.h"
#include "AkGameplayStatics.h"
#include "MonsterDataAsset.h"
#include "GS_Monster.generated.h"

class UGS_MonsterAnimInstance;
/**
 * 
 */
UCLASS()
class GAS_API AGS_Monster : public AGS_Character
{
	GENERATED_BODY()

public:
	AGS_Monster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AvoidanceRadius = 200.0f;
	
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

	UPROPERTY(Replicated, BlueprintReadOnly, Category="RTS")
	bool bCommandLocked = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="RTS")
	bool bSelectionLocked = false;

	FORCEINLINE bool IsCommandable() const { return !bCommandLocked; }
	FORCEINLINE bool IsSelectable() const { return !bSelectionLocked; }
	
	//void SetSelected(bool bIsSelected);
	void SetSelected(bool bIsSelected, bool bPlaySound = true);

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
	
protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY()
	TObjectPtr<UGS_MonsterAnimInstance> MonsterAnim;
	
	UPROPERTY(VisibleAnywhere)
	UDecalComponent* SelectionDecal;

	UPROPERTY(VisibleAnywhere)
	UAkComponent* AkComponent;
	
};

