#include "Props/Interactables/GS_Door.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Net/UnrealNetwork.h"
#include "AI/RTS/GS_RTSController.h"
#include "AkComponent.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/GS_AudioComponentBase.h"

AGS_Door::AGS_Door()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootSceneComp = CreateDefaultSubobject<UBoxComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;

	DoorFrameMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMeshComp"));
	DoorFrameMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorFrameMeshComp->SetCollisionObjectType(ECC_WorldStatic);
	DoorFrameMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	DoorFrameMeshComp->SetupAttachment(RootComponent);
	DoorFrameMeshComp->PrimaryComponentTick.bCanEverTick = false;
	DoorFrameMeshComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	DoorFrameMeshComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	DoorMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComp"));
	DoorMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMeshComp->SetCollisionObjectType(ECC_WorldStatic);
	DoorMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	DoorMeshComp->SetupAttachment(DoorFrameMeshComp);
	DoorMeshComp->PrimaryComponentTick.bCanEverTick = false;
	DoorMeshComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	DoorMeshComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	TriggerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBoxComp->SetupAttachment(DoorFrameMeshComp);
	TriggerBoxComp->SetGenerateOverlapEvents(true);
	TriggerBoxComp->PrimaryComponentTick.bCanEverTick = false;
	TriggerBoxComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	TriggerBoxComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	AudioAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AudioAnchor"));
	AudioAnchorComponent->SetupAttachment(RootComponent);
	AudioAnchorComponent->PrimaryComponentTick.bCanEverTick = false;
	AudioAnchorComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	AudioAnchorComponent->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	AudioAnchorComponent->SetRelativeLocation(AudioAnchorRelativeLocation);

	// AkComponent는 기본적으로 생성하지 않음 (BP에서 선택적으로 추가)
	DoorAkComponent = nullptr;
}


void AGS_Door::BeginPlay()
{
	Super::BeginPlay();

	RefreshDoorAudioSetup(true);

	InitDoor();
	TriggerBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_Door::OnTriggerBeginOverlap);
}

void AGS_Door::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리 (레벨 전환 안정성)
	SafeClearTimer(DoorCloseTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AGS_Door::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Transform 검증 (서버 안정성)
	const FRotator TransformRotation = Transform.GetRotation().Rotator();
	if (!UGS_AudioComponentBase::IsTransformValid(Transform.GetLocation(), TransformRotation))
	{
		return;
	}

	RefreshDoorAudioSetup(true);
}

void AGS_Door::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Something overlapped: %s"), *OtherActor->GetName());

	AGS_Character* Character = Cast<AGS_Character>(OtherActor);
	if (!Character || !HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Overlap but not AGS_Character: %s"), *OtherActor->GetName());
		return;
	}
	if (!bIsOpen)
	{
		bIsOpen = true;
		Server_DoorOpen(Character);
	}
	
}


void AGS_Door::Server_DoorOpen_Implementation(AActor* TargetActor)
{
	// 문 사운드 먼저 재생 (BP 오버라이드 여부와 무관하게 항상 재생)
	PlayOpenSound();

	//문 열리는 함수 호출하고
	DoorOpen();

	// 안전한 타이머 관리
	SafeClearTimer(DoorCloseTimerHandle);

	if (IsWorldContextValid())
	{
		GetWorldTimerManager().SetTimer(DoorCloseTimerHandle, this, &AGS_Door::CheckForPlayerInTrigger, 2.0f, false);
	}
}

void AGS_Door::CheckForPlayerInTrigger()
{
	if (!IsWorldContextValid())
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	TriggerBoxComp->GetOverlappingActors(OverlappingActors, AGS_Character::StaticClass());

	if (OverlappingActors.Num() > 0)
		//오버랩 되는 엑터가 있으면 타이머 초기화
	{
		if (IsWorldContextValid())
		{
			GetWorldTimerManager().SetTimer(DoorCloseTimerHandle, this, &AGS_Door::CheckForPlayerInTrigger, 2.0f, false);
		}
	}
	else
	{
		// 문 사운드 먼저 재생 (BP 오버라이드 여부와 무관하게 항상 재생)
		PlayCloseSound();

		//오버랩 되는 엑터가 없으면 문 닫히는 함수 호출
		DoorClose();
		bIsOpen = false;
	}
}

