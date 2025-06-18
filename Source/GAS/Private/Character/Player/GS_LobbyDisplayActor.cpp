// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/GS_LobbyDisplayActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/Character/GS_LobbyAnimInstance.h"

AGS_LobbyDisplayActor::AGS_LobbyDisplayActor()
{
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SetRootComponent(SkeletalMeshComponent);
    bReplicates = true;
}

void AGS_LobbyDisplayActor::SetupDisplay_Implementation(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass)
{
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetSkeletalMesh(NewMesh);
        SkeletalMeshComponent->SetAnimInstanceClass(NewAnimClass);
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