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

	// 초기화
	bIsInitializing = false;
	CachedGameInstance = Cast<UGS_GameInstance>(UGameplayStatics::GetGameInstance(this));

	// 위젯 검색 최적화: 한 번의 순회로 모든 필요한 위젯 찾기
	if (WidgetTree)
	{
		// 1. 이름으로 직접 찾기 시도
		static const TArray<FName> SliderNames = {
			FName(TEXT("+- Slider")),
			FName(TEXT("Slider")),
			FName(TEXT("SB_Slider")),
			FName(TEXT("BGM_Slider")),
			FName(TEXT("Volume_Slider")),
			FName(TEXT("Audio_Slider"))
		};

		for (const FName& SliderName : SliderNames)
		{
			BGMVolumeSlider = WidgetTree->FindWidget<USlider>(SliderName);
			if (BGMVolumeSlider)
			{
				break;
			}
		}

		// 2. 이름으로 찾지 못했으면 전체 위젯 트리 검색 (한 번만)
		if (!BGMVolumeSlider)
		{
			TArray<UWidget*> AllWidgets;
			WidgetTree->GetAllWidgets(AllWidgets);

			for (UWidget* Widget : AllWidgets)
			{
				// 슬라이더 직접 찾기
				if (!BGMVolumeSlider)
				{
					if (USlider* Slider = Cast<USlider>(Widget))
					{
						BGMVolumeSlider = Slider;
					}
				}

				// 커스텀 슬라이더 위젯 내부 검색
				if (!BGMVolumeSlider && Widget->GetClass()->GetName() == TEXT("HQUI_Button_Settings_Slider_C"))
				{
					if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
					{
						if (UserWidget->WidgetTree)
						{
							TArray<UWidget*> ChildWidgets;
							UserWidget->WidgetTree->GetAllWidgets(ChildWidgets);

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

								// 둘 다 찾았으면 종료
								if (BGMVolumeSlider && BGMVolumeText)
								{
									break;
								}
							}
						}
					}
				}

				// 슬라이더를 찾았으면 종료
				if (BGMVolumeSlider)
				{
					break;
				}
			}
		}
	}

	// BGM 볼륨 슬라이더 초기화
	if (BGMVolumeSlider)
	{
		// 이벤트 바인딩 (InitializeValues 호출 전에 수행)
		BGMVolumeSlider->OnValueChanged.Clear();
		BGMVolumeSlider->OnValueChanged.AddDynamic(this, &UGS_AudioOptionUI::OnBGMVolumeChanged);

		// 저장된 값으로 슬라이더 초기화
		InitializeValues();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioOptionUI] BGM 볼륨 슬라이더를 찾을 수 없습니다. 블루프린트에서 직접 바인딩해야 합니다."));
	}
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
		const float CurrentVolume = CachedGameInstance->GetBGMVolume();
		TempBGMVolume = CurrentVolume;

		// UI 값 계산 (중복 제거)
		const float SliderValue = CurrentVolume * 100.0f;

		// 슬라이더 UI 업데이트
		if (BGMVolumeSlider)
		{
			BGMVolumeSlider->SetValue(SliderValue);
		}

		// 텍스트 UI 업데이트
		if (BGMVolumeText)
		{
			BGMVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), SliderValue)));
		}

		// AudioManager 호출은 불필요 (즉시 저장 방식이므로 이미 설정되어 있음)
	}

	// 초기화 완료, 플래그 해제
	bIsInitializing = false;
}

void UGS_AudioOptionUI::OnBGMVolumeChanged(float Value)
{
	// 초기화 중에는 이벤트 무시 (InitializeValues에서 SetValue 호출 시)
	if (bIsInitializing)
	{
		return;
	}

	// 슬라이더 값은 0-100 범위이므로 0.0-1.0 범위로 변환
	const float NormalizedVolume = FMath::Clamp(Value / 100.0f, 0.0f, 1.0f);

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

void UGS_AudioOptionUI::ApplySettings_Implementation()
{
	// 즉시 저장 방식이므로 OnBGMVolumeChanged에서 이미 저장됨
	// 저장 버튼 호출 시를 대비해 한 번 더 호출하지만, GameInstance에서 중복 저장 방지됨
	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->SetBGMVolume(TempBGMVolume);
	}
}

void UGS_AudioOptionUI::CancelSettings_Implementation()
{
	// 즉시 저장 방식이므로 이미 변경사항이 저장되어 있음
	// InitializeValues를 호출하면 저장된 값(현재 값)을 다시 로드함
	InitializeValues();
}

