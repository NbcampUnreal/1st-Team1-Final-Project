#include "Props/Placer/GS_PlacerBase.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/Player/Monster/GS_IronFang.h"
#include "DungeonEditor/GS_DEController.h"
#include "DungeonEditor/Component/PlaceInfoComponent.h"
#include "DungeonEditor/Data/GS_PlaceableObjectsRow.h"
#include "Props/GS_RoomBase.h"
#include "RuneSystem/GS_EnumUtils.h"


AGS_PlacerBase::AGS_PlacerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	StaticMeshCompo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshCompo->SetupAttachment(RootComponent);

	StaticMeshCompo->SetCollisionProfileName("NoCollision");
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		TEXT("StaticMesh'/Game/DungeonEditor/Meshes/SM_Plane_100x100.SM_Plane_100x100'"));
	if (MeshFinder.Succeeded())
	{
		PlaneMesh = MeshFinder.Object;
		if (nullptr != PlaneMesh)
		{
			StaticMeshCompo->SetStaticMesh(PlaneMesh);
		}
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlaceAcceptedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Place_Indicators/MI_Place_Accepted.MI_Place_Accepted'"));
	if (PlaceAcceptedMatFinder.Succeeded())
	{
		PlaceAcceptedMaterial = PlaceAcceptedMatFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlaceRejectedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Place_Indicators/MI_Place_Rejected.MI_Place_Rejected'"));
	if (PlaceRejectedMatFinder.Succeeded())
	{
		 PlaceRejectedMaterial = PlaceRejectedMatFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BuildAcceptedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Building_Indicators/MI_Building_Accepted.MI_Building_Accepted'"));
	if (BuildAcceptedMatFinder.Succeeded())
	{
		BuildAcceptedMaterial = BuildAcceptedMatFinder.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BuildRejectedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Building_Indicators/MI_Building_Rejected.MI_Building_Rejected'"));
	if (BuildRejectedMatFinder.Succeeded())
	{
		BuildRejectedMaterial = BuildRejectedMatFinder.Object;
	}

	// 사운드 로드
	PlaceSuccessSound = nullptr;
	PlaceFailSound = nullptr;
	
	// 드래그 사운드 로드
	static ConstructorHelpers::FObjectFinder<USoundBase> DragSoundFinder(
		TEXT("SoundWave'/Game/WwiseAudio/Audio/SFX_DE_GridSnap.SFX_DE_GridSnap'"));
	if (DragSoundFinder.Succeeded())
	{
		PlaceDragSound = DragSoundFinder.Object;
	}
	else
	{
		PlaceDragSound = nullptr;
	}

	// 드래그 사운드 관련 초기화
	LastCellPosition = FIntPoint(-1, -1);
	LastDragSoundTime = 0.0f;

	bUpdatePlaceIndicators = false;
	Direction = EPlacerDirectionType::Forward;
}

void AGS_PlacerBase::BeginPlay()
{
	Super::BeginPlay();

	SetupObjectPlacer();
}

void AGS_PlacerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawPlacementIndicators();
}

void AGS_PlacerBase::SetupObjectPlacer()
{
	// 데이터 테이블에서 찾아서 세팅해야함.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		if (Cast<AGS_DEController>(PC))
		{
			AGS_DEController* DEPC = Cast<AGS_DEController>(PC);
			if (DEPC->GetBuildManager())
			{
				BuildManagerRef = Cast<AGS_DEController>(PC)->GetBuildManager();
			}
		}
	}

	if (ObjectNameInTable.DataTable)
	{
		const FGS_PlaceableObjectsRow* RowPtr = ObjectNameInTable.GetRow<FGS_PlaceableObjectsRow>(ObjectNameInTable.RowName.ToString());
		if (nullptr != RowPtr)
		{
			ObjectData = *RowPtr;

			ObjectSize = { FMath::Clamp(ObjectData.ObjectSize.X, 1, ObjectData.ObjectSize.X)
				, FMath::Clamp(ObjectData.ObjectSize.Y, 1, ObjectData.ObjectSize.Y) };
		}
	}

	int LoopCount = ObjectSize.X * ObjectSize.Y;
	for (int i = 0; i < LoopCount; ++i)
	{
		CreateIndicatorMesh();
	}

	bUpdatePlaceIndicators = true;
}

void AGS_PlacerBase::BuildObject()
{
	bUpdatePlaceIndicators = true;

	DrawPlacementIndicators();
	if (bCanBuild)
	{
		// 성공 사운드 재생
		if (PlaceSuccessSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), PlaceSuccessSound);
		}
		
		bUpdatePlaceIndicators = true;

		FIntPoint CursorPoint = BuildManagerRef->GetCellUnderCursor();
		FVector2d CenterLocation = BuildManagerRef->GetCenterOfRectArea(CursorPoint, ObjectSize, RotateYaw);
		FVector SpawnLocation = FVector(CenterLocation.X, CenterLocation.Y, ObjectData.OffSet.Z);
		FVector2D SpawnOffset = FVector2D(ObjectData.OffSet.X, ObjectData.OffSet.Y);
		//FVector SpawnLocation = FVector(CenterLocation.X, CenterLocation.Y, BuildManagerRef->GetLocationUnderCursorCamera().Z);
		FRotator SpawnRotator = GetActorRotation();
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ObjectData.PlaceableObjectClass, SpawnLocation, FRotator::ZeroRotator);
		NewActor->SetActorRotation(NewActor->GetActorRotation() + FRotator(0.0f, RotateYaw, 0.0f));
		SpawnOffset = SpawnOffset.GetRotated(RotateYaw);
		SpawnLocation = NewActor->GetActorLocation();
		NewActor->SetActorLocation(FVector(SpawnLocation.X + SpawnOffset.X, SpawnLocation.Y + SpawnOffset.Y, SpawnLocation.Z));
		
		// 영역 차지 로직
		TArray<FIntPoint> IntPointArray;
		CalCellsInRectArea(IntPointArray);

		EDEditorCellType TargetType = BuildManagerRef->GetTargetCellType(ObjectData.ObjectType, ObjectData.TrapType);
		bool IsRoom = false;
		for (int i = 0; i < IntPointArray.Num(); i++)
		{
			if (ObjectData.ObjectType == EObjectType::Room)
			{
				if (AGS_RoomBase* RoomBase = Cast<AGS_RoomBase>(NewActor))
				{
					RoomBase->HideCeiling();
					RoomBase->UseDepthStencil();
				}
				TargetType = GetRoomCellInfo(i);
				IsRoom = true;
			}
			else if (ObjectData.ObjectType == EObjectType::DoorAndWall)
			{
				if (ObjectData.DoorAndWallType == EDoorAndWallType::Wall)
				{
					TargetType = EDEditorCellType::VerticalPlaceable;
					UActorComponent* ActorCompo = NewActor->FindComponentByTag(UStaticMeshComponent::StaticClass(),"Wall");
					if (ActorCompo)
					{
						if (UStaticMeshComponent* WallStaticMesh = Cast<UStaticMeshComponent>(ActorCompo))
						{
							WallStaticMesh->SetRenderCustomDepth(true);
						}
					}
				}
			}
			BuildManagerRef->SetOccupancyData(IntPointArray[i], TargetType, ObjectData.ObjectType, NewActor, IsRoom);
		}

		// Cell Info를 Component에 전달 및 저장
		if (UPlaceInfoComponent* PlaceInfoCompo = NewActor->GetComponentByClass<UPlaceInfoComponent>())
		{
			PlaceInfoCompo->SetCellInfo(ObjectData.ObjectType, ObjectData.TrapType, IntPointArray);
		}

		// 먼지 이펙트 생성 DustEffectTemplate
		if (DustEffectTemplate)
		{
			const FVector EffectSpawnLocation = NewActor->GetActorLocation();
			const FRotator EffectSpawnRotation = NewActor->GetActorRotation();
			
			UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				DustEffectTemplate,
				EffectSpawnLocation,
				FRotator::ZeroRotator,
				FVector(1.0f),
				true,
				true
			);

			if (SpawnedEffect)
			{
				float EffectScale = 0.25f * FMath::Max(ObjectData.ObjectSize.X , ObjectData.ObjectSize.Y);
				SpawnedEffect->SetFloatParameter(FName("Scale_All"), EffectScale);
				SpawnedEffect->SetRelativeRotation(EffectSpawnRotation);
			}
		}
	}
	else
	{
		// 실패 사운드 재생
		if (PlaceFailSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), PlaceFailSound);
		}
	}
}

