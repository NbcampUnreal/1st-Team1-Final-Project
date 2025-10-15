// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GS_SettingsApplyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UGS_SettingsApplyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 설정 적용 인터페이스
 * 옵션 UI에서 설정을 적용하거나 취소하는 기능을 정의합니다.
 */
class GAS_API IGS_SettingsApplyInterface
{
	GENERATED_BODY()

public:
	/**
	 * 설정을 적용하고 저장합니다.
	 * Save 버튼을 눌렀을 때 호출됩니다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Settings")
	void ApplySettings();

	/**
	 * 설정을 취소하고 이전 값으로 되돌립니다.
	 * Cancel/Close 버튼을 눌렀을 때 호출됩니다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Settings")
	void CancelSettings();
};

