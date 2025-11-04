// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Props/Item/E_ItemType.h"
#include "GS_ItemData.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ItemData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TMap<FName, UStaticMesh*> ItemMeshs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 CurCount;
};
