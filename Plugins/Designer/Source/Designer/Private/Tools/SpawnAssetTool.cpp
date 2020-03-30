/**
 * MIT License
 * 
 * Copyright(c) 2018 RoelBartstra
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "SpawnAssetTool.h"
#include "DesignerModule.h"

#include "UObject/Class.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/CollisionProfile.h"

#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "AssetSelection.h"
#include "EditorViewportClient.h"
#include "SnappingUtils.h"
#include "Editor/UnrealEd/Private/Editor/ActorPositioning.h"
#include "Editor/UnrealEd/Classes/ActorFactories/ActorFactory.h"
#include "Runtime/Engine/Public/LevelUtils.h"
#include "Runtime/Core/Public/Internationalization/Internationalization.h"

#include "DesignerSettings.h"

#include "Editor.h"

FSpawnAssetTool::FSpawnAssetTool(UDesignerSettings* DesignerSettings)
{
	this->DesignerSettings = DesignerSettings;

	UStaticMesh* StaticMesh = nullptr;
	if (!IsRunningCommandlet())
	{
		UMaterialInterface* SpawnVisualizerMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Designer/MI_SpawnVisualizer.MI_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(SpawnVisualizerMaterial != nullptr);
		SpawnVisualizerMID = UMaterialInstanceDynamic::Create(SpawnVisualizerMaterial, GetTransientPackage());
		check(SpawnVisualizerMID != nullptr);
		StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Designer/SM_SpawnVisualizer.SM_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(StaticMesh != nullptr);
		UMaterialInterface* PreviewActorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Designer/MI_PreviewActor.MI_PreviewActor"), nullptr, LOAD_None, nullptr);
		PreviewActorMID = UMaterialInstanceDynamic::Create(PreviewActorMaterial, GetTransientPackage());
		check(PreviewActorMID != nullptr);
		UMaterialInterface* PreviewActorPulsingMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Designer/MI_PreviewActorPulsing.MI_PreviewActorPulsing"), nullptr, LOAD_None, nullptr);
		PreviewActorPulsingMID = UMaterialInstanceDynamic::Create(PreviewActorPulsingMaterial, GetTransientPackage());
		check(PreviewActorPulsingMID != nullptr);
	}

	SpawnVisualizerComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SpawnVisualizerComponent"));
	SpawnVisualizerComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SpawnVisualizerComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SpawnVisualizerComponent->SetStaticMesh(StaticMesh);
	SpawnVisualizerComponent->SetMaterial(0, SpawnVisualizerMID);
	SpawnVisualizerComponent->SetAbsolute(true, true, true);
	SpawnVisualizerComponent->CastShadow = false;

	// For debugging.
	DesignerSettings->PreviewActorMaterial = PreviewActorMID;
	DesignerSettings->PreviewActorPulsingMaterial = PreviewActorPulsingMID;
}

FSpawnAssetTool::~FSpawnAssetTool()
{

}

void FSpawnAssetTool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DesignerSettings);
	Collector.AddReferencedObject(SpawnVisualizerComponent);
}

FString FSpawnAssetTool::GetName() const
{
	return TEXT("SpawnAssetTool");
}

void FSpawnAssetTool::EnterTool()
{
	SetToolActive(false);
	PreviewActor = nullptr;
	SpawnedActor = nullptr;
	SpawnVisualizerComponent->SetVisibility(true);
}

void FSpawnAssetTool::ExitTool()
{
	SpawnedActor = nullptr;

	if (SpawnVisualizerComponent->IsRegistered())
	{
		SpawnVisualizerComponent->UnregisterComponent();
	}
}

bool FSpawnAssetTool::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	//if (IsValid(SpawnedActor))
	//	return InActor == SpawnedActor && !FMath::IsNearlyZero(SpawnedActor->GetActorScale3D().Size());

	//// Make sure select none works when spawning actor.
	//if (InActor != SpawnedActor && !bInSelection)
	//	return true;

	return !IsToolActive;
}

bool FSpawnAssetTool::MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::MouseEnter"));

	RefreshPreviewActors();

	return IsToolActive;
}

bool FSpawnAssetTool::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::MouseLeave"));

	DestroyPreviewActors();

	return IsToolActive;
}

bool FSpawnAssetTool::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	if (PreviewActor != nullptr && SpawnedActor == nullptr)
	{
		RecalculateSpawnTransform(ViewportClient, Viewport);
		UpdatePreviewActorsTransform();
	}

	return IsToolActive;
}

bool FSpawnAssetTool::ReceivedFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::Receive focus"));
	return IsToolActive;
}

bool FSpawnAssetTool::LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	SetToolActive(false);
	DestroyPreviewActors();

	return false;
}

bool FSpawnAssetTool::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 InMouseX, int32 InMouseY)
{
	bool bHandled = IsToolActive;

	if (SpawnedActor != nullptr)
	{
		RecalculateMousePlaneIntersectionWorldLocation(ViewportClient, Viewport);
		UpdateSpawnedActorTransform();
		UpdateSpawnVisualizerMaterialParameters();

		bHandled = true;
	}

	return bHandled;
}

bool FSpawnAssetTool::InputAxis(FEditorViewportClient* InViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime)
{
	return IsToolActive;
}

bool FSpawnAssetTool::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	return IsToolActive;
}

bool FSpawnAssetTool::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = false;

	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey"));

	if (Key == EKeys::LeftControl || Key == EKeys::RightControl)
	{
		if (Event == IE_Pressed && !IsToolActive)
		{
			UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey, tool activated"));
			RefreshPlaceableSelectedAssets();

			// Pick random asset to spawn.
			if (PlaceableSelectedAssets.Num() > 0)
			{
				UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Pcked new random asset"));
				TargetAssetDataToSpawn = PlaceableSelectedAssets[FMath::RandRange(0, PlaceableSelectedAssets.Num() - 1)];
			}

			if (TargetAssetDataToSpawn.GetAsset() != nullptr)
			{
				UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAssetDataToSpawn.GetAsset());
				if (ActorFactory)
				{
					// Recalculate mouse down, if it fails, return.
					if (!RecalculateSpawnTransform(ViewportClient, Viewport))
						return bHandled;

					SetToolActive(true);

					RegenerateRandomRotationOffset();
					RegenerateRandomScale();

					RefreshPreviewActors();

					UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Actor factory available"));
				}
				else
				{
					UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: No actor factory for object"));
				}
			}
		}
		else if (Event == IE_Released && IsToolActive)
		{
			UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey, tool deactivated"));
			SetToolActive(false);
			bHandled = true;

			TargetAssetDataToSpawn = FAssetData();
			DestroyPreviewActors();

			if (SpawnedActor != nullptr)
			{
				GEditor->SelectActor(SpawnedActor, true, true, true, true);
			}
		}
	}

	// Randomize the object again if right mouse button is pressed in this mode.
	if (Key == EKeys::RightMouseButton && Event == IE_Pressed && SpawnedActor != nullptr && IsToolActive)
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey, regenerate random for spawned actor"));

		RegenerateRandomRotationOffset();
		RegenerateRandomScale();
		UpdateSpawnedActorTransform();
		UpdateSpawnVisualizerMaterialParameters();

		bHandled = true;
	}

	if (Key == EKeys::LeftMouseButton && Event == IE_Pressed && IsToolActive)
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey, spawn selected asset"));
		
		DestroyPreviewActors();

		if (TargetAssetDataToSpawn.GetAsset() != nullptr)
		{
			UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAssetDataToSpawn.GetAsset());
			if (ActorFactory)
			{
				// Recalculate mouse down, if it fails, return.
				if (!RecalculateSpawnTransform(ViewportClient, Viewport))
					return bHandled;

				SpawnedActor = GEditor->UseActorFactory(ActorFactory, TargetAssetDataToSpawn, &SpawnWorldTransform);

				DefaultDesignerActorExtent = SpawnedActor->CalculateComponentsBoundingBoxInLocalSpace(true).GetExtent();

				// Properly reset data.
				CursorPlaneIntersectionWorldLocation = SpawnWorldTransform.GetLocation();
				SpawnTracePlane = FPlane();

				FTransform SpawnVisualizerTransform = SpawnWorldTransform;
				SpawnVisualizerTransform.SetScale3D(FVector(10000));
				SpawnVisualizerComponent->SetRelativeTransform(SpawnVisualizerTransform);

				if (!SpawnVisualizerComponent->IsRegistered())
				{
					SpawnVisualizerComponent->RegisterComponentWithWorld(ViewportClient->GetWorld());
				}

				RegenerateRandomRotationOffset();
				RegenerateRandomScale();
				UpdateSpawnedActorTransform();
				UpdateSpawnVisualizerMaterialParameters();

				bHandled = true;
				UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Actor factory available"));
			}
			else
			{
				UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: No actor factory for object"));
			}
		}
	}

	/** Left mouse button released while tool is active */
	if (Key == EKeys::LeftMouseButton && Event == IE_Released && IsToolActive)
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey, free spawned actor"));

		if (SpawnedActor != nullptr && FMath::IsNearlyZero(SpawnedActor->GetActorScale3D().Size()))
		{
			SpawnedActor->Destroy(false, false);
			GEditor->RedrawLevelEditingViewports();
		}
		//else
		//{
		//	GEditor->SelectActor(SpawnedActor, true, true, true, true);
		//}

		//SpawnedActor = nullptr;
		DefaultDesignerActorExtent = FVector::ZeroVector;

		if (SpawnVisualizerComponent->IsRegistered())
		{
			SpawnVisualizerComponent->UnregisterComponent();
		}

		//DestroyPreviewActors();

		bHandled = true;
	}

	return bHandled;
}

