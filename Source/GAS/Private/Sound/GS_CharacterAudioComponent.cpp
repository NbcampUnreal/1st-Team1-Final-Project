// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_CharacterAudioComponent.h"
#include "AkGameplayStatics.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "AkAudioDevice.h"
#include "Engine/Engine.h"
#include "Character/Skill/ESkill.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Skill/GS_SkillSet.h"
#include "Character/Player/Seeker/GS_Seeker.h"

UGS_CharacterAudioComponent::UGS_CharacterAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CachedAkComponent = nullptr;
}

void UGS_CharacterAudioComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGS_CharacterAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGS_CharacterAudioComponent::PlaySkill()
{
	FOnAkPostEventCallback DummyCallback;

	SkillEventID = UAkGameplayStatics::PostEvent(SkillEvent,
		GetOwner(), // Post the event to the owner of this component 
		0, // No callback mask
		DummyCallback // No callback
	);
}

void UGS_CharacterAudioComponent::StopSkill()
{
	if (GetOwner())
	{
		UAkGameplayStatics::StopActor(GetOwner());
	}
}

UAkComponent* UGS_CharacterAudioComponent::GetOrCreateAkComponent()
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("UGS_CharacterAudioComponent::GetOrCreateAkComponent - Owner is null"));
		return nullptr;
	}

	// 캐시된 컴포넌트가 유효한지 확인
	if (CachedAkComponent && IsValid(CachedAkComponent))
	{
		return CachedAkComponent;
	}

	// 기존 AkComponent 찾기
	CachedAkComponent = GetOwner()->FindComponentByClass<UAkComponent>();
	
	if (!CachedAkComponent)
	{
		// AkComponent가 없으면 새로 생성
		CachedAkComponent = NewObject<UAkComponent>(GetOwner(), TEXT("RuntimeAkAudioComponent"));
		if (CachedAkComponent)
		{
			CachedAkComponent->SetupAttachment(GetOwner()->GetRootComponent());
			CachedAkComponent->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("UGS_CharacterAudioComponent::GetOrCreateAkComponent - Created new AkComponent"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UGS_CharacterAudioComponent::GetOrCreateAkComponent - Failed to create AkComponent"));
		}
	}
	
	return CachedAkComponent;
}

void UGS_CharacterAudioComponent::PlaySoundAtLocation(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	// 데디케이티드 서버에서는 사운드 재생하지 않음
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}

	if (!SoundEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGS_CharacterAudioComponent::PlaySoundAtLocation - SoundEvent is null"));
		return;
	}

	if (!FAkAudioDevice::Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGS_CharacterAudioComponent::PlaySoundAtLocation - Wwise AudioDevice is not initialized"));
		return;
	}

	if (Location != FVector::ZeroVector)
	{
		UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, GetWorld());
	}
	else
	{
		// 위치가 Zero Vector면 Owner 위치에서 재생
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp)
		{
			AkComp->PostAkEvent(SoundEvent);
		}
	}
}

void UGS_CharacterAudioComponent::PlaySkillSoundFromDataTable(ESkillSlot SkillSlot, bool bIsSkillStart)
{
	const FSkillInfo* TargetSkillInfo = GetSkillInfoFromDataTable(SkillSlot);

	if (!TargetSkillInfo)
	{
		return;
	}

	// 시작/종료 사운드 선택 및 재생
	UAkAudioEvent* SoundToPlay = bIsSkillStart ? TargetSkillInfo->SkillStartSound : TargetSkillInfo->SkillEndSound;
	
	if (SoundToPlay)
	{
		// AGS_Seeker의 Multicast_PlaySkillSound 호출
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(GetOwner()))
		{
			OwnerSeeker->Multicast_PlaySkillSound(SoundToPlay);
		}
	}
}

