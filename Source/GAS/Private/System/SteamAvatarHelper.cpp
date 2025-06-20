#include "System/SteamAvatarHelper.h"
#include "OnlineSubsystemUtils.h"
#include "TextureResource.h"
#include "PixelFormat.h"

// --- GetLocalSteamID 함수 구현 ---
// AdvancedSteamFriendsLibrary.cpp에서 GetLocalSteamIDFromSteam 함수를 복사 및 수정
FUniqueNetIdRepl USteamAvatarHelper::GetLocalSteamID()
{
    FUniqueNetIdRepl netId;
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
    if (SteamAPI_Init())
    {
        // FUniqueNetIdSteam2를 사용하여 로컬 Steam ID를 생성합니다.
        TSharedPtr<const FUniqueNetId> SteamID(new const FUniqueNetIdSteam2(SteamUser()->GetSteamID()));
        netId.SetUniqueNetId(SteamID);
    }
#endif
    return netId;
}

// --- GetSteamAvatar 함수 구현 ---
// AdvancedSteamFriendsLibrary.cpp에서 GetSteamFriendAvatar 함수를 복사 및 수정
UTexture2D* USteamAvatarHelper::GetSteamAvatar(const FUniqueNetIdRepl& UniqueNetId, ESteamAvatarSize AvatarSize)
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
    if (!UniqueNetId.IsValid() || UniqueNetId.GetUniqueNetId()->GetType() != STEAM_SUBSYSTEM)
    {
        return nullptr;
    }

    if (SteamAPI_Init())
    {
        uint64 id = *((uint64*)UniqueNetId.GetUniqueNetId()->GetBytes());
        int Picture = 0;

        switch (AvatarSize)
        {
        case ESteamAvatarSize::SteamAvatar_Small: Picture = SteamFriends()->GetSmallFriendAvatar(id); break;
        case ESteamAvatarSize::SteamAvatar_Medium: Picture = SteamFriends()->GetMediumFriendAvatar(id); break;
        case ESteamAvatarSize::SteamAvatar_Large: Picture = SteamFriends()->GetLargeFriendAvatar(id); break;
        default: break;
        }

        if (Picture <= 0) // -1은 로딩 중, 0은 데이터 없음을 의미할 수 있음
        {
            return nullptr;
        }

        uint32 Width = 0;
        uint32 Height = 0;
        if (!SteamUtils()->GetImageSize(Picture, &Width, &Height) || Width == 0 || Height == 0)
        {
            return nullptr;
        }

        uint8* oAvatarRGBA = new uint8[Width * Height * 4];
        SteamUtils()->GetImageRGBA(Picture, oAvatarRGBA, Width * Height * 4);

        UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

        if (FTexturePlatformData* PlatformData = Avatar->GetPlatformData())
        {
            uint8* MipData = (uint8*)PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            FMemory::Memcpy(MipData, oAvatarRGBA, Height * Width * 4);
            PlatformData->Mips[0].BulkData.Unlock();
            Avatar->UpdateResource();
        }

        delete[] oAvatarRGBA;
        return Avatar;
    }
#endif
    return nullptr;
}