void AGS_Door::DoorOpen_Implementation()
{
	// 사운드는 Server_DoorOpen에서 이미 재생됨
	// BP에서 문 여는 애니메이션 등 구현
}

void AGS_Door::DoorClose_Implementation()
{
	// 사운드는 CheckForPlayerInTrigger에서 이미 재생됨
	// BP에서 문 닫는 애니메이션 등 구현
}

// ===============================
// Audio Functions Implementation
// ===============================

bool AGS_Door::IsRTSMode() const
{
	if (!GetWorld())
	{
		return false;
	}

	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC)
	{
		return false;
	}

	return Cast<AGS_RTSController>(LocalPC) != nullptr;
}

UAkAudioEvent* AGS_Door::SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound) const
{
	const bool bRTS = IsRTSMode();
	return bRTS ? RTSSound : TPSSound;
}

bool AGS_Door::ShouldPlayDoorSoundAtLocation(const FVector& DoorLocation) const
{
	// 월드 유효성 체크
	if (!GetWorld() || !IsValid(this))
	{
		return false;
	}

	// 위치 검증 (NaN/Infinity 체크)
	if (!UGS_AudioComponentBase::IsLocationValid(DoorLocation))
	{
		return false;
	}

	// 플레이어 컨트롤러 가져오기
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC || !IsValid(LocalPC))
	{
		return false;
	}

	// 리스너 위치 가져오기
	FVector ListenerLocation;

	if (LocalPC->PlayerCameraManager && IsValid(LocalPC->PlayerCameraManager))
	{
		ListenerLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
	}
	else if (APawn* PlayerPawn = LocalPC->GetPawn())
	{
		if (IsValid(PlayerPawn))
		{
			ListenerLocation = PlayerPawn->GetActorLocation();
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	// 리스너 위치 검증
	if (!UGS_AudioComponentBase::IsLocationValid(ListenerLocation))
	{
		return false;
	}

	const float DistanceToListener = FVector::Dist(DoorLocation, ListenerLocation);

	// 매우 가까우면 항상 재생
	if (DistanceToListener <= 800.0f) // 8m
	{
		return true;
	}

	// 모드별 거리 체크
	const bool bRTS = IsRTSMode();

	if (bRTS)
	{
		// RTS 모드: 거리만 체크 (200m)
		const bool bShouldPlay = DistanceToListener <= 20000.0f;
		return bShouldPlay;
	}
	else
	{
		// TPS 모드: 거리 체크 (최대 거리 사용)
		const bool bShouldPlay = DistanceToListener <= DoorSoundMaxDistance;
		return bShouldPlay;
	}
}

void AGS_Door::SetDoorAkComponent(UAkComponent* NewAkComponent)
{
	// 유효성 체크 (서버 안정성)
	if (!IsValid(NewAkComponent))
	{
		return;
	}

	DoorAkComponent = NewAkComponent;
	RefreshDoorAudioSetup(false);
}

void AGS_Door::PlayOpenSound()
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bIsRTS = IsRTSMode();
	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(OpenSound_TPS, OpenSound_RTS);

	if (SoundEvent)
	{
		Multicast_PlayOpenSound();
	}
}

void AGS_Door::PlayCloseSound()
{
	if (!HasAuthority())
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(CloseSound_TPS, CloseSound_RTS);
	if (SoundEvent)
	{
		Multicast_PlayCloseSound();
	}
}

