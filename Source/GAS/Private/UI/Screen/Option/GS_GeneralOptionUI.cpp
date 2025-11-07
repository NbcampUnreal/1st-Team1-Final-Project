// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Option/GS_GeneralOptionUI.h"
#include "Internationalization/StringTable.h"

void UGS_GeneralOptionUI::NativeConstruct()
{
	Super::NativeConstruct();
	LoadedTable = nullptr;
}

FText UGS_GeneralOptionUI::GetLocalizedText(FString Key)
{
	if (LoadedTable)
	{
		FName TableId = LoadedTable->GetStringTableId();
		return FText::FromStringTable(TableId, Key);
	}
	return FText::GetEmpty();
}
