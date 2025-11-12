// Private/System/GS_SeamlessTravelLoadingSubsystem.cpp

#include "System/GISubsys/GS_SeamlessTravelLoadingSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"

UGS_SeamlessTravelLoadingSubsystem::UGS_SeamlessTravelLoadingSubsystem()
    : LoadingCoverWidget(nullptr)
    , bIsCoverVisible(false)
{
}

void UGS_SeamlessTravelLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

#if !UE_SERVER
    if (!LoadingCoverWidgetClass)
    {
        static const TCHAR* FallbackPath =
            TEXT("/Game/UI/LevelUI/WBP_SeamlessLoading.WBP_SeamlessLoading_C");
        if (UClass* Cls = StaticLoadClass(UUserWidget::StaticClass(), nullptr, FallbackPath))
        {
            LoadingCoverWidgetClass = Cls;
        }
    }
#endif
}

void UGS_SeamlessTravelLoadingSubsystem::Deinitialize()
{
    if (LoadingCoverWidget)
    {
        LoadingCoverWidget->RemoveFromParent();
        LoadingCoverWidget = nullptr;
    }

    bIsCoverVisible = false;

    Super::Deinitialize();
}

ULocalPlayer* UGS_SeamlessTravelLoadingSubsystem::GetPrimaryLocalPlayer() const
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        return nullptr;
    }

    const TArray<ULocalPlayer*>& LocalPlayers = GI->GetLocalPlayers();
    return LocalPlayers.Num() > 0 ? LocalPlayers[0] : nullptr;
}

void UGS_SeamlessTravelLoadingSubsystem::ShowLoadingCover(ULocalPlayer* LP)
{
#if !UE_SERVER
    if (!LP)
    {
        LP = GetPrimaryLocalPlayer();
    }
    if (!LoadingCoverWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SeamlessLoading] LoadingCoverWidgetClass is null."));
        return;
    }

    if (!IsValid(LoadingCoverWidget))
    {
        // ★ GI를 WorldContext로 사용 (Seamless Travel 동안에도 생존)
        UGameInstance* GI = GetGameInstance();
        if (!GI)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SeamlessLoading] No GameInstance."));
            return;
        }

        LoadingCoverWidget = CreateWidget<UUserWidget>(GI, LoadingCoverWidgetClass);
        if (!LoadingCoverWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SeamlessLoading] CreateWidget failed."));
            return;
        }

        if (LP)
        {
            LoadingCoverWidget->SetOwningLocalPlayer(LP);
            LoadingCoverWidget->AddToPlayerScreen(10000);   // LP 기준으로 추가
        }
        else
        {
            LoadingCoverWidget->AddToViewport(10000);       // LP가 없으면 전역 뷰포트에 추가
        }
    }

    LoadingCoverWidget->SetVisibility(ESlateVisibility::Visible);
    bIsCoverVisible = true;
#endif
}

void UGS_SeamlessTravelLoadingSubsystem::HideLoadingCover()
{
#if !UE_SERVER
    if (!IsValid(LoadingCoverWidget))
    {
        LoadingCoverWidget = nullptr;
        bIsCoverVisible = false;
        return;
    }

    if (LoadingCoverWidget->IsInViewport())
    {
        UE_LOG(LogTemp, Warning, TEXT("화면 가리개 OFF"));
        LoadingCoverWidget->RemoveFromParent();
    }

    LoadingCoverWidget = nullptr;
    bIsCoverVisible = false;
#endif
}