void FSpawnAssetTool::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{	
	if (SpawnedActor == nullptr)
	{
		DrawSphere(PDI, SpawnWorldTransform.GetLocation(), SpawnWorldTransform.GetRotation().Rotator(), FVector(5.F), 32, 32, GEngine->DebugEditorMaterial->GetRenderProxy(), SDPG_Foreground, false);
	}
}

void FSpawnAssetTool::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{

}

bool FSpawnAssetTool::StartModify()
{
	return false;
}

bool FSpawnAssetTool::EndModify()
{
	return false;
}

void FSpawnAssetTool::StartTrans()
{

}

void FSpawnAssetTool::EndTrans()
{

}

void FSpawnAssetTool::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{

}

void FSpawnAssetTool::SelectNone()
{

}

bool FSpawnAssetTool::BoxSelect(FBox& InBox, bool InSelect)
{
	return false;
}

bool FSpawnAssetTool::FrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect)
{
	return false;
}

void FSpawnAssetTool::SetToolActive(bool IsActive)
{
	if (IsActive)
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Setting tool active"));
		RefreshPreviewActors();
		GEditor->SelectNone(true, true, false);
	}
	else
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Setting tool inactive"));
		DestroyPreviewActors();
	}

	IsToolActive = IsActive;
}

void FSpawnAssetTool::SetAllMaterialsForActor(AActor* Actor, UMaterialInterface* Material)
{
	if (IsValid(Actor) && IsValid(Material))
	{
		TArray<UPrimitiveComponent*> PrimitiveComponentArray;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponentArray, true);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponentArray)
		{
			if (IsValid(PrimitiveComponent))
			{
				for (int32 MaterialIndex = 0; MaterialIndex < PrimitiveComponent->GetNumMaterials(); MaterialIndex++)
				{
					PrimitiveComponent->SetMaterial(MaterialIndex, Material);
				}
			}
		}
	}
}

