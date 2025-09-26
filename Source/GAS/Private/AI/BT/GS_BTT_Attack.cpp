#include "AI/BT/GS_BTT_Attack.h"
#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/GS_Character.h"

UGS_BTT_Attack::UGS_BTT_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UGS_BTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AGS_AIController* AIController = Cast<AGS_AIController>(OwnerComp.GetAIOwner());
	if(!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard->GetValueAsBool(AGS_AIController::CanAttackKey))
	{
		return EBTNodeResult::Failed;
	}

	// 타겟이 죽었는지 확인
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(AGS_AIController::TargetActorKey));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	if (AGS_Character* TargetChar = Cast<AGS_Character>(TargetActor))
	{
		if (TargetChar->IsDead())
		{
			return EBTNodeResult::Failed;
		}
	}

	const float Now = OwnerComp.GetWorld()->GetTimeSeconds();
	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(AGS_AIController::LastAttackTimeKey, Now);

	AGS_Monster* Monster = Cast<AGS_Monster>(OwnerComp.GetAIOwner()->GetPawn());
	if(!Monster)
	{
		return EBTNodeResult::Failed;
	}

	Monster->Attack();
	return EBTNodeResult::InProgress;
}

void UGS_BTT_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 공격 중에도 타겟이 죽었는지 확인
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(AGS_AIController::TargetActorKey));
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (AGS_Character* TargetChar = Cast<AGS_Character>(TargetActor))
	{
		if (TargetChar->IsDead())
		{
			// 타겟이 죽었으면 공격 중단
			AGS_Monster* Monster = Cast<AGS_Monster>(OwnerComp.GetAIOwner()->GetPawn());
			if (Monster)
			{
				UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
				if (AnimInstance->Montage_IsPlaying(Monster->AttackMontage))
				{
					AnimInstance->Montage_Stop(0.2f, Monster->AttackMontage);
				}

				// 모든 공격 관련 사운드 중단
				if (Monster->MonsterAudioComponent)
				{
					Monster->MonsterAudioComponent->StopSwingSound();
					Monster->MonsterAudioComponent->StopCombatSound();
				}
			}
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}

	AGS_Monster* Monster = Cast<AGS_Monster>(OwnerComp.GetAIOwner()->GetPawn());
	if(!Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
	if (!AnimInstance->Montage_IsPlaying(Monster->AttackMontage))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UGS_BTT_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AGS_Monster* Monster = Cast<AGS_Monster>(OwnerComp.GetAIOwner()->GetPawn());
	if(Monster)
	{
		UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
		if (AnimInstance->Montage_IsPlaying(Monster->AttackMontage))
		{
			AnimInstance->Montage_Stop(0.0f, nullptr);
		}

		// 모든 공격 관련 사운드 중단
		if (Monster->MonsterAudioComponent)
		{
			Monster->MonsterAudioComponent->StopSwingSound();
			Monster->MonsterAudioComponent->StopCombatSound();
		}
	}

	return EBTNodeResult::Aborted;
}
