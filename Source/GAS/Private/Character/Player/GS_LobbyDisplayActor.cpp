// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/GS_LobbyDisplayActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/Character/GS_LobbyAnimInstance.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "UI/Screen/GS_UserIDWidget.h"
#include "System/GS_PlayerState.h"

AGS_LobbyDisplayActor::AGS_LobbyDisplayActor()
{
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SetRootComponent(SkeletalMeshComponent);
    bReplicates = true;

    bAlwaysRelevant = true;
    NetUpdateFrequency = 1;

    UserIDWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("UserIDWidget"));
    UserIDWidgetComponent->SetupAttachment(RootComponent);
    UserIDWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // 위젯이 항상 카메라를 보도록 설정
    UserIDWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, -10.f));
    UserIDWidgetComponent->SetDrawSize(FVector2D(200, 40));
    UserIDWidgetComponent->SetVisibility(true);
}

void AGS_LobbyDisplayActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGS_LobbyDisplayActor, CurrentSkeletalMesh);
    DOREPLIFETIME(AGS_LobbyDisplayActor, CurrentAnimClass);
    DOREPLIFETIME(AGS_LobbyDisplayActor, CurrentWeaponMeshList);
    DOREPLIFETIME(AGS_LobbyDisplayActor, CurrentSubMeshList);
    DOREPLIFETIME(AGS_LobbyDisplayActor, bIsReady);
    DOREPLIFETIME(AGS_LobbyDisplayActor, AssociatedPlayerState);
}

void AGS_LobbyDisplayActor::BeginPlay()
{
    Super::BeginPlay();
    if (UserIDWidgetClass)
    {
        UserIDWidgetComponent->SetWidgetClass(UserIDWidgetClass);
    }
}

void AGS_LobbyDisplayActor::OnRep_SetupDisplay()
{
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetSkeletalMesh(CurrentSkeletalMesh);
        SkeletalMeshComponent->SetAnimInstanceClass(CurrentAnimClass);

        // 기존 무기 제거 (중복 스폰 방지)
        TArray<USceneComponent*> AttachedComponents;
        SkeletalMeshComponent->GetChildrenComponents(true, AttachedComponents);
        for (USceneComponent* Child : AttachedComponents)
        {
            if (Child->IsA<USkeletalMeshComponent>())
            {
                Child->DestroyComponent();
            }
        }

        for (const FWeaponMeshPair& WeaponPair : CurrentWeaponMeshList)
        {
            USkeletalMeshComponent* WeaponComponent = NewObject<USkeletalMeshComponent>(this);
            WeaponComponent->RegisterComponent();
            WeaponComponent->SetSkeletalMesh(WeaponPair.WeaponSkeletalMeshClass);
            WeaponComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponPair.SocketName);
        }

        for (const USkeletalMesh* SubMesh : CurrentSubMeshList)
        {
            USkeletalMeshComponent* SubMeshComponent = NewObject<USkeletalMeshComponent>(this);
            SubMeshComponent->RegisterComponent();
            SubMeshComponent->SetSkeletalMesh(const_cast<USkeletalMesh*>(SubMesh));
    
            SubMeshComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
    
            SubMeshComponent->SetLeaderPoseComponent(SkeletalMeshComponent);
        }
    }
}

void AGS_LobbyDisplayActor::OnRep_ReadyState()
{
    if (SkeletalMeshComponent)
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        if (UGS_LobbyAnimInstance* LobbyAnimInstance = Cast<UGS_LobbyAnimInstance>(AnimInstance))
        {
            LobbyAnimInstance->bIsReady = bIsReady;
        }
    }
}

void AGS_LobbyDisplayActor::OnRep_PlayerState()
{
    if (AssociatedPlayerState && UserIDWidgetComponent)
    {
        UGS_UserIDWidget* UserIDWidget = Cast<UGS_UserIDWidget>(UserIDWidgetComponent->GetUserWidgetObject());
        if (UserIDWidget)
        {
            UserIDWidget->SetupWidget(AssociatedPlayerState);
        }
    }
}
