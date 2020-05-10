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

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

FSpawnAssetTool::FSpawnAssetTool(UDesignerSettings* DesignerSettings)
{
	this->DesignerSettings = DesignerSettings;

	UStaticMesh* PlaneStaticMesh = nullptr;

	if (!IsRunningCommandlet())
	{
		UMaterialInterface* SpawnVisualizerMaterial = LoadObject<UMaterialInterface>(GetDesignerSettings(), TEXT("/Designer/MI_SpawnVisualizer.MI_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(SpawnVisualizerMaterial != nullptr);

		SpawnVisualizerMID = UMaterialInstanceDynamic::Create(SpawnVisualizerMaterial, GetDesignerSettings());
		check(SpawnVisualizerMID != nullptr);

		PlaneStaticMesh = LoadObject<UStaticMesh>(GetDesignerSettings(), TEXT("/Designer/SM_SpawnVisualizer.SM_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(PlaneStaticMesh != nullptr);

		PreviewActorMaterial = LoadObject<UMaterialInterface>(GetDesignerSettings(), TEXT("/Designer/MI_PreviewActor.MI_PreviewActor"), nullptr, LOAD_None, nullptr);
		check(PreviewActorMaterial != nullptr);

		PreviewActorPulsingMaterial = LoadObject<UMaterialInterface>(GetDesignerSettings(), TEXT("/Designer/MI_PreviewActorPulsing.MI_PreviewActorPulsing"), nullptr, LOAD_None, nullptr);
		check(PreviewActorPulsingMaterial != nullptr);
	}

	SpawnPlaneComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SpawnVisualizerComponent"));
	SpawnPlaneComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SpawnPlaneComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SpawnPlaneComponent->SetStaticMesh(PlaneStaticMesh);
	SpawnPlaneComponent->SetMaterial(0, SpawnVisualizerMID);
	SpawnPlaneComponent->SetAbsolute(true, true, true);
	SpawnPlaneComponent->CastShadow = false;

	ControlledSpawnedActor = nullptr;
	ReleasedSpawnedActor = nullptr;
}

FSpawnAssetTool::~FSpawnAssetTool()
{

}

void FSpawnAssetTool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DesignerSettings);
	Collector.AddReferencedObject(SpawnPlaneComponent);
}

FString FSpawnAssetTool::GetName() const
{
	return TEXT("SpawnAssetTool");
}

void FSpawnAssetTool::EnterTool()
{
	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::EnterTool"));

	SetToolActive(false);
}

void FSpawnAssetTool::ExitTool()
{
	UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::ExitTool"));

	SetToolActive(false);
}

bool FSpawnAssetTool::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	// While the tool is active no selection is allowed.
	return !IsToolActive;
}

bool FSpawnAssetTool::MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	// Make sure we are in full control of the mouse behavior when the tool is active.
	return IsToolActive;
}

bool FSpawnAssetTool::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	// Make sure we are in full control of the mouse behavior when the tool is active.
	return IsToolActive;
}

// Called when no mouse button is down, so can be used for the preview asset when the user holds down alt.
bool FSpawnAssetTool::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	if (IsToolActive && !ControlledSpawnedActor)
	{
		if (PreviewActorArray.Num() == 0)
		{
			RefreshPreviewActors();
		}

		RecalculateSpawnTransform(ViewportClient, Viewport);

		// Update the cursor plane world location to be the same as the spawn location so the rotation calculation is done properly.
		CursorPlaneIntersectionWorldLocation = SpawnWorldTransform.GetLocation();

		UpdatePreviewActorTransform();
	}

	return IsToolActive;
}

bool FSpawnAssetTool::ReceivedFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	// Make sure we are in full control when the tool is active.
	return IsToolActive;
}

bool FSpawnAssetTool::LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{	
	SetToolActive(false);
	return FDesignerTool::LostFocus(ViewportClient, Viewport);
}

// Called when a mouse button is down in viewport, so can be used for when the user is placing an asset.
bool FSpawnAssetTool::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 InMouseX, int32 InMouseY)
{
	bool bHandled = IsToolActive;

	if (ControlledSpawnedActor != nullptr)
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
	// Make sure we are in full control when the tool is active.
	return IsToolActive;
}

bool FSpawnAssetTool::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	// Make sure we are in full control when the tool is active.
	return IsToolActive;
}