void FSpawnAssetTool::RefreshPreviewActors()
{
	DestroyPreviewActors();

	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Refreshing preview actor array"));
	
	UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAssetDataToSpawn.GetAsset());
	if (TargetAssetDataToSpawn.GetClass() != nullptr)
	{
		PreviewActor = SpawnPreviewActorFromFactory(ActorFactory, TargetAssetDataToSpawn, &SpawnWorldTransform, RF_Transient);
		if (IsValid(PreviewActor) && PreviewActor->IsValidLowLevel())
		{
			PreviewActorArray.Add(PreviewActor);
			PreviewActor->SetActorLabel("DesignerPreviewActor");
			SetAllMaterialsForActor(PreviewActor, PreviewActorMID);
		}

		PreviewActorPulsing = SpawnPreviewActorFromFactory(ActorFactory, TargetAssetDataToSpawn, &SpawnWorldTransform, RF_Transient);
		
		if (IsValid(PreviewActorPulsing) && PreviewActorPulsing->IsValidLowLevel())
		{
			PreviewActorArray.Add(PreviewActorPulsing);
			PreviewActorPulsing->SetActorLabel("DesignerPreviewActorPulsing");
			SetAllMaterialsForActor(PreviewActorPulsing, PreviewActorPulsingMID);
		}

	}
}

