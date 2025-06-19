#include "Props/Trap/GS_TrapBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/DamageEvents.h"
#include "Props/Trap/TrapMotion/GS_TrapMotionCompBase.h"
#include "EngineUtils.h"
#include "System/GameMode/GS_InGameGM.h"


AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;
	RootSceneComp->PrimaryComponentTick.bCanEverTick = false;
	RootSceneComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	RootSceneComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	RotationSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RotationScene"));
	RotationSceneComp->SetupAttachment(RootComponent);
	RotationSceneComp->PrimaryComponentTick.bCanEverTick = false;
	RotationSceneComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	RotationSceneComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;


	MeshParentSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("MeshParentSceneComp"));
	MeshParentSceneComp->SetupAttachment(RotationSceneComp);
	MeshParentSceneComp->PrimaryComponentTick.bCanEverTick = false;
	MeshParentSceneComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	MeshParentSceneComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;


	ActivateSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("ActivateSphereComp"));
	ActivateSphereComp->SetupAttachment(MeshParentSceneComp);
	ActivateSphereComp->PrimaryComponentTick.bCanEverTick = false;
	ActivateSphereComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	ActivateSphereComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	ActivateSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivateSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivateSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);




	DamageBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBoxComp->SetupAttachment(MeshParentSceneComp);
	// DamageBox 콜리전 설정
	DamageBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//DamageBoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBoxComp->SetCollisionObjectType(ECC_GameTraceChannel4);
	DamageBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBoxComp->ComponentTags.Add("OptimizedCollision");


}


void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();

	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->ComponentHasTag("OptimizedCollision"))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}


		}
	}
	

	/*if (HasAuthority())
	{
		AGS_TrapManager* TrapManager = GetTrapManager();
		if(TrapManager)
		{
			TrapManager->RegisterTrap(this);
			UE_LOG(LogTemp, Warning, TEXT("[TrapBase] TrapManager in BeginPlay"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[TrapBase] TrapManager is not in BeginPlay"));
		}
		
	}*/

	LoadTrapData();
	DamageBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
	ActivateSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnActivSCompBeginOverlap);
}

//함정 활성화

void AGS_TrapBase::OnActivSCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{

		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
		if (Seeker && !bIsActivated)
		{
			bIsActivated = true;
			if (!HasAuthority())
			{
				Server_ActivateTrap(OtherActor);
			}
			else
			{
				ActivateTrap(OtherActor);
			}

			if (!GetWorld()->GetTimerManager().IsTimerActive(CheckOverlapTimerHandle))
			{
				StartDeactivateTrapCheck();
			}
		}
	}
}

void AGS_TrapBase::Server_ActivateTrap_Implementation(AActor* TargetActor)
{
	ActivateTrap(TargetActor);
}


void AGS_TrapBase::ActivateTrap_Implementation(AActor* TargetActor)
{
	Multicast_EnableOptimizedCollision();
	
}


void AGS_TrapBase::Multicast_EnableOptimizedCollision_Implementation()
{
	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->ComponentHasTag("OptimizedCollision"))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			
		}
	}
}







//Sphere Comp에 End Overlap 시,

void AGS_TrapBase::StartDeactivateTrapCheck()
{
	GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AGS_TrapBase::CheckOverlappingSeeker, 5.0f, true);
}

void AGS_TrapBase::CheckOverlappingSeeker()
{
	TArray<AActor*> OverlappingActors;
	ActivateSphereComp->GetOverlappingActors(OverlappingActors, AGS_Seeker::StaticClass());

	if (OverlappingActors.Num() == 0)
	{
		DeActivateTrap();
		GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
		bIsActivated = false;
	}
}

void AGS_TrapBase::DeActivateTrap_Implementation()
{
	Multicast_DisableOptimizedCollision();
}


void AGS_TrapBase::Multicast_DisableOptimizedCollision_Implementation()
{
	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->ComponentHasTag("OptimizedCollision"))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			
		}
	}
}



//함정 데미지 
void AGS_TrapBase::LoadTrapData()
{
	if (!TrapDataTable) return;
	FTrapData* FoundTrapData = TrapDataTable->FindRow<FTrapData>(TrapID, TEXT("LoadTrapData"));
	if (FoundTrapData)
	{
		TrapData = *FoundTrapData;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrapData not found for TrapID : %s"), *TrapID.ToString());
	}
}

//데미지 박스에 오버랩된 경우 HandleTrapDamage 함수 실행
void AGS_TrapBase::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (!Seeker ||!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Overlapped Actor: %s (%s)"), *OtherActor->GetName(), *OtherActor->GetClass()->GetName());
	if (OtherComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlapped Component: %s (%s)"), *OtherComp->GetName(), *OtherComp->GetClass()->GetName());
	}



	//서버
	DamageBoxEffect(Seeker);
	CustomTrapEffect(Seeker);
	HandleTrapDamage(Seeker);

}



void AGS_TrapBase::Server_HandleTrapDamage_Implementation(AActor* OtherActor)
{
	HandleTrapDamage(OtherActor);
}


void AGS_TrapBase::HandleTrapDamage(AActor* OtherActor)
{
	//UE_LOG(LogTemp, Warning, TEXT("[Trap] HandleTrapDamage called by %s, Authority: %s"),
	//	*GetNameSafe(OtherActor),
	//	HasAuthority() ? TEXT("Server") : TEXT("Client"));
	if (!OtherActor) return;
	AGS_Seeker* DamagedSeeker = Cast<AGS_Seeker>(OtherActor);
	if (!DamagedSeeker) return;

	//디버프 연결
	if (UGS_DebuffComp* DebuffComp = DamagedSeeker->FindComponentByClass<UGS_DebuffComp>())
	{

		const FTrapEffect& Effect = TrapData.Effect;

		//Stun
		if (Effect.bStun)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Stun, nullptr);
		}

		//Slow
		if (Effect.bSlow)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Slow, nullptr);

		}

		//Burn
		if (Effect.bBurn)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Burn, nullptr);

		}

		//Lava
		if (Effect.bLava)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Lava, nullptr);

		}
	}

	//기본 데미지 부여
	if (TrapData.Effect.Damage <= 0.f) return;
	
	FDamageEvent DamageEvent;


	/*if (TrapData.Effect.bDoT)
	{
		ApplyDotDamage(OtherActor);
	}*/
	//else
	//{
	DamagedSeeker->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);
	//}

}

void AGS_TrapBase::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{
}





void AGS_TrapBase::Server_DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Server_DamageBoxEffect_Implementation called"));
	DamageBoxEffect(OtherActor);
}


void AGS_TrapBase::Multicast_DamageBoxEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_DamageBoxEffect_Implementation called"));
	DamageBoxEffect(TargetActor);
}

void AGS_TrapBase::DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("DamageBoxEffect Applied"));
}


void AGS_TrapBase::Server_CustomTrapEffect_Implementation(AActor* TargetActor)
{
	CustomTrapEffect(TargetActor);
}

void AGS_TrapBase::CustomTrapEffect_Implementation(AActor* TargetActor)
{

}


//플레이어가 안에 있는 경우 밀쳐내는 함수
void AGS_TrapBase::PushCharacterInBox(UBoxComponent* CollisionBox, float PushPower)
{
	if (!CollisionBox) return;

	TArray<AActor*> OverlappingActors;
	CollisionBox->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		AGS_Character* Character = Cast<AGS_Character>(Actor);
		if (Character)
		{
			FVector LocalCharacterLocation = GetActorTransform().InverseTransformPosition(Character->GetActorLocation());
			FVector PushDirection = (LocalCharacterLocation.Y >= 0.0f)
				? GetActorRightVector()
				: -GetActorRightVector();

			if (IsBlockedInDirection(Character->GetActorLocation(), PushDirection, 100.0f, Character))
			{
				PushDirection *= -1.0f;
			}

			PushDirection.Z = 0.0f;
			PushDirection = PushDirection.GetSafeNormal();

			FVector LaunchVelocity = PushDirection * PushPower + FVector(0, 0, 200.0f);

			Character->LaunchCharacter(LaunchVelocity, true, true);
		}
	}

}