void AGS_PlacerBase::ActiveObjectPlacer()
{
	SetActorTickEnabled(true);
	StaticMeshCompo->SetVisibility(true);
}

void AGS_PlacerBase::SetObjectSelectedState(bool InState)
{
	bObjectSelected = InState;
	
	if (!bObjectSelected)
	{
		//Boarder와 HPBar 숨기기..
		// 위 기능은 만들지 않을것이니.. 추후에 선택해서 위치를 다시 잡는 기능을 구현할 때 이용하면 될듯.
	}
}

void AGS_PlacerBase::RotatePlacer()
{
	Direction = (EPlacerDirectionType)(((int)Direction + 1) % (UGS_EnumUtils::GetEnumCount<EPlacerDirectionType>() - 1));

	RotateYaw += 90;
	RotateYaw = RotateYaw % 360;
	SetActorRotation(GetActorRotation() + FRotator(0.0f, 90.0f, 0.0f));
	
	bUpdatePlaceIndicators = true;
	DrawPlacementIndicators();
}

UStaticMeshComponent* AGS_PlacerBase::CreateIndicatorMesh()
{
	UStaticMeshComponent* NewStaticMeshCompo = NewObject<UStaticMeshComponent>(this,UStaticMeshComponent::StaticClass());
	if (NewStaticMeshCompo)
	{
		// 월드에 등록
		NewStaticMeshCompo->RegisterComponent();
		
		PlaceIndicators.Add(NewStaticMeshCompo);
		if (PlaneMesh)
		{
			NewStaticMeshCompo->SetStaticMesh(PlaneMesh);
			NewStaticMeshCompo->SetMaterial(0, PlaceAcceptedMaterial);
			NewStaticMeshCompo->SetVisibility(true);

			if (BuildManagerRef)
			{
				float ScaleXY = BuildManagerRef->GetGridCellSize() * 0.01f;
				NewStaticMeshCompo->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, 1.0f));
			}
		}
	}

	return NewStaticMeshCompo;
}