bool FSpawnAssetTool::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = false;

	if (Key == EKeys::LeftControl || Key == EKeys::RightControl)
	{
		if (Event == IE_Pressed && !IsToolActive)
		{
			ViewportClient->SetRealtimeOverride(true, LOCTEXT("RealtimeOverrideMessage_DesignerMode", "Designer Mode"));

			SetToolActive(true);

			RefreshPlaceableAsset();

			// Calculate world location from mouse position.
			RecalculateSpawnTransform(ViewportClient, Viewport);

			// Update actors with new transformation data.
			UpdatePreviewActorTransform();

			bHandled = true;
		}
		else if (Event == IE_Released && IsToolActive)
		{
			bHandled = true;

			ViewportClient->RemoveRealtimeOverride();
			GEditor->RedrawAllViewports(false);

			SetToolActive(false);
		}
	}

	// Randomize the object again if right mouse button is pressed in this mode.
	if (Key == EKeys::RightMouseButton && Event == IE_Pressed && IsToolActive)
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey: Regenerate random data for spawned actor."));

		RegenerateRandomRotationOffset();
		RegenerateRandomScale();

		if (PreviewActorArray.Num() > 0)
		{
			UpdatePreviewActorTransform();
		}

		if (ControlledSpawnedActor != nullptr)
		{
			UpdateSpawnedActorTransform();
		}

		// Instantly update the material else it might be out of date when the object is rescaled.
		UpdateSpawnVisualizerMaterialParameters();

		bHandled = true;
	}

	if (Key == EKeys::LeftMouseButton && Event == IE_Pressed && IsToolActive)
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool::InputKey: Spawn selected asset."));
		bHandled = true;

		if (TargetAssetDataToSpawn.GetAsset() != nullptr)
		{
			UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAssetDataToSpawn.GetAsset());
			if (ActorFactory)
			{
				// Recalculate mouse down, if it fails, return.
				if (!RecalculateSpawnTransform(ViewportClient, Viewport))
					return bHandled;

				DestroyPreviewActors();

				ControlledSpawnedActor = GEditor->UseActorFactory(ActorFactory, TargetAssetDataToSpawn, &SpawnWorldTransform);

				DefaultSpawnedActorExtent = ControlledSpawnedActor->CalculateComponentsBoundingBoxInLocalSpace(true).GetExtent();

				// Properly reset data.
				CursorPlaneIntersectionWorldLocation = SpawnWorldTransform.GetLocation();
				SpawnTracePlane = FPlane();

				FTransform SpawnVisualizerTransform = SpawnWorldTransform;
				SpawnVisualizerTransform.SetScale3D(FVector(10000));
				SpawnPlaneComponent->SetRelativeTransform(SpawnVisualizerTransform);

				RegisterSpawnPlane(ViewportClient);
				UpdateSpawnedActorTransform();
				UpdateSpawnVisualizerMaterialParameters();

				bHandled = true;
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
		bHandled = true;

		// Regenerate random data for next actor but only if the current actor has actually been spawned.
		if (ControlledSpawnedActor != nullptr)
		{
			RefreshPlaceableAsset();
		}

		DefaultSpawnedActorExtent = FVector::ZeroVector;

		DestroyPreviewActors();
		UnregisterSpawnPlane();

		ReleaseControlledActor();
	}

	return bHandled;
}

// This is called when the left or right mouse button is pressed twice?!?!?!
bool FSpawnAssetTool::StartModify()
{
	// If user double clicks mouse button while in preview mode it should still regenerate the random.
	if (PreviewActorArray.Num() > 0)
	{
		RegenerateRandomRotationOffset();
		RegenerateRandomScale();
		return true;
	}

	return false;
}

