// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GS_EnumUtils.generated.h"

/**
 * Enum 관련 함수 라이브러리
 */
UCLASS()
class GAS_API UGS_EnumUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	//Enum을 String으로 반환
	template<typename TEnum>
	static FORCEINLINE FString GetEnumAsString(const TEnum EnumValue)
	{
		const UEnum* EnumPtr = StaticEnum<TEnum>();
		return EnumPtr ? EnumPtr->GetNameStringByValue((int64)EnumValue) : FString("NULL");
	}

	//Enum을 Text로 반환
	template<typename TEnum>
	static FORCEINLINE FText GetEnumAsText(const TEnum EnumValue)
	{
		const UEnum* EnumPtr = StaticEnum<TEnum>();
		return EnumPtr ? EnumPtr->GetDisplayNameTextByValue((int64)EnumValue) : FText::FromString("NULL");
	}

	//Enum 항목 개수 반환
	template<typename TEnum>
	static FORCEINLINE int32 GetEnumCount()
	{
		const UEnum* EnumPtr = StaticEnum<TEnum>();
		return EnumPtr ? EnumPtr->NumEnums() : 0;
	}
};
