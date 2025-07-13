// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_BossHP.generated.h"

class UVerticalBox;
class UGS_HPWidget;
class AGS_Guardian;
class AGS_Character;
class AGS_Player;
class UGS_DrakharFeverGauge;

UCLASS()
class GAS_API UGS_BossHP : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void InitGuardianHPWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowBossHP();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideBossHP();

	// 보스 전투 상태 변경 시 호출
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnBossFightStateChanged(bool bInBossFight);
	
	void SetOwningActor(AGS_Character* InOwningCharacter) { WidgetOwner = InOwningCharacter; }

	bool FindBoss();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> HPWidgetList;
	
	// 보스 HP UI 요소들
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> BossHPBar;
	
	//Guardian HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_HPWidget> HPWidgetClass;
	UPROPERTY()
	TObjectPtr<UGS_HPWidget> HPWidgetInstance;
	
	UPROPERTY()
	TObjectPtr<AGS_Guardian> Guardian;
	UPROPERTY()
	TObjectPtr<AGS_Character> WidgetOwner;

private:
	// HP 업데이트를 위한 함수
	UFUNCTION()
	void OnBossHPChanged(class UGS_StatComp* InStatComp);
	
	// 피버모드 상태 변경 감지
	UFUNCTION()
	void OnFeverModeChanged(bool bIsFeverMode);
	
	// 피버모드 상태를 확인하는 타이머 함수
	UFUNCTION()
	void CheckFeverModeStatus();
	
	// HP 바 색상 관련
	FLinearColor NormalHPBarColor;
	FLinearColor FeverHPBarColor;
	
	// 피버모드 상태 추적
	bool bLastFeverMode;
	FTimerHandle FeverModeCheckTimer;
	
	// Guardian 초기화 재시도 타이머
	FTimerHandle GuardianInitRetryTimer;
};