void FSpawnAssetTool::DestroyPreviewActors()
{
	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Destroying preview actor array"));

	for (AActor* Actor : PreviewActorArray)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy(false, true);
		}
	}

	PreviewActorArray.Empty();
}

AActor* FSpawnAssetTool::SpawnPreviewActorFromFactory(UActorFactory* Factory, const FAssetData& AssetData, const FTransform* InActorTransform, EObjectFlags InObjectFlags)
{
	check(Factory);

	bool bIsAllowedToCreateActor = true;

	FText ActorErrorMsg;
	if (!Factory->CanCreateActorFrom(AssetData, ActorErrorMsg))
	{
		bIsAllowedToCreateActor = false;
		if (!ActorErrorMsg.IsEmpty())
		{
			FMessageLog EditorErrors("EditorErrors");
			EditorErrors.Warning(ActorErrorMsg);
			EditorErrors.Notify();
		}
	}

	//Load Asset
	UObject* Asset = AssetData.GetAsset();

	UWorld* OldWorld = nullptr;

	//// The play world needs to be selected if it exists
	//if (GIsEditor && PlayWorld && !GIsPlayInEditorWorld)
	//{
	//	OldWorld = SetPlayInEditorWorld(PlayWorld);
	//}

	AActor* Actor = NULL;
	if (bIsAllowedToCreateActor)
	{
		AActor* NewActorTemplate = Factory->GetDefaultActor(AssetData);

		if (!NewActorTemplate)
		{
			return NULL;
		}

		const FTransform ActorTransform = InActorTransform ? *InActorTransform : FActorPositioning::GetCurrentViewportPlacementTransform(*NewActorTemplate);

		ULevel* DesiredLevel = GWorld->GetCurrentLevel();

		// Don't spawn the actor if the current level is locked.
		if (!FLevelUtils::IsLevelLocked(DesiredLevel))
		{
			// Check to see if the level it's being added to is hidden and ask the user if they want to proceed
			const bool bLevelVisible = FLevelUtils::IsLevelVisible(DesiredLevel);
			//if (bLevelVisible || EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::OkCancel, FText::Format(LOCTEXT("CurrentLevelHiddenActorWillAlsoBeHidden", "Current level [{0}] is hidden, actor will also be hidden until level is visible"), FText::FromString(DesiredLevel->GetOutermost()->GetName()))))
			//{
				//const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "CreateActor", "Create Actor"));

				// Create the actor.
				Actor = Factory->CreateActor(Asset, DesiredLevel, ActorTransform, InObjectFlags);
				if (Actor != NULL)
				{
					//SelectNone(false, true);
					//SelectActor(Actor, true, true);
					//Actor->InvalidateLightingCache();
					Actor->PostEditMove(true);

					// Make sure the actors visibility reflects that of the level it's in
					if (!bLevelVisible)
					{
						Actor->bHiddenEdLevel = true;
						// We update components, so things like draw scale take effect.
						Actor->ReregisterAllComponents(); // @todo UE4 insist on a property update callback
					}
				}

				//RedrawLevelEditingViewports();


				if (Actor)
				{
					Actor->MarkPackageDirty();
					ULevel::LevelDirtiedEvent.Broadcast();
				}
			//}
		}
		else
		{
			//FNotificationInfo Info(NSLOCTEXT("UnrealEd", "Error_OperationDisallowedOnLockedLevel", "The requested operation could not be completed because the level is locked."));
			//Info.ExpireDuration = 3.0f;
			//FSlateNotificationManager::Get().AddNotification(Info);
		}
	}

	//// Restore the old world if there was one
	//if (OldWorld)
	//{
	//	RestoreEditorWorld(OldWorld);
	//}

	return Actor;
}

