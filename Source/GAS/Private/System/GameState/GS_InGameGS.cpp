#include "System/GameState/GS_InGameGS.h"

#include "EngineUtils.h"
#include "AI/RTS/GS_RTSController.h"
#include "Net/UnrealNetwork.h"
#include "System/GameMode/GS_InGameGM.h"
#include "Kismet/GameplayStatics.h"
#include "Props/GS_RoomBase.h"
#include "System/GS_PlayerState.h"

AGS_InGameGS::AGS_InGameGS()
{
	TotalGameTime = 900.0f;
	CurrentTime = 0.0f;
	LastServerTimeUpdate = 0.0f;
}

void AGS_InGameGS::SetDungeonData(int32 InTotalRoomCount)
{
	// 이 함수는 서버(GameMode)에서만 호출되어야 합니다.
	if (HasAuthority())
	{
		TotalRoomCount = InTotalRoomCount;

		// 이 변수를 true로 설정하면, 잠시 후 모든 클라이언트에서 OnRep_DungeonDataReplicated가 호출됩니다.
		bDungeonDataReady = true;
	}
}

void AGS_InGameGS::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(GameTimeHandle, this, &AGS_InGameGS::UpdateGameTime, 1.0f, true);
	}
}

void AGS_InGameGS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_InGameGS, TotalGameTime);
	DOREPLIFETIME(AGS_InGameGS, CurrentTime);
	DOREPLIFETIME(AGS_InGameGS, TotalRoomCount);
	DOREPLIFETIME(AGS_InGameGS, bDungeonDataReady);
}

void AGS_InGameGS::UpdateGameTime()
{
	CurrentTime += 1.0f;

	if (CurrentTime >= TotalGameTime)
	{
		if (GameTimeHandle.IsValid() && GetWorldTimerManager().IsTimerActive(GameTimeHandle))
		{
			UE_LOG(LogTemp, Error, TEXT("AGS_InGameGS: Game Over Time: %f. Notifying GameMode."), CurrentTime);

			GetWorldTimerManager().ClearTimer(GameTimeHandle);

			AGS_InGameGM* GM = GetWorld()->GetAuthGameMode<AGS_InGameGM>();
			if (GM)
			{
				GM->OnTimerEnd();
			}
		}
	}
}

FText AGS_InGameGS::GetFormattedTime() const
{
	int32 Remain = FMath::Max(int32(TotalGameTime - CurrentTime), 0);
	int32 Min = Remain / 60;
	int32 Sec = Remain % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Min, Sec));
}

float AGS_InGameGS::GetRemainingTime() const
{
	return FMath::Max(0.0f, TotalGameTime - CurrentTime);
}

void AGS_InGameGS::OnRep_CurrentTime()
{
	LastServerTimeUpdate = GetWorld()->GetTimeSeconds();
}

void AGS_InGameGS::OnRep_DungeonDataReplicated()
{
	// bDungeonDataReady가 true라는 신호를 받으면 검증을 시작합니다.
	if (bDungeonDataReady)
	{
		// Guardian 역할을 가진 플레이어의 클라이언트에서만 벽 숨김 처리를 위한 검증을 시작합니다.
		if (AGS_RTSController* MyController = Cast<AGS_RTSController>(GetGameInstance()->GetFirstLocalPlayerController()))
		{
			AGS_PlayerState* PS = MyController->GetPlayerState<AGS_PlayerState>();
			if (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
			{
				UE_LOG(LogTemp, Warning, TEXT("[로딩] 서버 던전 방 생성 준비 완료, 클라 검증 중.."));
				Client_VerifyRoomSpawning();
			}
		}
	}
}

void AGS_InGameGS::Client_VerifyRoomSpawning()
{
	// 현재 내(클라이언트) 월드에 스폰된 Room 액터의 수를 셉니다.
	int32 CurrentLocalRoomCount = 0;
	for (TActorIterator<AGS_RoomBase> It(GetWorld()); It; ++It)
	{
		CurrentLocalRoomCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("[로딩] CLIENT: 방 개수 %d / %d."), CurrentLocalRoomCount, TotalRoomCount);

	// 내 월드의 방 개수가 서버가 알려준 총 개수와 일치하는지 확인합니다.
	if (TotalRoomCount > 0 && CurrentLocalRoomCount >= TotalRoomCount)
	{
		// **검증 성공!** 모든 방이 클라이언트에 도착했습니다.
		UE_LOG(LogTemp, Warning, TEXT("[로딩] CLIENT: 검증 성공. All %d rooms are present. Hiding walls."), TotalRoomCount);

		if (AGS_RTSController* MyController = Cast<AGS_RTSController>(GetGameInstance()->GetFirstLocalPlayerController()))
		{
			// 이제 안전하게 벽 숨김 함수를 호출합니다.
			MyController->HideDungeonElements();
		}
	}
	else
	{
		// 아직 모든 방이 도착하지 않았습니다. 0.1초 뒤에 이 함수를 다시 실행하여 재검사합니다.
		FTimerHandle RetryTimer;
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, this, &AGS_InGameGS::Client_VerifyRoomSpawning, 0.1f, false);
	}
}