void AGS_Door::Multicast_PlayOpenSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Actor 유효성 체크 (서버 안정성)
	if (!IsValid(this))
	{
		return;
	}

	// 거리 기반 최적화 체크
	const bool bShouldPlay = ShouldPlayDoorSoundAtLocation(GetActorLocation());
	if (!bShouldPlay)
	{
		return;
	}

	const bool bIsRTS = IsRTSMode();
	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(OpenSound_TPS, OpenSound_RTS);

	if (SoundEvent)
	{
		// DoorAkComponent가 있으면 AudioAnchor 위치에서 재생, 없으면 Actor 자체 사용
		if (IsValid(DoorAkComponent))
		{
			const FVector AkComponentLoc = DoorAkComponent->GetComponentLocation();
			const FVector ActorLoc = GetActorLocation();
			const AkPlayingID PlayingID = DoorAkComponent->PostAkEvent(SoundEvent, 0, FOnAkPostEventCallback());
		}
		else
		{
			const AkPlayingID PlayingID = UAkGameplayStatics::PostEvent(SoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}
}

void AGS_Door::Multicast_PlayCloseSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Actor 유효성 체크 (서버 안정성)
	if (!IsValid(this))
	{
		return;
	}

	// 거리 기반 최적화 체크
	if (!ShouldPlayDoorSoundAtLocation(GetActorLocation()))
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(CloseSound_TPS, CloseSound_RTS);
	if (SoundEvent)
	{
		// DoorAkComponent가 있으면 AudioAnchor 위치에서 재생, 없으면 Actor 자체 사용
		if (IsValid(DoorAkComponent))
		{
			DoorAkComponent->PostAkEvent(SoundEvent, 0, FOnAkPostEventCallback());
		}
		else
		{
			UAkGameplayStatics::PostEvent(SoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}
}

void AGS_Door::RefreshDoorAudioSetup(bool bForceFindComponent)
{
	// World 유효성 체크 (서버 안정성)
	if (!GetWorld() || !IsValid(this))
	{
		return;
	}

	if (IsValid(AudioAnchorComponent))
	{
		AudioAnchorComponent->SetRelativeLocation(AudioAnchorRelativeLocation);
	}

	if (bForceFindComponent || DoorAkComponent == nullptr)
	{
		DoorAkComponent = FindComponentByClass<UAkComponent>();
	}

	if (!IsValid(DoorAkComponent))
	{
		return;
	}

	DoorAkComponent->SetComponentTickEnabled(false);
	AttachDoorAkComponentToAnchor();
}

void AGS_Door::AttachDoorAkComponentToAnchor()
{
	if (!IsValid(DoorAkComponent))
	{
		return;
	}

	// World 유효성 체크 (서버 안정성)
	if (!GetWorld() || !IsValid(this))
	{
		return;
	}

	// 오클루전 완전 비활성화 (방 모듈에 의한 소리 차단 방지)
	DoorAkComponent->OcclusionRefreshInterval = 0.0f;
	DoorAkComponent->EnableSpotReflectors = false;

	if (bUseAudioAnchor && IsValid(AudioAnchorComponent))
	{
		const FVector AnchorLocation = AudioAnchorComponent->GetComponentLocation();
		const FRotator AnchorRotation = AudioAnchorComponent->GetComponentRotation();

		// Transform 검증 (NaN/Infinity 체크)
		if (!UGS_AudioComponentBase::IsTransformValid(AnchorLocation, AnchorRotation))
		{
			return;
		}

		DoorAkComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DoorAkComponent->AttachToComponent(AudioAnchorComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		DoorAkComponent->SetRelativeLocation(FVector::ZeroVector);
		DoorAkComponent->SetRelativeRotation(FRotator::ZeroRotator);
		DoorAkComponent->SetRelativeScale3D(FVector::OneVector);
		DoorAkComponent->SetWorldLocation(AnchorLocation);
		DoorAkComponent->SetWorldRotation(AnchorRotation);
	}
	else if (IsValid(RootComponent))
	{
		DoorAkComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DoorAkComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	}
}

// ===================
// Timer Safety Functions
// ===================

void AGS_Door::SafeClearTimer(FTimerHandle& TimerHandle)
{
	if (TimerHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
		TimerHandle.Invalidate();
	}
}

bool AGS_Door::IsWorldContextValid() const
{
	UWorld* World = GetWorld();
	return World &&
		World->IsValidLowLevel() &&
		!World->bIsTearingDown &&
		IsValid(World) &&
		IsValid(this);
}