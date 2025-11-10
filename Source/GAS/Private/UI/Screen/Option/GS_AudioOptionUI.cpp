// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Option/GS_AudioOptionUI.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "System/GS_GameInstance.h"
#include "Sound/GS_AudioManager.h"
#include "Kismet/GameplayStatics.h"

void UGS_AudioOptionUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기화 플래그를 true로 시작 (슬라이더 바인딩 전에 설정)
	bIsInitializing = true;
	CachedGameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(this));

	// 위젯 검색 최적화: 한 번의 순회로 모든 필요한 위젯 찾기
	if (WidgetTree)
	{
		// 전체 위젯 트리 검색 (BGM과 SFX 슬라이더를 모두 찾기)
		TArray<UWidget*> AllWidgets;
		WidgetTree->GetAllWidgets(AllWidgets);

		// 커스텀 슬라이더 위젯들을 찾기
		TArray<UUserWidget*> CustomSliderWidgets;
		for (UWidget* Widget : AllWidgets)
		{
			// 커스텀 슬라이더 위젯 검색
			if (Widget->GetClass()->GetName() == TEXT("HQUI_Button_Settings_Slider_C"))
			{
				if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
				{
					CustomSliderWidgets.Add(UserWidget);
				}
			}
		}

		// 첫 번째 슬라이더를 BGM, 두 번째를 SFX로 할당
		if (CustomSliderWidgets.Num() >= 1)
		{
			// BGM 슬라이더 (첫 번째)
			if (UUserWidget* BGMWidget = CustomSliderWidgets[0])
			{
				if (BGMWidget->WidgetTree)
				{
					TArray<UWidget*> ChildWidgets;
					BGMWidget->WidgetTree->GetAllWidgets(ChildWidgets);

					for (UWidget* ChildWidget : ChildWidgets)
					{
						if (!BGMVolumeSlider)
						{
							BGMVolumeSlider = Cast<USlider>(ChildWidget);
						}

						if (!BGMVolumeText && ChildWidget->GetName() == TEXT("SliderText"))
						{
							BGMVolumeText = Cast<UTextBlock>(ChildWidget);
						}

						if (BGMVolumeSlider && BGMVolumeText)
						{
							//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI] BGM Slider and Text found"));
							break;
						}
					}
				}
			}
		}

		if (CustomSliderWidgets.Num() >= 2)
		{
			// SFX 슬라이더 (두 번째)
			if (UUserWidget* SFXWidget = CustomSliderWidgets[1])
			{
				if (SFXWidget->WidgetTree)
				{
					TArray<UWidget*> ChildWidgets;
					SFXWidget->WidgetTree->GetAllWidgets(ChildWidgets);

					for (UWidget* ChildWidget : ChildWidgets)
					{
						if (!SFXVolumeSlider)
						{
							SFXVolumeSlider = Cast<USlider>(ChildWidget);
						}

						if (!SFXVolumeText && ChildWidget->GetName() == TEXT("SliderText"))
						{
							SFXVolumeText = Cast<UTextBlock>(ChildWidget);
						}

						if (SFXVolumeSlider && SFXVolumeText)
						{
							//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI] SFX Slider and Text found"));
							break;
						}
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AudioOptionUI] Only %d slider widget(s) found, need 2 for BGM and SFX"), CustomSliderWidgets.Num());
		}
	}

	// BGM 볼륨 슬라이더 초기화
	if (BGMVolumeSlider)
	{
		// 이벤트 바인딩 (InitializeValues 호출 전에 수행)
		BGMVolumeSlider->OnValueChanged.Clear();
		BGMVolumeSlider->OnValueChanged.AddDynamic(this, &UGS_AudioOptionUI::OnBGMVolumeChanged);
		//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI] BGM Slider bound to OnBGMVolumeChanged"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioOptionUI] BGM 볼륨 슬라이더를 찾을 수 없습니다. 블루프린트에서 직접 바인딩해야 합니다."));
	}

	// SFX 볼륨 슬라이더 초기화
	if (SFXVolumeSlider)
	{
		// 이벤트 바인딩 (InitializeValues 호출 전에 수행)
		SFXVolumeSlider->OnValueChanged.Clear();
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UGS_AudioOptionUI::OnSFXVolumeChanged);
		//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI] SFX Slider bound to OnSFXVolumeChanged"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioOptionUI] SFX 볼륨 슬라이더를 찾을 수 없습니다. 블루프린트에서 직접 바인딩해야 합니다."));
	}

	// 저장된 값으로 슬라이더 초기화
	InitializeValues();
}

void UGS_AudioOptionUI::NativeDestruct()
{
	// 캐시 정리
	CachedGameInstance.Reset();

	Super::NativeDestruct();
}

