#include "UI/Character/GS_BossHP.h"

#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_GuardianController.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_DrakharVFXComponent.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerState.h"
#include "System/GS_PlayerRole.h"
#include "UI/Character/GS_DrakharFeverGauge.h"
#include "UI/Character/GS_HPWidget.h"

void UGS_BossHP::NativeConstruct()
{
	Super::NativeConstruct();
	
	// HP 바 색상 초기화
	NormalHPBarColor = FLinearColor(0.7f, 0.0f, 0.0f, 0.8f);
	FeverHPBarColor = FLinearColor(0.6f, 0.0f, 1.0f, 0.8f);  // 피버모드 보라색 (R=0.5, G=0, B=1, A=1)
	
	// 피버모드 상태 초기화
	bLastFeverMode = false;
	
	// 즉시 Guardian 위젯 초기화 시도
	InitGuardianHPWidget();
}

void UGS_BossHP::NativeDestruct()
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FeverModeCheckTimer);
		GetWorld()->GetTimerManager().ClearTimer(GuardianInitRetryTimer);
	}
	
	Super::NativeDestruct();
}

void UGS_BossHP::ShowBossHP()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UGS_BossHP::HideBossHP()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UGS_BossHP::OnBossFightStateChanged(bool bInBossFight)
{
	AGS_PlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<AGS_PlayerState>();
	AGS_Player* CurrentPlayer = Cast<AGS_Player>(GetOwningPlayer()->GetPawn());
	bool bIsGuardianPlayer = IsValid(CurrentPlayer) && CurrentPlayer->IsA<AGS_Guardian>();
	
	// 가디언은 항상 보임
	if (IsValid(PlayerState) && (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian || bIsGuardianPlayer))
	{
		ShowBossHP();
		return;
	}

	// 시커도 가디언 HP를 항상 볼 수 있도록 변경
	if (IsValid(PlayerState) && PlayerState->CurrentPlayerRole == EPlayerRole::PR_Seeker)
	{
		ShowBossHP();
		return;
	}

	// 기타 경우에는 보스 전투 상태에 따라 결정
	if (bInBossFight)
	{
		ShowBossHP();
	}
	else
	{
		HideBossHP();
	}
}

void UGS_BossHP::InitGuardianHPWidget()
{
	if (!IsValid(HPWidgetList))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GS_BossHP] HPWidgetList is not valid!"));
		return;
	}
	
	if (IsValid(HPWidgetClass))
	{
		HPWidgetList->ClearChildren();
	}

	if (FindBoss())
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]------------Boss OK???")),true, true, FLinearColor::Red,8.f);
		
		// HPWidgetClass가 설정되어 있으면 동적으로 위젯 생성
		if (IsValid(HPWidgetClass))
		{
			HPWidgetInstance = CreateWidget<UGS_HPWidget>(this, HPWidgetClass);
			HPWidgetInstance->SetOwningActor(Guardian);
			HPWidgetInstance->InitializeHPWidget(Guardian->GetStatComp());
			HPWidgetList->AddChildToVerticalBox(HPWidgetInstance);
		}
		// HPWidgetClass가 없으면 기존 디자인 요소들에 HP 데이터 연결
		else
		{
			// 가디언의 스탯 컴포넌트에 HP 변경 이벤트 바인딩
			UGS_StatComp* StatComp = Guardian->GetStatComp();
			if (IsValid(StatComp))
			{
				StatComp->OnCurrentHPChanged.AddUObject(this, &UGS_BossHP::OnBossHPChanged);
				// 초기 HP 값 설정
				OnBossHPChanged(StatComp);
			}
			
			// 드라카인 경우 피버모드 상태 변경 감지
			AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Guardian);
			if (IsValid(Drakhar))
			{
				// 초기 색상 설정
				bLastFeverMode = Drakhar->GetIsFeverMode();
				OnFeverModeChanged(bLastFeverMode);
				
				// 드라카의 VFX 컴포넌트 OnFeverModeChanged와 연동
				if (UGS_DrakharVFXComponent* VFXComp = Drakhar->GetVFXComponent())
				{
					// VFX 컴포넌트를 통해 즉각적인 피버모드 변경 감지 (멀티플레이 최적화)
					GetWorld()->GetTimerManager().SetTimer(FeverModeCheckTimer, this, &UGS_BossHP::CheckFeverModeStatus, 0.5f, true);
				}
				else
				{
					// VFX 컴포넌트가 없으면 기존 방식 사용
					GetWorld()->GetTimerManager().SetTimer(FeverModeCheckTimer, this, &UGS_BossHP::CheckFeverModeStatus, 0.1f, true);
				}
			}
		}
		
		// 역할에 따라 초기 위젯 노출 상태 결정
		AGS_PlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<AGS_PlayerState>();
		if (IsValid(PlayerState))
		{
			// 현재 플레이어가 드라카(Guardian)인지 확인
			AGS_Player* CurrentPlayer = Cast<AGS_Player>(GetOwningPlayer()->GetPawn());
			bool bIsGuardianPlayer = IsValid(CurrentPlayer) && CurrentPlayer->IsA<AGS_Guardian>();
			
			// 가디언과 시커 모두 가디언 HP를 볼 수 있음
			if (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian || 
				PlayerState->CurrentPlayerRole == EPlayerRole::PR_Seeker || 
				bIsGuardianPlayer)
			{
				ShowBossHP();
			}
			else
			{
				// 기타 역할은 숨김
				HideBossHP();
			}
		}
		else
		{
			// 안전을 위해 PlayerState가 없으면 숨김
			HideBossHP();
		}
	}
	else
	{
		// Guardian를 찾지 못했을 경우 0.5초 후 재시도
		GetWorld()->GetTimerManager().SetTimer(GuardianInitRetryTimer, this, &UGS_BossHP::InitGuardianHPWidget, 0.5f, false);
		
		// 보스를 찾지 못하면 일시적으로 숨김
		HideBossHP();
	}
}