void FSpawnAssetTool::RefreshPlaceableSelectedAssets()
{
	// Refresh selectable asset array;
	PlaceableSelectedAssets.Empty();
	TArray<FAssetData> SelectedAssets;
	AssetSelectionUtils::GetSelectedAssets(SelectedAssets);
	for (FAssetData AssetData : SelectedAssets)
	{
		if (IsAssetDataPlaceable(AssetData))
		{
			PlaceableSelectedAssets.Add(AssetData);
		}
	}

	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: %d placeable assets"), PlaceableSelectedAssets.Num());
}

bool FSpawnAssetTool::IsAssetDataPlaceable(FAssetData AssetData)
{
	bool bPlaceable = false;

	UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(AssetData.GetAsset());
	bPlaceable = ActorFactory != nullptr;

	if (AssetData.GetClass() == UBlueprint::StaticClass() && bPlaceable)
	{
		// For blueprints, attempt to determine placeability from its tag information

		const FName NativeParentClassTag = TEXT("NativeParentClass");
		const FName ClassFlagsTag = TEXT("ClassFlags");

		FString TagValue;

		if (AssetData.GetTagValue(NativeParentClassTag, TagValue) && !TagValue.IsEmpty())
		{
			// If the native parent class can't be placed, neither can the blueprint.
			UObject* Outer = nullptr;
			ResolveName(Outer, TagValue, false, false);
			UClass* NativeParentClass = FindObject<UClass>(ANY_PACKAGE, *TagValue);

			bPlaceable = AssetSelectionUtils::IsClassPlaceable(NativeParentClass);
		}

		if (bPlaceable && AssetData.GetTagValue(ClassFlagsTag, TagValue) && !TagValue.IsEmpty())
		{
			// Check to see if this class is placeable from its class flags
			const int32 NotPlaceableFlags = CLASS_NotPlaceable | CLASS_Deprecated | CLASS_Abstract;
			uint32 ClassFlags = FCString::Atoi(*TagValue);

			bPlaceable = (ClassFlags & NotPlaceableFlags) == CLASS_None;
		}

		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: is placeable blueprint class: %s"), (bPlaceable ? TEXT("True") : TEXT("False")));
	}

	return bPlaceable;
}

bool FSpawnAssetTool::UpdateSpawnVisualizerMaterialParameters()
{
	if (SpawnVisualizerMID)
	{
		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorInputDownWorldLocation"), FLinearColor(SpawnWorldTransform.GetLocation()));

		FVector Extent = DefaultDesignerActorExtent * SpawnedActor->GetActorScale3D();
		EAxisType PositiveAxis = DesignerSettings->GetPositiveAxisToAlignWithCursor();
		float ActorRadius = PositiveAxis == EAxisType::Right ? Extent.Y : PositiveAxis == EAxisType::Up ? Extent.Z : Extent.X;
		ActorRadius = FMath::Abs(ActorRadius);

		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorPlaneWorldLocation"), FLinearColor(CursorPlaneIntersectionWorldLocation.X, CursorPlaneIntersectionWorldLocation.Y, CursorPlaneIntersectionWorldLocation.Z, ActorRadius));

		FLinearColor ForwardVectorColor = PositiveAxis == EAxisType::Up ? FLinearColor::Blue : PositiveAxis == EAxisType::Right ? FLinearColor::Green : FLinearColor::Red;
		SpawnVisualizerMID->SetVectorParameterValue(FName("ForwardAxisColor"), ForwardVectorColor);

		return true;
	}

	return false;
}

