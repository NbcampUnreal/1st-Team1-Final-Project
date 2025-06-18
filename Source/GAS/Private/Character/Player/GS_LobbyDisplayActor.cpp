// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/GS_LobbyDisplayActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/Character/GS_LobbyAnimInstance.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"

AGS_LobbyDisplayActor::AGS_LobbyDisplayActor()
{
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SetRootComponent(SkeletalMeshComponent);
    bReplicates = true;
}

void AGS_LobbyDisplayActor::SetupDisplay_Implementation(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass,
    const TArray<FWeaponMeshPair>& WeaponMeshList)
{
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetSkeletalMesh(NewMesh);
        SkeletalMeshComponent->SetAnimInstanceClass(NewAnimClass);

        for (const FWeaponMeshPair& WeaponPair : WeaponMeshList)
        {
            UE_LOG(LogTemp, Warning, TEXT("Weapon SkeletalMesh class : %s | Socket Name : %s"), *WeaponPair.WeaponSkeletalMeshClass->GetName(), *WeaponPair.SocketName.ToString());
            USkeletalMeshComponent* WeaponComponent = NewObject<USkeletalMeshComponent>(this);
            WeaponComponent->RegisterComponent();
            WeaponComponent->SetSkeletalMesh(WeaponPair.WeaponSkeletalMeshClass);

            WeaponComponent->AttachToComponent(SkeletalMeshComponent,
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                WeaponPair.SocketName);
        }
    }
}

void AGS_LobbyDisplayActor::SetReadyState_Implementation(bool bIsReady)
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