bool UGS_BossHP::FindBoss()
{
	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (!IsValid(GS))
	{
		return false;
	}

	TArray<APlayerState*> PSA = GS->PlayerArray;
	for (APlayerState* PS : PSA)
	{
		AGS_PlayerState* GSPS = Cast<AGS_PlayerState>(PS);
		if (IsValid(GSPS))
		{
			AGS_Player* Player = Cast<AGS_Player>(GSPS->GetPawn());
			if (IsValid(Player) && Player->IsA<AGS_Guardian>())
			{
				Guardian = Cast<AGS_Guardian>(Player);
				return true;
			}
		}
	}
	return false;
}

void UGS_BossHP::OnBossHPChanged(UGS_StatComp* InStatComp)
{
	if (!IsValid(InStatComp))
	{
		return;
	}

	float CurrentHP = InStatComp->GetCurrentHealth();
	float MaxHP = InStatComp->GetMaxHealth();
	float HPPercent = CurrentHP / MaxHP;

	// Progress Bar 업데이트
	if (IsValid(BossHPBar))
	{
		BossHPBar->SetPercent(HPPercent);
	}
}

void UGS_BossHP::OnFeverModeChanged(bool bIsFeverMode)
{
	if (!IsValid(BossHPBar))
	{
		return;
	}

	// 피버모드에 따라 HP 바 색상 변경
	FLinearColor TargetColor = bIsFeverMode ? FeverHPBarColor : NormalHPBarColor;
	BossHPBar->SetFillColorAndOpacity(TargetColor);
}

void UGS_BossHP::CheckFeverModeStatus()
{
	// 가디언이 유효한지 확인
	if (!IsValid(Guardian))
	{
		// 가디언이 무효하면 타이머 중지
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(FeverModeCheckTimer);
		}
		return;
	}
	
	AGS_Drakhar* CurrentDrakhar = Cast<AGS_Drakhar>(Guardian);
	if (!IsValid(CurrentDrakhar))
	{
		return;
	}
	
	bool bCurrentFeverMode = CurrentDrakhar->GetIsFeverMode();
	if (bLastFeverMode != bCurrentFeverMode)
	{
		OnFeverModeChanged(bCurrentFeverMode);
		bLastFeverMode = bCurrentFeverMode;
	}
}