bool FSpawnAssetTool::RecalculateSpawnTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	FTransform NewSpawnTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector);

	const int32	HitX = Viewport->GetMouseX();
	const int32	HitY = Viewport->GetMouseY();

	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));
	FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);	

	const FViewportCursorLocation Cursor(View, ViewportClient, HitX, HitY);
	const FActorPositionTraceResult TraceResult = FActorPositioning::TraceWorldForPositionWithDefault(Cursor, *View, &PreviewActorArray);

	// For some reason the state is default when it fails to hit anything.
	if (TraceResult.State == FActorPositionTraceResult::Default)
	{
		return false;
	}

	NewSpawnTransform.SetLocation(TraceResult.Location);

	FRotator CursorWorldRotation = FRotationMatrix::MakeFromZX(GetDesignerSettings()->AxisToAlignWithNormal == EAxisType::None ? FVector::UpVector : TraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();

	FRotator SpawnRotationSnapped = CursorWorldRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	CursorWorldRotation.Roll = GetDesignerSettings()->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : CursorWorldRotation.Roll;
	CursorWorldRotation.Pitch = GetDesignerSettings()->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : CursorWorldRotation.Pitch;
	CursorWorldRotation.Yaw = GetDesignerSettings()->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : CursorWorldRotation.Yaw;
	NewSpawnTransform.SetRotation(CursorWorldRotation.Quaternion());

	SpawnWorldTransform = NewSpawnTransform;
	
	return true;
}

void FSpawnAssetTool::RecalculateMousePlaneIntersectionWorldLocation(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));
	// SceneView is deleted with the ViewFamily
	FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);
	FViewportCursorLocation MouseViewportRay(SceneView, ViewportClient, Viewport->GetMouseX(), Viewport->GetMouseY());

	FViewportCursorLocation ViewportCenterRay(SceneView, ViewportClient, Viewport->GetSizeXY().X * 0.5F, Viewport->GetSizeXY().Y * 0.5F);

	FVector TraceStartLocation = MouseViewportRay.GetOrigin();
	FVector TraceDirection = MouseViewportRay.GetDirection();
	FVector TraceEndLocation = TraceStartLocation + TraceDirection * WORLD_MAX;

	SpawnTracePlane = FPlane(SpawnWorldTransform.GetLocation(), SpawnWorldTransform.GetRotation().GetUpVector());
	CursorPlaneIntersectionWorldLocation = FMath::LinePlaneIntersection(TraceStartLocation, TraceEndLocation, SpawnTracePlane);
}

