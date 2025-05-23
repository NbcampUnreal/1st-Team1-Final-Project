// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameState/GS_InGameGS.h"
#include "Net/UnrealNetwork.h"

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
	DOREPLIFETIME_CONDITION(AGS_InGameGS, NumClientsLoaded, COND_None);
	DOREPLIFETIME_CONDITION(AGS_InGameGS, NumClientsExpected, COND_None);
}

void AGS_InGameGS::UpdateGameTime()
{
	CurrentTime += 1.0f;

	if (CurrentTime >= TotalGameTime)
	{
		UE_LOG(LogTemp, Error, TEXT("Game Over Time: %f"), CurrentTime);
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

void AGS_InGameGS::OnRep_Loaded()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Client: OnRep_Loaded - NumClientsLoaded = %d / %d"), NumClientsLoaded, NumClientsExpected);
	}
}

void AGS_InGameGS::AddLoadedClient()
{
	if (HasAuthority())
	{
		++NumClientsLoaded;
		UE_LOG(LogTemp, Log, TEXT("InGameGS: AddLoadedClient -> %d / %d"), NumClientsLoaded, NumClientsExpected);

		if (NumClientsExpected > 0 && NumClientsLoaded >= NumClientsExpected)
		{
			UE_LOG(LogTemp, Warning, TEXT("InGameGS: *** ALL CLIENTS LOADED (%d) *** Broadcasting..."), NumClientsLoaded);
			OnAllClientsLoaded.Broadcast();
		}
	}
}