void UGS_AudioOptionUI::InitializeValues()
{
	// 초기화 중 플래그 설정 (OnBGMVolumeChanged 이벤트 무시용)
	bIsInitializing = true;

	// 캐시된 GameInstance 사용 (없으면 다시 캐싱 시도)
	if (!CachedGameInstance.IsValid())
	{
		CachedGameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(this));
	}

	if (CachedGameInstance.IsValid())
	{
		// BGM 볼륨 초기화
		const float CurrentBGMVolume = CachedGameInstance->GetBGMVolume();
		TempBGMVolume = CurrentBGMVolume;
		const float BGMSliderValue = CurrentBGMVolume * 100.0f;

		//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI::InitializeValues] BGM Volume: %.2f (Slider: %.0f)"), CurrentBGMVolume, BGMSliderValue);

		if (BGMVolumeSlider)
		{
			BGMVolumeSlider->SetValue(BGMSliderValue);
		}

		if (BGMVolumeText)
		{
			BGMVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), BGMSliderValue)));
		}

		// SFX 볼륨 초기화
		const float CurrentSFXVolume = CachedGameInstance->GetSFXVolume();
		TempSFXVolume = CurrentSFXVolume;
		const float SFXSliderValue = CurrentSFXVolume * 100.0f;

		//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI::InitializeValues] SFX Volume: %.2f (Slider: %.0f)"), CurrentSFXVolume, SFXSliderValue);

		if (SFXVolumeSlider)
		{
			SFXVolumeSlider->SetValue(SFXSliderValue);
		}

		if (SFXVolumeText)
		{
			SFXVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), SFXSliderValue)));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioOptionUI::InitializeValues] GameInstance is invalid!"));
	}

	// 초기화 완료, 플래그 해제
	bIsInitializing = false;
}

void UGS_AudioOptionUI::OnBGMVolumeChanged(float Value)
{
	// 초기화 중에는 이벤트 무시 (InitializeValues에서 SetValue 호출 시)
	if (bIsInitializing)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioOptionUI::OnBGMVolumeChanged] IGNORED - bIsInitializing=true, Value=%.0f"), Value);
		return;
	}

	// 슬라이더 값은 0-100 범위이므로 0.0-1.0 범위로 변환
	const float NormalizedVolume = FMath::Clamp(Value / 100.0f, 0.0f, 1.0f);

	//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI::OnBGMVolumeChanged] Value: %.0f -> Normalized: %.2f"), Value, NormalizedVolume);

	// 임시 볼륨 값 업데이트
	TempBGMVolume = NormalizedVolume;

	// SliderText 업데이트
	if (BGMVolumeText)
	{
		BGMVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Value)));
	}

	// 캐시된 GameInstance 사용 (반복 캐스팅 방지)
	if (!CachedGameInstance.IsValid())
	{
		CachedGameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(this));
	}

	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->SetBGMVolume(NormalizedVolume);
	}
}

void UGS_AudioOptionUI::OnSFXVolumeChanged(float Value)
{
	// 초기화 중에는 이벤트 무시
	if (bIsInitializing)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioOptionUI::OnSFXVolumeChanged] IGNORED - bIsInitializing=true, Value=%.0f"), Value);
		return;
	}

	// 슬라이더 값은 0-100 범위이므로 0.0-1.0 범위로 변환
	const float NormalizedVolume = FMath::Clamp(Value / 100.0f, 0.0f, 1.0f);

	//UE_LOG(LogTemp, Log, TEXT("[AudioOptionUI::OnSFXVolumeChanged] Value: %.0f -> Normalized: %.2f"), Value, NormalizedVolume);

	// 임시 볼륨 값 업데이트
	TempSFXVolume = NormalizedVolume;

	// SliderText 업데이트
	if (SFXVolumeText)
	{
		SFXVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Value)));
	}

	// GameInstance를 통해 SFX 볼륨 설정
	if (!CachedGameInstance.IsValid())
	{
		CachedGameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(this));
	}

	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->SetSFXVolume(NormalizedVolume);
	}
}

void UGS_AudioOptionUI::SetBGMVolumeStatic(UObject* WorldContextObject, float Volume)
{
	if (!WorldContextObject)
	{
		return;
	}

	// 슬라이더 값은 0-100 범위이므로 0.0-1.0 범위로 변환
	const float NormalizedVolume = FMath::Clamp(Volume / 100.0f, 0.0f, 1.0f);

	// GameInstance를 통해 BGM 볼륨 설정
	if (UGS_GameInstance* GameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		GameInstance->SetBGMVolume(NormalizedVolume);
	}
}

void UGS_AudioOptionUI::SetSFXVolumeStatic(UObject* WorldContextObject, float Volume)
{
	if (!WorldContextObject)
	{
		return;
	}

	// 슬라이더 값은 0-100 범위이므로 0.0-1.0 범위로 변환
	const float NormalizedVolume = FMath::Clamp(Volume / 100.0f, 0.0f, 1.0f);

	// GameInstance를 통해 SFX 볼륨 설정
	if (UGS_GameInstance* GameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		GameInstance->SetSFXVolume(NormalizedVolume);
	}
}

void UGS_AudioOptionUI::ApplySettings_Implementation()
{
	// 즉시 저장 방식이므로 OnBGMVolumeChanged/OnSFXVolumeChanged에서 이미 저장됨
	// 저장 버튼 호출 시를 대비해 한 번 더 호출하지만, GameInstance에서 중복 저장 방지됨
	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->SetBGMVolume(TempBGMVolume);
		CachedGameInstance->SetSFXVolume(TempSFXVolume);
	}
}

void UGS_AudioOptionUI::CancelSettings_Implementation()
{
	// 즉시 저장 방식이므로 이미 변경사항이 저장되어 있음
	// InitializeValues를 호출하면 저장된 값(현재 값)을 다시 로드함
	InitializeValues();
}