bool FSpawnAssetTool::EndModify()
{
	return false;
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

		PreviousSelection.Empty();

		if (GEditor != nullptr)
		{
			// Store selection in case the user doesn't spawn an object we go to the previous selection.
			if (GEditor->GetSelectedActors() != nullptr)
			{
				GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(PreviousSelection);
				GEditor->SelectNone(false, true, false);
			}

			// Deselect all actors.
			GEditor->SelectNone(true, true, false);
		}

		// We have to set the tool active after doing all the selection related stuff, because selection is disabled while the tool is active.
		IsToolActive = true;
	}
	else
	{
		UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Setting tool inactive."));
		DestroyPreviewActors();		
		UnregisterSpawnPlane();
		ReleaseControlledActor();

		// We have to deactivate the tool before we start messing with the selections, because selections are disabled while using the tool.
		IsToolActive = false;

		if (GEditor != nullptr)
		{
			GEditor->SelectNone(false, true, false);
			if (ReleasedSpawnedActor != nullptr)
			{
				UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Select newly spawned asset."));
				GEditor->SelectActor(ReleasedSpawnedActor, true, true, true, true);
			}
			else // Restore previous selection.
			{
				UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: Selecting %d previous assets."), PreviousSelection.Num());
				for (AActor* PreviousSelectedActor : PreviousSelection)
				{
					if (PreviousSelectedActor != nullptr)
					{
						GEditor->SelectActor(PreviousSelectedActor, true, true, true, true);
					}
				}
			}
		}

		ControlledSpawnedActor = nullptr;
		ReleasedSpawnedActor = nullptr;
	}
}

void FSpawnAssetTool::SetAllMaterialsForActor(AActor* Actor, UMaterialInterface* Material)
{
	if (Actor != nullptr && Material != nullptr)
	{
		TArray<UPrimitiveComponent*> PrimitiveComponentArray;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponentArray, true);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponentArray)
		{
			if (PrimitiveComponent != nullptr)
			{
				int32 MaterialCount = PrimitiveComponent->GetNumMaterials();
				for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; MaterialIndex++)
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
	
	UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAssetDataToSpawn.GetAsset());
	if (TargetAssetDataToSpawn.GetClass() != nullptr)
	{
		PreviewActor = SpawnPreviewActorFromFactory(ActorFactory, TargetAssetDataToSpawn, &SpawnWorldTransform, RF_Transient);
		if (IsValid(PreviewActor) && PreviewActor->IsValidLowLevel())
		{
			PreviewActorArray.Add(PreviewActor);
			PreviewActor->SetActorLabel("DesignerPreviewActor");
			SetAllMaterialsForActor(PreviewActor, PreviewActorMaterial);
		}

		if (DesignerSettings->bScaleBoundsTowardsCursor)
		{
			PreviewActorPulsing = SpawnPreviewActorFromFactory(ActorFactory, TargetAssetDataToSpawn, &SpawnWorldTransform, RF_Transient);
		
			if (IsValid(PreviewActorPulsing) && PreviewActorPulsing->IsValidLowLevel())
			{
				PreviewActorArray.Add(PreviewActorPulsing);
				PreviewActorPulsing->SetActorLabel("DesignerPreviewActorPulsing");
				SetAllMaterialsForActor(PreviewActorPulsing, PreviewActorPulsingMaterial);

			}
		}
	}
}

void FSpawnAssetTool::DestroyPreviewActors()
{
	for (AActor* Actor : PreviewActorArray)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy(false, true);
		}
	}

	PreviewActorArray.Empty();
	PreviewActor = nullptr;
	PreviewActorPulsing = nullptr;
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

void FSpawnAssetTool::RefreshPlaceableAsset()
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
		
	// Pick random asset to spawn.
	if (PlaceableSelectedAssets.Num() > 0)
	{
		if (PlaceableSelectedAssets.Num() == 1)
		{
			TargetAssetDataToSpawn = PlaceableSelectedAssets[0];
		}
		else
		{
			// Remove previous asset, so a random selected asset is always a new one.
			PlaceableSelectedAssets.Remove(TargetAssetDataToSpawn);
			TargetAssetDataToSpawn = PlaceableSelectedAssets[FMath::RandRange(0, PlaceableSelectedAssets.Num() - 1)];
		}
	}

	if (TargetAssetDataToSpawn.GetAsset() != nullptr)
	{
		UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAssetDataToSpawn.GetAsset());
		if (ActorFactory != nullptr)
		{
			// Create preview actors.
			RefreshPreviewActors();

			// Generate random data.
			RegenerateRandomRotationOffset();
			RegenerateRandomScale();
		}
		else
		{
			UE_LOG(LogDesigner, Log, TEXT("SpawnAssetTool: No actor factory for object"));
		}
	}
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
	}

	return bPlaceable;
}