void UGS_CharacterAudioComponent::PlaySkillCollisionSoundFromDataTable(ESkillSlot SkillSlot, uint8 CollisionType)
{
	const FSkillInfo* TargetSkillInfo = GetSkillInfoFromDataTable(SkillSlot);

	if (!TargetSkillInfo)
	{
		return;
	}

	// 충돌 타입에 따라 적절한 사운드 선택
	UAkAudioEvent* SoundToPlay = nullptr;
	const TCHAR* CollisionTypeName = nullptr;
	
	switch (CollisionType)
	{
	case 0: // Wall
		SoundToPlay = TargetSkillInfo->WallCollisionSound;
		CollisionTypeName = TEXT("Wall");
		break;
	case 1: // Monster
		SoundToPlay = TargetSkillInfo->MonsterCollisionSound;
		CollisionTypeName = TEXT("Monster");
		break;
	case 2: // Guardian
		SoundToPlay = TargetSkillInfo->GuardianCollisionSound;
		CollisionTypeName = TEXT("Guardian");
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("UGS_CharacterAudioComponent::PlaySkillCollisionSoundFromDataTable - Invalid CollisionType"));
		return;
	}

	if (SoundToPlay)
	{
		// AGS_Seeker의 Multicast_PlaySkillSound 호출
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(GetOwner()))
		{
			OwnerSeeker->Multicast_PlaySkillSound(SoundToPlay);
		}
	}
}

const FSkillInfo* UGS_CharacterAudioComponent::GetSkillInfoFromDataTable(ESkillSlot SkillSlot) const
{
	// Owner가 AGS_Player인지 확인
	AGS_Player* OwnerPlayer = Cast<AGS_Player>(GetOwner());
	if (!OwnerPlayer)
	{
		return nullptr;
	}

	// SkillComp 가져오기
	UGS_SkillComp* SkillComp = OwnerPlayer->GetSkillComp();
	if (!SkillComp)
	{
		return nullptr;
	}

	// 데이터 테이블 가져오기
	UDataTable* SkillDataTable = SkillComp->GetSkillDataTable();
	if (!SkillDataTable)
	{
		return nullptr;
	}

	// 캐릭터 타입을 기반으로 RowName 구하기
	FString CharTypeString = UEnum::GetValueAsString(OwnerPlayer->GetCharacterType());
	int32 SeparatorIndex;
	if (CharTypeString.FindChar(TEXT(':'), SeparatorIndex))
	{
		CharTypeString = CharTypeString.RightChop(SeparatorIndex + 2);
	}
	FName RowName = FName(*CharTypeString);

	// 스킬셋 찾기
	FString Context;
	FGS_SkillSet* SkillSet = SkillDataTable->FindRow<FGS_SkillSet>(RowName, Context);
	if (!SkillSet)
	{
		return nullptr;
	}

	// 스킬 슬롯에 따라 적절한 스킬 정보 반환
	switch (SkillSlot)
	{
		case ESkillSlot::Ready:    return &SkillSet->ReadySkill;
		case ESkillSlot::Aiming:   return &SkillSet->AimingSkill;
		case ESkillSlot::Moving:   return &SkillSet->MovingSkill;
		case ESkillSlot::Ultimate: return &SkillSet->UltimateSkill;
		case ESkillSlot::Rolling:  return &SkillSet->RollingSkill;
		default:                   return nullptr;
	}
}

void UGS_CharacterAudioComponent::RequestSkillAudio(ESkillSlot SkillSlot, int32 AudioEventType, FVector Location)
{
	// Event-Driven 방식으로 적절한 함수 호출
	switch (AudioEventType)
	{
	case 0: // SkillStart
		PlaySkillSoundFromDataTable(SkillSlot, true);
		break;
	case 1: // SkillEnd
		PlaySkillSoundFromDataTable(SkillSlot, false);
		break;
	case 2: // WallCollision
		PlaySkillCollisionSoundFromDataTable(SkillSlot, 0);
		break;
	case 3: // MonsterCollision
		PlaySkillCollisionSoundFromDataTable(SkillSlot, 1);
		break;
	case 4: // GuardianCollision
		PlaySkillCollisionSoundFromDataTable(SkillSlot, 2);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("UGS_CharacterAudioComponent::RequestSkillAudio - Invalid AudioEventType: %d"), AudioEventType);
		break;
	}
}

void UGS_CharacterAudioComponent::PlaySkillSoundFromSkillInfo(bool bIsSkillStart, UAkAudioEvent* SkillStartSound, UAkAudioEvent* SkillEndSound)
{
	UAkAudioEvent* SoundToPlay = bIsSkillStart ? SkillStartSound : SkillEndSound;
	
	if (SoundToPlay)
	{
		// ActorComponent에서는 Multicast를 사용할 수 없으므로 직접 재생
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp)
		{
			AkComp->PostAkEvent(SoundToPlay);
		}
	}
}

