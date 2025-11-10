// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Interface/GS_SettingsApplyInterface.h"
#include "GS_AudioOptionUI.generated.h"

class USlider;
class UTextBlock;
class UGS_GameInstance;

/**
 * 오디오 옵션 UI 위젯
 * BGM 볼륨 조절 슬라이더를 제공하며, 슬라이더 조절 시 즉시 저장됩니다.
 */
UCLASS()
class GAS_API UGS_AudioOptionUI : public UUserWidget, public IGS_SettingsApplyInterface
{
	GENERATED_BODY()

public:
	// BGM 볼륨 슬라이더 (커스텀 UI 위젯에서 동적으로 찾음)
	UPROPERTY()
	USlider* BGMVolumeSlider;

	// BGM 볼륨 텍스트 (슬라이더 옆에 표시되는 숫자)
	UPROPERTY()
	UTextBlock* BGMVolumeText;

	// 임시 볼륨 값 (슬라이더 조정 시 사용, 즉시 저장 방식으로 변경되어 주로 로깅용)
	UPROPERTY()
	float TempBGMVolume;

	// SFX 볼륨 슬라이더 (커스텀 UI 위젯에서 동적으로 찾음)
	UPROPERTY()
	USlider* SFXVolumeSlider;

	// SFX 볼륨 텍스트 (슬라이더 옆에 표시되는 숫자)
	UPROPERTY()
	UTextBlock* SFXVolumeText;

	// 임시 SFX 볼륨 값
	UPROPERTY()
	float TempSFXVolume;

	// 초기화 중 플래그 (InitializeValues 실행 중 이벤트 무시용)
	bool bIsInitializing;

	// GameInstance 캐싱 (반복 캐스팅 방지)
	UPROPERTY()
	TWeakObjectPtr<UGS_GameInstance> CachedGameInstance;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * GameInstance에서 현재 설정 값을 읽어와 UI와 임시 변수를 초기화합니다.
	 * 메뉴를 열 때나 취소할 때 호출됩니다.
	 */
	void InitializeValues();

public:
	/**
	 * BGM 볼륨 슬라이더 값이 변경될 때 호출되는 함수
	 * 즉시 저장 방식으로 GameInstance->SetBGMVolume을 호출하여 저장합니다.
	 * @param Value 새로운 볼륨 값 (0.0 ~ 100.0, 내부에서 0.0 ~ 1.0으로 변환)
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void OnBGMVolumeChanged(float Value);

	/**
	 * SFX 볼륨 슬라이더 값이 변경될 때 호출되는 함수
	 * 즉시 저장 방식으로 GameInstance->SetSFXVolume을 호출하여 저장합니다.
	 * @param Value 새로운 볼륨 값 (0.0 ~ 100.0, 내부에서 0.0 ~ 1.0으로 변환)
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void OnSFXVolumeChanged(float Value);

	/**
	 * 블루프린트에서 정적으로 호출 가능한 BGM 볼륨 설정 함수
	 * @param WorldContextObject 월드 컨텍스트
	 * @param Volume 새로운 볼륨 값 (0.0 ~ 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	static void SetBGMVolumeStatic(UObject* WorldContextObject, float Volume);

	/**
	 * 블루프린트에서 정적으로 호출 가능한 SFX 볼륨 설정 함수
	 * @param WorldContextObject 월드 컨텍스트
	 * @param Volume 새로운 볼륨 값 (0.0 ~ 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	static void SetSFXVolumeStatic(UObject* WorldContextObject, float Volume);

	// IGS_SettingsApplyInterface 구현
	/**
	 * 설정 저장 (Save 버튼 클릭 시)
	 * 즉시 저장 방식이므로 이미 OnBGMVolumeChanged에서 저장되었지만,
	 * 저장 버튼 호출 시를 위해 한 번 더 저장합니다 (중복이지만 안전).
	 */
	virtual void ApplySettings_Implementation() override;

	/**
	 * 설정 취소 (Cancel/Close 버튼 클릭 시)
	 * 즉시 저장 방식이므로 이미 변경사항이 저장되어 있음.
	 * InitializeValues를 호출하여 현재 저장된 값을 다시 로드합니다.
	 */
	virtual void CancelSettings_Implementation() override;
};