bool FSpawnAssetTool::UpdateSpawnVisualizerMaterialParameters()
{
	if (ControlledSpawnedActor != nullptr && SpawnVisualizerMID != nullptr)
	{
		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorInputDownWorldLocation"), FLinearColor(SpawnWorldTransform.GetLocation()));

		FVector Extent = DefaultSpawnedActorExtent * ControlledSpawnedActor->GetActorScale3D();
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
	
	// Trace world, ignore preview actors.
	const FActorPositionTraceResult TraceResult = FActorPositioning::TraceWorldForPositionWithDefault(Cursor, *View, &PreviewActorArray); 

	// For some reason the state is default when it fails to hit anything.
	if (TraceResult.State == FActorPositionTraceResult::Default)
	{
		return false;
	}

	NewSpawnTransform.SetLocation(TraceResult.Location);

	FVector RotationUpVector = GetDesignerSettings()->AxisToAlignWithNormal == EAxisType::None ? FVector::UpVector : TraceResult.SurfaceNormal;
	FRotator CursorWorldRotation = FRotationMatrix::MakeFromZX(RotationUpVector, FVector::ForwardVector).Rotator();

	FRotator SpawnRotationSnapped = CursorWorldRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	CursorWorldRotation.Roll = GetDesignerSettings()->SnapRotationToGrid.X ? SpawnRotationSnapped.Roll : CursorWorldRotation.Roll;
	CursorWorldRotation.Pitch = GetDesignerSettings()->SnapRotationToGrid.Y ? SpawnRotationSnapped.Pitch : CursorWorldRotation.Pitch;
	CursorWorldRotation.Yaw = GetDesignerSettings()->SnapRotationToGrid.Z ? SpawnRotationSnapped.Yaw : CursorWorldRotation.Yaw;
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

void FSpawnAssetTool::UpdatePreviewActorTransform()
{
	FTransform NewSpawnedActorTransform = SpawnWorldTransform;
	NewSpawnedActorTransform.SetScale3D(GetSpawnActorScale());
	NewSpawnedActorTransform.SetRotation(GetSpawnActorRotation().Quaternion());
	
	FVector RelativeLocationOffset = GetDesignerSettings()->RelativeLocationOffset;
	if (GetDesignerSettings()->bScaleRelativeLocationOffset)
	{
		RelativeLocationOffset *= GetSpawnActorScale();
	}

	FVector WorldLocationOffset = GetDesignerSettings()->WorldLocationOffset;
	if (GetDesignerSettings()->bScaleWorldLocationOffset)
	{
		WorldLocationOffset *= GetSpawnActorScale();
	}

	if (PreviewActor != nullptr)
	{
		PreviewActor->SetActorTransform(NewSpawnedActorTransform);
		PreviewActor->AddActorLocalOffset(RelativeLocationOffset);
		PreviewActor->AddActorWorldOffset(WorldLocationOffset);
	}

	if (PreviewActorPulsing != nullptr)
	{
		PreviewActorPulsing->SetActorTransform(NewSpawnedActorTransform);
		PreviewActorPulsing->AddActorLocalOffset(RelativeLocationOffset);
		PreviewActorPulsing->AddActorWorldOffset(WorldLocationOffset);
	}

}

void FSpawnAssetTool::UpdateSpawnedActorTransform()
{
	FTransform NewSpawnedActorTransform = SpawnWorldTransform;

	FVector CursorDirection;
	float CursorDistance;
	(CursorPlaneIntersectionWorldLocation - SpawnWorldTransform.GetLocation()).ToDirectionAndLength(CursorDirection, CursorDistance);

	FVector NewScale = GetSpawnActorScale();
	// If the object also scales towards the mouse we use the randoms scale as a ratio
	if (GetDesignerSettings()->bScaleBoundsTowardsCursor)
	{
		NewScale /= FMath::Max(NewScale.X, FMath::Max(NewScale.Y, NewScale.Z));
	}

	if (GetDesignerSettings()->bScaleBoundsTowardsCursor)
	{
		EAxisType PositiveAxis = GetDesignerSettings()->GetPositiveAxisToAlignWithCursor();
		float BoundsUsedForScale;
		if (PositiveAxis == EAxisType::Forward)
			BoundsUsedForScale = DefaultSpawnedActorExtent.X;
		else if (PositiveAxis == EAxisType::Right)
			BoundsUsedForScale = DefaultSpawnedActorExtent.Y;
		else if (PositiveAxis == EAxisType::Up)
			BoundsUsedForScale = DefaultSpawnedActorExtent.Z;
		else
			BoundsUsedForScale = FMath::Max(DefaultSpawnedActorExtent.X, DefaultSpawnedActorExtent.Y);

		NewScale *= FVector(CursorDistance / BoundsUsedForScale);
	}

	if (NewScale.ContainsNaN())
	{
		NewScale = FVector(GetDesignerSettings()->MinimalScale);
		UE_LOG(LogDesigner, Warning, TEXT("New scale contained NaN, so it is set to the minimal scale. DefaultDesignerActorExtent = %s."), *DefaultSpawnedActorExtent.ToString());
	}

	// Clamp the scale by minimum scale value.
	FVector ClampedAbsoluteNewScale = NewScale.GetAbs();
	ClampedAbsoluteNewScale = ClampedAbsoluteNewScale.ComponentMax(FVector(GetDesignerSettings()->MinimalScale));
	NewScale = NewScale.GetSignVector() * ClampedAbsoluteNewScale;

	NewSpawnedActorTransform.SetScale3D(NewScale);

	NewSpawnedActorTransform.SetRotation(GetSpawnActorRotation().Quaternion());
	
	FVector RelativeLocationOffset = GetDesignerSettings()->RelativeLocationOffset;
	if (GetDesignerSettings()->bScaleRelativeLocationOffset)
	{
		RelativeLocationOffset *= NewScale;
	}

	FVector WorldLocationOffset = GetDesignerSettings()->WorldLocationOffset;
	if (GetDesignerSettings()->bScaleWorldLocationOffset)
	{
		WorldLocationOffset *= NewScale;
	}

	if (ControlledSpawnedActor != nullptr)
	{
		ControlledSpawnedActor->SetActorTransform(NewSpawnedActorTransform);
		ControlledSpawnedActor->AddActorLocalOffset(RelativeLocationOffset);
		ControlledSpawnedActor->AddActorWorldOffset(WorldLocationOffset);
	}
}

void FSpawnAssetTool::RegenerateRandomRotationOffset()
{
	GetDesignerSettings()->RandomRotation.RegenerateRandomValue();
}

FRotator FSpawnAssetTool::GetRandomRotationOffset() const
{
	return FRotator( // Pitch, Yaw, Roll = Y, Z, X.
		GetDesignerSettings()->RandomRotation.Y.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotation.Z.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotation.X.GetCurrentRandomValue()
	);
}

void FSpawnAssetTool::RegenerateRandomScale()
{
	GetDesignerSettings()->RandomScale.RegenerateRandomValue();
}

FVector FSpawnAssetTool::GetSpawnActorScale() const
{
	if (GetDesignerSettings()->bApplyRandomScale)
	{
		return GetDesignerSettings()->RandomScale.GetCurrentRandomValue();
	}
	else
	{
		return FVector::OneVector;
	}
}

FRotator FSpawnAssetTool::GetSpawnActorRotation()
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
	}

	// Apply the generated random rotation offset if the user has set the bApplyRandomRotation setting
	if (GetDesignerSettings()->bApplyRandomRotation)
	{
		DesignerActorRotation = FRotator(DesignerActorRotation.Quaternion() * GetRandomRotationOffset().Quaternion());
	}

	// Snap the axes to the grid if the user has set bSnapToGridRotation
	FRotator SpawnRotationSnapped = DesignerActorRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	DesignerActorRotation.Roll = GetDesignerSettings()->SnapRotationToGrid.X ? SpawnRotationSnapped.Roll : DesignerActorRotation.Roll;
	DesignerActorRotation.Pitch = GetDesignerSettings()->SnapRotationToGrid.Y ? SpawnRotationSnapped.Pitch : DesignerActorRotation.Pitch;
	DesignerActorRotation.Yaw = GetDesignerSettings()->SnapRotationToGrid.Z ? SpawnRotationSnapped.Yaw : DesignerActorRotation.Yaw;

	return DesignerActorRotation;
}

void FSpawnAssetTool::RegisterSpawnPlane(FEditorViewportClient* InViewportClient)
{
	if (!SpawnPlaneComponent->IsRegistered())
	{
		SpawnPlaneComponent->RegisterComponentWithWorld(InViewportClient->GetWorld());
	}
}

void FSpawnAssetTool::UnregisterSpawnPlane()
{
	if (SpawnPlaneComponent->IsRegistered())
	{
		SpawnPlaneComponent->UnregisterComponent();
	}
}

void FSpawnAssetTool::ReleaseControlledActor()
{
	if (ControlledSpawnedActor)
	{
		ReleasedSpawnedActor = ControlledSpawnedActor;
		ControlledSpawnedActor = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE