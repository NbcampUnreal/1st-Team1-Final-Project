// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_StatRow.generated.h"


USTRUCT(BlueprintType)
struct GAS_API FGS_StatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HP = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")

	float ATK = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")

	float DEF = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")

	float AGL = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")

	float ATS = 0.f;

	FGS_StatRow operator+(const FGS_StatRow& Other) const
	{
		FGS_StatRow Result;
		Result.HP = HP + Other.HP;
		Result.ATK = ATK + Other.ATK;
		Result.DEF = DEF + Other.DEF;
		Result.AGL = AGL + Other.AGL;
		Result.ATS = ATS + Other.ATS;
		return Result;
	}
};