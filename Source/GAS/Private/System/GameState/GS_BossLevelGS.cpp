#include "System/GameState/GS_BossLevelGS.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_GameInstance.h"
#include "System/GameMode/GS_BossLevelGM.h"

AGS_BossLevelGS::AGS_BossLevelGS()
{
	BossTotalTime = 900.f;
	BossCurrentTime = 0.f;
	LastServerTimeUpdate = 0.f;
}

void AGS_BossLevelGS::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UGS_GameInstance* GI = Cast<UGS_GameInstance>(GetGameInstance());
		if (GI)
		{
			BossTotalTime = GI->RemainingTime;
		}

		if (BossTotalTime > 0.f)
		{
			GetWorldTimerManager().SetTimer(BossTimerHandle, this, &AGS_BossLevelGS::UpdateBossTime, 1.0f, true);
		}
		else
		{
			AGS_BossLevelGM* GM = GetWorld()->GetAuthGameMode<AGS_BossLevelGM>();
			if (GM)
			{
				GM->OnTimerEnd();
			}
		}
	}
}

void AGS_BossLevelGS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_BossLevelGS, BossCurrentTime);
	DOREPLIFETIME(AGS_BossLevelGS, BossTotalTime);
}

FText AGS_BossLevelGS::GetFormattedBossTime() const
{
	int32 Remain = FMath::Max(int32(BossTotalTime - BossCurrentTime), 0);
	int32 Min = Remain / 60;
	int32 Sec = Remain % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Min, Sec));
}

float AGS_BossLevelGS::GetRemainingBossTime() const
{
	return FMath::Max(0.0f, BossTotalTime - BossCurrentTime);
}

void AGS_BossLevelGS::UpdateBossTime()
{
	BossCurrentTime += 1.0f;

	if (BossCurrentTime >= BossTotalTime)
	{
		if (BossTimerHandle.IsValid() && GetWorldTimerManager().IsTimerActive(BossTimerHandle))
		{
			UE_LOG(LogTemp, Error, TEXT("AGS_BossLevelGM: Game Over Time: %f. Notifying GameMode."), BossCurrentTime);

			GetWorldTimerManager().ClearTimer(BossTimerHandle);

			AGS_BossLevelGM* GM = GetWorld()->GetAuthGameMode<AGS_BossLevelGM>();
			if (GM)
			{
				GM->OnTimerEnd();
			}
		}
	}
}

void AGS_BossLevelGS::OnRep_BossCurrentTime()
{
	LastServerTimeUpdate = GetWorld()->GetTimeSeconds();
}