void AGS_PlacerBase::DrawPlacementIndicators()
{
	if (BuildManagerRef)
	{
		// 현재 커서 위치 가져오기
		FIntPoint CurrentCellPosition = BuildManagerRef->GetCellUnderCursor();
		
		// 커서 위치가 변경되었거나 Indicator를 업데이트 해주어야 할 때만 동작
		if (bUpdatePlaceIndicators || BuildManagerRef->IsCellUnderSursorChanged())
		{
			// 드래그 사운드 재생 (셀 위치가 변경되었고 쿨다운이 지났을 때)
			if (LastCellPosition != CurrentCellPosition && LastCellPosition != FIntPoint(-1, -1))
			{
				float CurrentTime = GetWorld()->GetTimeSeconds();
				if (CurrentTime - LastDragSoundTime >= DragSoundCooldown)
				{
					if (PlaceDragSound)
					{
						UGameplayStatics::PlaySound2D(GetWorld(), PlaceDragSound, 0.3f); // 볼륨 조금 낮게
					}
					LastDragSoundTime = CurrentTime;
				}
			}
			
			// 현재 위치를 이전 위치로 저장
			LastCellPosition = CurrentCellPosition;
			
			bUpdatePlaceIndicators = false;
			bCanBuild = true;

			TArray<FIntPoint> IntPointArray;
			CalCellsInRectArea(IntPointArray);

			EDEditorCellType TargetType = BuildManagerRef->GetTargetCellType(ObjectData.ObjectType, ObjectData.TrapType);
			for (int i = 0; i < IntPointArray.Num(); i++)
			{
				FVector CellLocation = BuildManagerRef->GetCellLocation(IntPointArray[i]);
				PlaceIndicators[i]->SetWorldLocation(CellLocation);
				
				if (!BuildManagerRef->CheckOccupancyData(IntPointArray[i], TargetType))
					//&& FMath::Abs(CellLocation.Z - BaseBuildLevel) < MaxHeightDifferenceforConstruction))
				{
					PlaceIndicators[i]->SetMaterial(0, PlaceAcceptedMaterial);
				}
				else
				{
					PlaceIndicators[i]->SetMaterial(0, PlaceRejectedMaterial);
					bCanBuild = false;
				}
			}
		}
	}
}

void AGS_PlacerBase::CalCellsInRectArea(TArray<FIntPoint>& InIntPointArray)
{
	float BaseBuildLevel = BuildManagerRef->GetCellLocation(BuildManagerRef->GetCellUnderCursor()).Z;
	FVector2d Center = BuildManagerRef->GetCenterOfRectArea(BuildManagerRef->GetCellUnderCursor(), ObjectSize, RotateYaw);
	
	StaticMeshCompo->SetWorldLocation(FVector(Center.X, Center.Y, BaseBuildLevel));
	BuildManagerRef->GetCellsInRectArea(InIntPointArray, BuildManagerRef->GetCellUnderCursor(), ObjectSize, RotateYaw);
}

// EDEditorCellType AGS_PlacerBase::GetTargetCellType()
// {
// 	EObjectType ObjectType = ObjectData.ObjectType;
// 	if (ObjectType == EObjectType::Room)
// 	{
// 		return EDEditorCellType::None;
// 	}
// 	else if (ObjectType == EObjectType::Monster)
// 	{
// 		return EDEditorCellType::FloorPlace;
// 	}
// 	else if (ObjectType == EObjectType::Trap)
// 	{
// 		ETrapPlacement TrapType = ObjectData.TrapType;
// 		if (TrapType == ETrapPlacement::Floor)
// 			return EDEditorCellType::FloorPlace;
// 		else if (TrapType == ETrapPlacement::Ceiling)
// 			return EDEditorCellType::CeilingPlace;
// 		else if (TrapType == ETrapPlacement::Wall)
// 			return EDEditorCellType::WallPlace;
// 	}
// 	else if (ObjectType == EObjectType::DoorAndWall)
// 	{
// 		// 나중에 벽과 문을 구분지어주어야 함.
// 		return EDEditorCellType::Door;
// 	}
// 	else
// 	{
// 		return EDEditorCellType::None;
// 	}
// 	return EDEditorCellType::None;
// }

EDEditorCellType AGS_PlacerBase::GetRoomCellInfo(int InIdx)
{
	FIntPoint FindIdx;
	FindIdx.X = InIdx / ObjectSize.Y;
	FindIdx.Y = InIdx % ObjectSize.Y;
	if (ObjectData.RoomCellInfo.Find(FindIdx))
	{
		return ObjectData.RoomCellInfo[FindIdx];
	}
	return EDEditorCellType::HorizontalPlaceable;
}