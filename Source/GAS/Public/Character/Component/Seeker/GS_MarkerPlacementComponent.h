#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/GS_MarkerTypes.h"
#include "GS_MarkerPlacementComponent.generated.h"

class AGS_MarkerActor;
class UGS_MarkerSelectionWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_MarkerPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_MarkerPlacementComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// 마커 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	TSubclassOf<AGS_MarkerActor> MarkerActorClass;

	// 플레이어당 최대 마커 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	int32 MaxMarkersPerPlayer = 10;

	// 레이캐스트 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	float RaycastDistance = 500.0f;

	// 마커 선택 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UGS_MarkerSelectionWidget> MarkerSelectionWidgetClass;

	// 마커 배치 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	class UAkAudioEvent* MarkerPlacementSound;

	// 마커 배치 시도 (Input Action에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void TryPlaceMarker();

	// 마커 선택 UI 열기
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void OpenMarkerSelectionUI();

	// 마커 타입 설정
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void SetSelectedMarkerType(EMarkerType NewType);

	// 서버에 마커 스폰 요청
	UFUNCTION(Server, Reliable)
	void Server_SpawnMarker(FVector Location, FRotator Rotation, EMarkerType Type);

	// 마커 배치 사운드 재생 (Multicast)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMarkerPlacementSound(FVector Location);

private:
	// 현재 소유한 마커 목록
	UPROPERTY()
	TArray<AGS_MarkerActor*> SpawnedMarkers;

	// 현재 선택된 마커 타입
	EMarkerType SelectedMarkerType;

	// UI 위젯 인스턴스
	UPROPERTY()
	UGS_MarkerSelectionWidget* MarkerSelectionWidget;

	// 가장 오래된 마커 삭제
	void RemoveOldestMarker();

	// UI 선택 콜백
	UFUNCTION()
	void OnMarkerSelected(EMarkerType Type);
};

