// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_RotateControllerYaw.h"

#include <ThirdParty/hlslcc/hlslcc/src/hlslcc_lib/ir_hierarchical_visitor.h>

#include "Character/GS_TpsController.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/OverlapResult.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

void UGS_AN_RotateControllerYaw::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner()))
    {
        if (AGS_TpsController* TpsController = Cast<AGS_TpsController>(Seeker->GetController()))
        {
            UWorld* World = MeshComp->GetWorld();
            if (!World)
            {
                return;
            }

            // Player Controller 기준 원점, 방향.
            FVector PlayerEyeLocation;
            FRotator PlayerEyeRotation;
            TpsController->GetPlayerViewPoint(PlayerEyeLocation, PlayerEyeRotation);
            const FVector ViewDirection = PlayerEyeRotation.Vector();

            // Sweep Collision.
            constexpr float MaxRange = 1800.f; // 전방 거리
            constexpr float AimRadius = 45.f; // 두께
            const FVector Start = PlayerEyeLocation;
            const FVector End = PlayerEyeLocation + ViewDirection * MaxRange;

            // Collision Channel.
            FCollisionObjectQueryParams ObjTypes;
            ObjTypes.AddObjectTypesToQuery(ECC_Pawn); // 추후 몬스터 커스텀 채널로 변경.

            // List of collision channel to ignore.
            FCollisionQueryParams IgnoreCollisionQueryParams(SCENE_QUERY_STAT(BasicAttackAimAssist), false, Seeker);
            IgnoreCollisionQueryParams.AddIgnoredActor(Seeker); // 자기 자신 무시. // 추후 몬스터 커스틈 채널로 변경하여 해당 로직 삭제.

            // Sphere Sweep.
            TArray<FHitResult> HitResults;
            const FCollisionShape Sphere = FCollisionShape::MakeSphere(AimRadius);

            const bool bDetected = World->SweepMultiByObjectType(
                HitResults,
                Start, End, FQuat::Identity,
                ObjTypes,
                Sphere,
                IgnoreCollisionQueryParams
                );

            if (bDetected)
            {
                // 감지된 몬스터 중 가장 앞의 몬스터 선별.
                AActor* TargetActor = nullptr;
                float NearStandard = TNumericLimits<float>::Max();

                // 하나의 액터의 여러 콜리전 Hit 를 묶는다.
                TSet<AActor*> Seen;

                for (const FHitResult& Hit : HitResults)
                {
                    AActor* Actor = Hit.GetActor();
                    if (!Actor || Actor == Seeker)
                    {
                        continue;
                    }
                    // <- 여기에서 팀 과 적 을 구분.
                    if (Seen.Contains(Actor))
                    {
                        continue;
                    }

                    Seen.Add(Actor);

                    if (Hit.Time < NearStandard)
                    {
                        NearStandard = Hit.Time;
                        TargetActor = Actor;
                    }
                }

                if (!TargetActor)
                {
                    Seeker->SetActorRotation(FRotator(0.f, PlayerEyeRotation.Yaw, 0.f));
                    return;
                }

                // 지정된 Target Monster 의 중심을 보도록 Yaw 보정.
                const FVector From = Seeker->GetActorLocation();
                const FVector To = TargetActor->GetActorLocation();

                const FRotator DesiredYawOnly(0.f, (To - From).Rotation().Yaw, 0.f);
                Seeker->SetActorRotation(DesiredYawOnly);

                // Debugging
                /*if (Seeker->HasAuthority())
                {
                    TpsController->Client_DrawAimAssistDebug(Start, End, TargetActor->GetActorLocation(), 1.0f);
                }*/
            }
            else
            {
                Seeker->SetActorRotation(FRotator(0.f, PlayerEyeRotation.Yaw, 0.f));
            }
        }
    }
}

