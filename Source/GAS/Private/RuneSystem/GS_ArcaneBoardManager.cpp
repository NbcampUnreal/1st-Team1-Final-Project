// Fill out your copyright notice in the Description page of Project Settings.


#include "RuneSystem/GS_ArcaneBoardManager.h"

UGS_ArcaneBoardManager::UGS_ArcaneBoardManager()
{
}

bool UGS_ArcaneBoardManager::SetCurrClass(ECharacterClass NewClass)
{
	return false;
}

bool UGS_ArcaneBoardManager::CanPlaceRuneAt(int32 RuneID, const FIntPoint& Pos)
{
	return false;
}

bool UGS_ArcaneBoardManager::RemoveRune(int32 RuneID)
{
	return false;
}

TMap<FName, float> UGS_ArcaneBoardManager::CalculateStatEffects()
{
	return TMap<FName, float>();
}

void UGS_ArcaneBoardManager::ApplyChanges()
{
}

void UGS_ArcaneBoardManager::ResetAllRune()
{
}

void UGS_ArcaneBoardManager::UpdateConnections()
{
}

void UGS_ArcaneBoardManager::FindConnectedRunes(const TArray<FPlacedRuneInfo>& StartNode, TArray<int32>& CheckedIDs, TArray<int32>& ResultIDs)
{
}