bool AGS_TrapBase::IsBlockedInDirection(const FVector& Start, const FVector& Direction, float Distance,  AGS_Character* CharacterToIgnore)
{
	FHitResult HitResult;
	FVector End = Start + Direction * Distance;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (CharacterToIgnore)
	{
		Params.AddIgnoredActor(CharacterToIgnore);
	}

	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params);
}



//Trap Motion
AGS_TrapManager* AGS_TrapBase::GetTrapManager() const
{
	/*if (UWorld* World = GetWorld())
	{
		if (AGS_InGameGM* GM = Cast<AGS_InGameGM>(World->GetAuthGameMode()))
		{
			return GM->GetTrapManager();
		}
	}
	return nullptr;*/

	for (TActorIterator<AGS_TrapManager> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}


UGS_TrapMotionCompBase* AGS_TrapBase::GetValidMotionComponent() const
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);

	bool bFoundAnyMotionComp = false;

	for (UActorComponent* Comp : Components)
	{
		if (UGS_TrapMotionCompBase* MotionComp = Cast<UGS_TrapMotionCompBase>(Comp))
		{
			/*UE_LOG(LogTemp, Warning, TEXT("[Trap: %s] MotionComp exists : %s / Active: %s"),
				*GetName(), *MotionComp->GetName(), MotionComp->IsActive() ? TEXT("True") : TEXT("False"));*/
				return MotionComp;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("[Trap: %s] MotionComp does not exist at all"), *GetName());
	return nullptr;
}

bool AGS_TrapBase::CanStartMotion() const
{
	return true;
}



//void AGS_TrapBase::ClearDotTimerForActor(AActor* Actor)
//{
//	if (!Actor)
//	{
//		return;
//	}
//	FTimerHandle TimerHandle;
//	if (ActiveDoTTimers.RemoveAndCopyValue(Actor, TimerHandle))
//	{
//		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
//		UE_LOG(LogTemp, Warning, TEXT("DoT Timer Successfully ended - Actor: %s"), *GetNameSafe(Actor));
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("DoT Timer failed to end - Actor: %s (no timer)"), *GetNameSafe(Actor));
//	}
//
//}





//void AGS_TrapBase::ApplyDotDamage(AActor* DamagedActor)
//{
//	if (!DamagedActor || !HasAuthority() || !TrapData.Effect.bDoT)
//	{
//		return;
//	}
//
//	if (ActiveDoTTimers.Contains(DamagedActor))
//	{
//		ClearDotTimerForActor(DamagedActor);
//	}
//
//	int32 CurrentTick = 0;
//	FTimerHandle TimerHandle;
//
//	TWeakObjectPtr<AActor> WeakActor = DamagedActor;
//
//	FTimerDelegate Delegate;
//	//타이머가 끝나면 실행되는 람다
//	Delegate.BindLambda([=, this]() mutable
//		{
//			if (!WeakActor.IsValid() || !TrapData.Effect.bDoT)
//			{
//				ClearDotTimerForActor(WeakActor.Get());
//				return;
//			}
//			
//			AActor* ValidActor = WeakActor.Get();
//
//			if (!ValidActor || !IsValid(ValidActor))
//			{
//				ClearDotTimerForActor(ValidActor);
//				return;
//			}
//			FDamageEvent DamageEvent;
//			ValidActor->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);
//			UE_LOG(LogTemp, Warning, TEXT("CurrentTick : %d"), CurrentTick);
//			CurrentTick++;
//
//
//
//			if (CurrentTick >= TrapData.Effect.DamageCount)
//			{
//				////current tick이 damage count보다 같거나 크다면 타이머 초기화 후 ActiveDoTTimers 맵에서 제거
//				//if (ActiveDoTTimers.Contains(ValidActor))
//				//{
//				//	GetWorld()->GetTimerManager().ClearTimer(ActiveDoTTimers[ValidActor]);
//				//	
//				//	//크래시 지점
//				//	ActiveDoTTimers.Remove(ValidActor);
//				//	//
//				//}
//
//				ClearDotTimerForActor(ValidActor);
//			}
//		});
//
//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, TrapData.Effect.DamageInterval, true);
//	ActiveDoTTimers.Add(DamagedActor, TimerHandle);
//}