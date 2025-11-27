#include "Character/Component/Seeker/GS_MarkerPlacementComponent.h"
#include "Props/Marker/GS_MarkerActor.h"
#include "UI/Marker/GS_MarkerSelectionWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"

UGS_MarkerPlacementComponent::UGS_MarkerPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SelectedMarkerType = EMarkerType::X; // 기본값
}

void UGS_MarkerPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGS_MarkerPlacementComponent::TryPlaceMarker()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	// UI가 열려있지 않고, 선택된 타입이 있다면 바로 배치 시도
	// (만약 처음 F키를 눌렀을 때 무조건 UI를 열고 싶다면 로직 변경 필요)
	// 여기서는 "이미 선택된 상태면 배치, 아니면 UI" 로직을 구현하기 위해
	// UI 인스턴스 여부 등을 체크할 수 있음. 
	// 요구사항: "F키 -> UI 메뉴 ... 다시 F키 -> 배치"
	// 하지만 "배치 방식: F키 -> 레이캐스트" 라고 되어 있음.
	// 일반적인 UX: F키를 짧게 누르면 배치, 길게 누르면 메뉴? 또는 별도 키?
	// 요구사항 분석: "마커 선택: UI 메뉴로 선택", "배치 방식: F키"
	// 따라서 F키는 배치 전용으로 가정하고, 메뉴는 별도 키(또는 F키 홀드)나 
	// 처음 접속 시 기본값이 있으므로 바로 배치되도록 함.
	
	// 레이캐스트 수행
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector End = CamLoc + (CamRot.Vector() * RaycastDistance);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	// Visibility 채널로 트레이스 (벽/바닥)
	if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params))
	{
		// 데칼 회전 계산: Normal 방향으로 정렬
		// 데칼의 X축이 투영 방향이므로, Normal의 반대 방향이 X가 되어야 함 (-Normal)
		// 하지만 보통 DecalComponent는 -X 방향으로 투영함.
		// Unreal Decal: X축이 투영 깊이 방향.
		// 표면에 붙이려면 Decal의 X축이 Surface Normal의 반대 방향이어야 함.
		// RotationFromXVector 사용 시 X축을 정렬.
		
		FRotator DecalRot = Hit.ImpactNormal.Rotation();
		// Hit.ImpactNormal.Rotation()은 X축이 Normal 방향인 회전을 반환.
		// Decal은 -X로 투영하든 X로 투영하든 상관없지만 보통 표면을 바라봐야 함.
		// Decal Actor의 기본 회전값 확인 필요. 보통 ImpactNormal.Rotation() 주면 됨.
		// 단, X가 튀어나오는 방향이므로 데칼 박스가 파묻히지 않게 주의.
		
		// 약간 띄워서 배치하는 것이 좋음? 데칼은 겹쳐도 상관 없음.
		
		Server_SpawnMarker(Hit.Location, DecalRot, SelectedMarkerType);
	}
}

void UGS_MarkerPlacementComponent::OpenMarkerSelectionUI()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !MarkerSelectionWidgetClass) return;

	if (!MarkerSelectionWidget)
	{
		MarkerSelectionWidget = CreateWidget<UGS_MarkerSelectionWidget>(PC, MarkerSelectionWidgetClass);
		if (MarkerSelectionWidget)
		{
			MarkerSelectionWidget->OnMarkerSelected.AddDynamic(this, &UGS_MarkerPlacementComponent::OnMarkerSelected);
		}
	}

	if (MarkerSelectionWidget && !MarkerSelectionWidget->IsInViewport())
	{
		MarkerSelectionWidget->AddToViewport();
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void UGS_MarkerPlacementComponent::SetSelectedMarkerType(EMarkerType NewType)
{
	SelectedMarkerType = NewType;
}

void UGS_MarkerPlacementComponent::OnMarkerSelected(EMarkerType Type)
{
	SelectedMarkerType = Type;
	
	// UI 닫기 및 입력 모드 복구
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
}

void UGS_MarkerPlacementComponent::Server_SpawnMarker_Implementation(FVector Location, FRotator Rotation, EMarkerType Type)
{
	if (!MarkerActorClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGS_MarkerActor* NewMarker = GetWorld()->SpawnActor<AGS_MarkerActor>(MarkerActorClass, Location, Rotation, SpawnParams);
	if (NewMarker)
	{
		NewMarker->MarkerType = Type;
		
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			NewMarker->OwnerPlayerState = OwnerPawn->GetPlayerState();
		}
		
		// Force Net Update to sync immediately
		NewMarker->ForceNetUpdate();
		NewMarker->OnRep_MarkerType(); // Server visual update

		SpawnedMarkers.Add(NewMarker);

		// 개수 제한 체크
		if (SpawnedMarkers.Num() > MaxMarkersPerPlayer)
		{
			RemoveOldestMarker();
		}

		// 마커 배치 사운드 재생 (모든 클라이언트에 멀티캐스트)
		Multicast_PlayMarkerPlacementSound(Location);
	}
}

void UGS_MarkerPlacementComponent::Multicast_PlayMarkerPlacementSound_Implementation(FVector Location)
{
	// 마커 배치 사운드가 설정되어 있는지 확인
	if (!MarkerPlacementSound)
	{
		return;
	}

	// Wwise 오디오 시스템이 초기화되어 있는지 확인
	if (!FAkAudioDevice::Get() || !FAkAudioDevice::Get()->IsInitialized())
	{
		return;
	}

	// 월드 유효성 검사
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 마커 위치에서 사운드 재생 (3D 공간 사운드)
	UAkGameplayStatics::PostEventAtLocation(
		MarkerPlacementSound,
		Location,
		FRotator::ZeroRotator,
		World
	);
}

void UGS_MarkerPlacementComponent::RemoveOldestMarker()
{
	if (SpawnedMarkers.Num() > 0)
	{
		AGS_MarkerActor* Oldest = SpawnedMarkers[0];
		if (IsValid(Oldest))
		{
			Oldest->Destroy();
		}
		SpawnedMarkers.RemoveAt(0);
	}
	
	// 유효하지 않은 마커 정리 (배열 정화)
	for (int32 i = SpawnedMarkers.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(SpawnedMarkers[i]))
		{
			SpawnedMarkers.RemoveAt(i);
		}
	}
}

