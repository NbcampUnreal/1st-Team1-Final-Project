// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
// 필요한 UE 헤더들
#include "Engine/Texture2D.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlineSessionInterface.h"

// --- Steamworks API 헤더 포함 ---
// AdvancedSteamFriendsLibrary.h에서 가져온 경고 비활성화 pragma
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#pragma warning(disable:4265)
#endif

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
#include <steam/steam_api.h> // Steam API 직접 포함
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

class FUniqueNetIdSteam2 : public FUniqueNetId
{
PACKAGE_SCOPE:
	/** Holds the net id for a player */
	uint64 UniqueNetId;

	/** Hidden on purpose */
	FUniqueNetIdSteam2() :
		UniqueNetId(0)
	{
	}

	/**
	* Copy Constructor
	*
	* @param Src the id to copy
	*/
	explicit FUniqueNetIdSteam2(const FUniqueNetIdSteam2& Src) :
		UniqueNetId(Src.UniqueNetId)
	{
	}

public:
	/**
	* Constructs this object with the specified net id
	*
	* @param InUniqueNetId the id to set ours to
	*/
	explicit FUniqueNetIdSteam2(uint64 InUniqueNetId) :
		UniqueNetId(InUniqueNetId)
	{
	}

	/**
	* Constructs this object with the steam id
	*
	* @param InUniqueNetId the id to set ours to
	*/
	explicit FUniqueNetIdSteam2(CSteamID InSteamId) :
		UniqueNetId(InSteamId.ConvertToUint64())
	{
	}

	/**
	* Constructs this object with the specified net id
	*
	* @param String textual representation of an id
	*/
	explicit FUniqueNetIdSteam2(const FString& Str) :
		UniqueNetId(FCString::Atoi64(*Str))
	{
	}


	/**
	* Constructs this object with the specified net id
	*
	* @param InUniqueNetId the id to set ours to (assumed to be FUniqueNetIdSteam in fact)
	*/
	explicit FUniqueNetIdSteam2(const FUniqueNetId& InUniqueNetId) :
		UniqueNetId(*(uint64*)InUniqueNetId.GetBytes())
	{
	}

	virtual FName GetType() const override
	{
		return STEAM_SUBSYSTEM;
	}

	/**
	* Get the raw byte representation of this net id
	* This data is platform dependent and shouldn't be manipulated directly
	*
	* @return byte array of size GetSize()
	*/
	virtual const uint8* GetBytes() const override
	{
		return (uint8*)&UniqueNetId;
	}

	/**
	* Get the size of the id
	*
	* @return size in bytes of the id representation
	*/
	virtual int32 GetSize() const override
	{
		return sizeof(uint64);
	}

	/**
	* Check the validity of the id
	*
	* @return true if this is a well formed ID, false otherwise
	*/
	virtual bool IsValid() const override
	{
		return UniqueNetId != 0 && CSteamID(UniqueNetId).IsValid();
	}

	/**
	* Platform specific conversion to string representation of data
	*
	* @return data in string form
	*/
	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("%llu"), UniqueNetId);
	}

	/**
	* Get a human readable representation of the net id
	* Shouldn't be used for anything other than logging/debugging
	*
	* @return id in string form
	*/
	virtual FString ToDebugString() const override
	{
		CSteamID SteamID(UniqueNetId);
		if (SteamID.IsLobby())
		{
			return FString::Printf(TEXT("Lobby [0x%llX]"), UniqueNetId);
		}
		else if (SteamID.BAnonGameServerAccount())
		{
			return FString::Printf(TEXT("Server [0x%llX]"), UniqueNetId);
		}
		else if (SteamID.IsValid())
		{
			const FString NickName(SteamFriends() ? UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(UniqueNetId)) : TEXT("UNKNOWN"));
			return FString::Printf(TEXT("%s [0x%llX]"), *NickName, UniqueNetId);
		}
		else
		{
			return FString::Printf(TEXT("INVALID [0x%llX]"), UniqueNetId);
		}
	}


	virtual uint32 GetTypeHash() const override
	{
		return ::GetTypeHash(UniqueNetId);
	}

	/** Convenience cast to CSteamID */
	operator CSteamID()
	{
		return UniqueNetId;
	}

	/** Convenience cast to CSteamID */
	operator const CSteamID() const
	{
		return UniqueNetId;
	}

	/** Convenience cast to CSteamID pointer */
	operator CSteamID* ()
	{
		return (CSteamID*)&UniqueNetId;
	}

	/** Convenience cast to CSteamID pointer */
	operator const CSteamID* () const
	{
		return (const CSteamID*)&UniqueNetId;
	}

	friend FArchive& operator<<(FArchive& Ar, FUniqueNetIdSteam2& UserId)
	{
		return Ar << UserId.UniqueNetId;
	}
};


// --- 필요한 Enum 및 Struct 정의 ---
// AdvancedSteamFriendsLibrary.h 에서 복사
UENUM(Blueprintable)
enum class ESteamAvatarSize : uint8
{
	SteamAvatar_INVALID = 0,
	SteamAvatar_Small = 1,
	SteamAvatar_Medium = 2,
	SteamAvatar_Large = 3
};

#include "SteamAvatarHelper.generated.h"

UCLASS()
class GAS_API USteamAvatarHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Steam Avatar")
	static UTexture2D* GetSteamAvatar(const FUniqueNetIdRepl& UniqueNetId, ESteamAvatarSize AvatarSize = ESteamAvatarSize::SteamAvatar_Medium);

	UFUNCTION(BlueprintCallable, Category = "Steam Avatar")
	static FUniqueNetIdRepl GetLocalSteamID();
	
	
};