void FSpawnAssetTool::UpdateSpawnedActorTransform()
{
	FTransform NewSpawnedActorTransform = SpawnWorldTransform;

	FVector CursorDirection;
	float CursorDistance;
	(CursorPlaneIntersectionWorldLocation - SpawnWorldTransform.GetLocation()).ToDirectionAndLength(CursorDirection, CursorDistance);

	FVector NewScale = FVector::OneVector;
	if (GetDesignerSettings()->bApplyRandomScale)
	{
		NewScale = GetRandomScale();

		// If the object also scales towards the mouse we use the randoms scale as a ratio
		if (GetDesignerSettings()->bScaleBoundsTowardsCursor)
		{
			NewScale /= FMath::Max(NewScale.X, FMath::Max(NewScale.Y, NewScale.Z));
		}
	}

	if (GetDesignerSettings()->bScaleBoundsTowardsCursor)
	{
		EAxisType PositiveAxis = GetDesignerSettings()->GetPositiveAxisToAlignWithCursor();
		float BoundsUsedForScale;
		if (PositiveAxis == EAxisType::Forward)
			BoundsUsedForScale = DefaultDesignerActorExtent.X;
		else if (PositiveAxis == EAxisType::Right)
			BoundsUsedForScale = DefaultDesignerActorExtent.Y;
		else if (PositiveAxis == EAxisType::Up)
			BoundsUsedForScale = DefaultDesignerActorExtent.Z;
		else
			BoundsUsedForScale = FMath::Max(DefaultDesignerActorExtent.X, DefaultDesignerActorExtent.Y);

		NewScale *= FVector(CursorDistance / BoundsUsedForScale);
	}

	if (NewScale.ContainsNaN())
	{
		NewScale = FVector::OneVector;
		UE_LOG(LogDesigner, Warning, TEXT("New scale contained NaN, so it is set to one. DefaultDesignerActorExtent = %s."), *DefaultDesignerActorExtent.ToString());
	}

	NewSpawnedActorTransform.SetScale3D(NewScale);

	NewSpawnedActorTransform.SetRotation(GetDesignerActorRotation().Quaternion());
	SpawnedActor->SetActorTransform(NewSpawnedActorTransform);
	SpawnedActor->AddActorWorldOffset(GetDesignerSettings()->WorldLocationOffset);
	SpawnedActor->AddActorLocalOffset(GetDesignerSettings()->RelativeLocationOffset);
}

void FSpawnAssetTool::UpdatePreviewActorsTransform()
{
	if (PreviewActor != nullptr)
	{
		PreviewActor->SetActorTransform(SpawnWorldTransform);
		PreviewActorPulsing->SetActorTransform(SpawnWorldTransform);
	}
}

void FSpawnAssetTool::RegenerateRandomRotationOffset()
{
	GetDesignerSettings()->RandomRotationX.RegenerateRandomValue();
	GetDesignerSettings()->RandomRotationY.RegenerateRandomValue();
	GetDesignerSettings()->RandomRotationZ.RegenerateRandomValue();
}

FRotator FSpawnAssetTool::GetRandomRotationOffset() const
{
	return FRotator( // Pitch, Yaw, Roll = Y, Z, X.
		GetDesignerSettings()->RandomRotationY.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotationZ.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotationX.GetCurrentRandomValue()
	);
}

void FSpawnAssetTool::RegenerateRandomScale()
{
	GetDesignerSettings()->RandomScaleX.RegenerateRandomValue();
	GetDesignerSettings()->RandomScaleY.RegenerateRandomValue();
	GetDesignerSettings()->RandomScaleZ.RegenerateRandomValue();
}

FVector FSpawnAssetTool::GetRandomScale() const
{
	return FVector(
		GetDesignerSettings()->RandomScaleX.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomScaleY.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomScaleZ.GetCurrentRandomValue()
	);
}

