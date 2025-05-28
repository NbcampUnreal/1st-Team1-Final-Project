#include "System/GameState/GS_InGameGS.h"
#include "Net/UnrealNetwork.h"
#include "System/GameMode/GS_InGameGM.h"
#include "Kismet/GameplayStatics.h"

AGS_InGameGS::AGS_InGameGS()
{
	TotalGameTime = 900.0f;
	CurrentTime = 0.0f;
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
	
	DOREPLIFETIME(AGS_InGameGS, CurrentTime);
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


void AGS_InGameGS::OnRep_CurrentTime()
{
	OnTimerUpdated.Broadcast(GetFormattedTime());
}