FRotator FSpawnAssetTool::GetDesignerActorRotation()
{
	FVector MouseDirection;
	float MouseDistance;
	(CursorPlaneIntersectionWorldLocation - SpawnWorldTransform.GetLocation()).ToDirectionAndLength(MouseDirection, MouseDistance);

	// If the mouse is exactly at the CursorInputDownWorldTransform, which happens on mouse click down.
	if (MouseDirection.IsNearlyZero())
		MouseDirection = SpawnWorldTransform.GetRotation().GetForwardVector();

	FVector ForwardVector = GetDesignerSettings()->AxisToAlignWithCursor == EAxisType::None ? SpawnWorldTransform.GetRotation().GetForwardVector() : MouseDirection;
	FVector UpVector = SpawnWorldTransform.GetRotation().GetUpVector();

	// if they're almost same, we need to find arbitrary vector
	if (FMath::IsNearlyEqual(FMath::Abs(ForwardVector | UpVector), 1.f))
	{
		// make sure we don't ever pick the same as NewX
		UpVector = (FMath::Abs(ForwardVector.Z) < (1.f - KINDA_SMALL_NUMBER)) ? FVector(0, 0, 1.f) : FVector(1.f, 0, 0);
	}

	FVector RightVector = (UpVector ^ ForwardVector).GetSafeNormal();
	UpVector = ForwardVector ^ RightVector;

	FVector SwizzledForwardVector = FVector::ZeroVector;
	FVector SwizzledRightVector = FVector::ZeroVector;
	FVector SwizzledUpVector = FVector::ZeroVector;

	switch (GetDesignerSettings()->AxisToAlignWithNormal)
	{
	case EAxisType::Forward:
		SwizzledForwardVector = UpVector;
		break;
	case EAxisType::Backward:
		SwizzledForwardVector = -UpVector;
		break;
	case EAxisType::Right:
		SwizzledRightVector = UpVector;
		break;
	case EAxisType::Left:
		SwizzledRightVector = -UpVector;
		break;
	case EAxisType::Down:
		SwizzledUpVector = -UpVector;
		break;
	default: // Axis type none or up
		SwizzledUpVector = UpVector;
		break;
	}

	switch (GetDesignerSettings()->AxisToAlignWithCursor)
	{
	case EAxisType::Backward:
		SwizzledForwardVector = -ForwardVector;
		break;
	case EAxisType::Right:
		SwizzledRightVector = ForwardVector;
		break;
	case EAxisType::Left:
		SwizzledRightVector = -ForwardVector;
		break;
	case EAxisType::Up:
		SwizzledUpVector = ForwardVector;
		break;
	case EAxisType::Down:
		SwizzledUpVector = -ForwardVector;
		break;
	default: // Axis type none or forward
		SwizzledForwardVector = ForwardVector;
		break;
	}

	bool bIsForwardVectorSet = !SwizzledForwardVector.IsNearlyZero();
	bool bIsRightVectorSet = !SwizzledRightVector.IsNearlyZero();
	bool bIsUpVectorSet = !SwizzledUpVector.IsNearlyZero();

	FRotator DesignerActorRotation;

	if (!bIsForwardVectorSet && bIsRightVectorSet && bIsUpVectorSet)
	{
		DesignerActorRotation = FRotationMatrix::MakeFromZY(SwizzledUpVector, SwizzledRightVector).Rotator();
	}
	else if (!bIsRightVectorSet && bIsForwardVectorSet && bIsUpVectorSet)
	{
		DesignerActorRotation = FRotationMatrix::MakeFromZX(SwizzledUpVector, SwizzledForwardVector).Rotator();
	}
	else if (!bIsUpVectorSet && bIsForwardVectorSet && bIsRightVectorSet)
	{
		DesignerActorRotation = FRotationMatrix::MakeFromXY(SwizzledForwardVector, SwizzledRightVector).Rotator();
	}
	else
	{
		// Default rotation of everything else fails
		DesignerActorRotation = FMatrix(ForwardVector, RightVector, UpVector, FVector::ZeroVector).Rotator();
		UE_LOG(LogDesigner, Warning, TEXT("Falling back to default rotation."));
	}

	// Apply the generated random rotation offset if the user has set the bApplyRandomRotation setting
	if (GetDesignerSettings()->bApplyRandomRotation)
	{
		DesignerActorRotation = FRotator(DesignerActorRotation.Quaternion() * GetRandomRotationOffset().Quaternion());
	}

	// Snap the axes to the grid if the user has set bSnapToGridRotation
	FRotator SpawnRotationSnapped = DesignerActorRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	DesignerActorRotation.Roll = GetDesignerSettings()->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : DesignerActorRotation.Roll;
	DesignerActorRotation.Pitch = GetDesignerSettings()->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : DesignerActorRotation.Pitch;
	DesignerActorRotation.Yaw = GetDesignerSettings()->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : DesignerActorRotation.Yaw;

	return DesignerActorRotation;
}