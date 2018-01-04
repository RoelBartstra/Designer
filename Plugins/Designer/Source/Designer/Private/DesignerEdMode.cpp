//  Copyright 2018 Roel Bartstra.

//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :

//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.

//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "DesignerEdMode.h"
#include "DesignerEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

#include "DesignerModule.h"
#include "DesignerSettings.h"

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

const FEditorModeID FDesignerEdMode::EM_DesignerEdModeId = TEXT("EM_DesignerEdMode");

FDesignerEdMode::FDesignerEdMode()
{
	DesignerSettings = NewObject<UDesignerSettings>(GetTransientPackage(), TEXT("DesignerEdModeSettings"), RF_Transactional);
	DesignerSettings->SetParent(this);

	// Load resources and construct brush component
	UStaticMesh* StaticMesh = nullptr;
	if (!IsRunningCommandlet())
	{
		UMaterial* PlacementVisualizerMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Designer/M_PlacementVisualizer.M_PlacementVisualizer"), nullptr, LOAD_None, nullptr);
		PlacementVisualizerMID = UMaterialInstanceDynamic::Create(PlacementVisualizerMaterial, GetTransientPackage());
		check(PlacementVisualizerMID != nullptr);
		StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Designer/SM_PlacementVisualizer.SM_PlacementVisualizer"), nullptr, LOAD_None, nullptr);
		check(StaticMesh != nullptr);
	}

	PlacementVisualizerComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("PlacementVisualizerComponent"));
	PlacementVisualizerComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	PlacementVisualizerComponent->SetCollisionObjectType(ECC_WorldDynamic);
	PlacementVisualizerComponent->SetStaticMesh(StaticMesh);
	PlacementVisualizerComponent->SetMaterial(0, PlacementVisualizerMID);
	PlacementVisualizerComponent->SetAbsolute(true, true, true);
	PlacementVisualizerComponent->CastShadow = false;

	//bBrushTraceValid = false;
	//BrushLocation = FVector::ZeroVector;

	//// Get the default opacity from the material.
	//FName OpacityParamName("OpacityAmount");
	//BrushMID->GetScalarParameterValue(OpacityParamName, DefaultBrushOpacity);
}

FDesignerEdMode::~FDesignerEdMode()
{

}

void FDesignerEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Call parent implementation
	FEdMode::AddReferencedObjects(Collector);

	Collector.AddReferencedObject(DesignerSettings);
	Collector.AddReferencedObject(PlacementVisualizerComponent);
}

TSharedPtr<class FModeToolkit> FDesignerEdMode::GetToolkit()
{
	return Toolkit;
}

void FDesignerEdMode::Enter()
{
	FEdMode::Enter();

	CanSpawnActor = false;
	SpawnedDesignerActor = nullptr;
	
	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FDesignerEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	PlacementVisualizerComponent->SetVisibility(true);
}

void FDesignerEdMode::Exit()
{
	CanSpawnActor = false;
	SpawnedDesignerActor = nullptr;

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	PlacementVisualizerComponent->UnregisterComponent();

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

void FDesignerEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);
	
	if (SpawnedDesignerActor)
	{
		FVector Axis1, Axis2;
		PlacementPlane.FindBestAxisVectors(Axis1, Axis2);
		FVector Direction;
		float Length;
		(CursorPlaneHitLocation - DesignerActorTransformExcludingOffset.GetLocation()).ToDirectionAndLength(Direction, Length);
		DrawCircle(PDI, DesignerActorTransformExcludingOffset.GetLocation(), Axis1, Axis2, FLinearColor::Blue, Length, 32, 0);
		PDI->DrawLine(DesignerActorTransformExcludingOffset.GetLocation(), CursorPlaneHitLocation, FLinearColor::Green, 1);
		PDI->DrawPoint(CursorPlaneHitLocation, FLinearColor::Red, 4, 2);
	}
}

bool FDesignerEdMode::GetCursor(EMouseCursor::Type& OutCursor) const
{
	OutCursor = EMouseCursor::Crosshairs;
	return true;
}

bool FDesignerEdMode::LostFocus(FEditorViewportClient * ViewportClient, FViewport * Viewport)
{
	// Can not spawn actor any more after losing focus to make sure the user has to press ctrl to allow spawning actors.
	CanSpawnActor = false;
	SpawnedDesignerActor = nullptr;

	return FEdMode::LostFocus(ViewportClient, Viewport);
}

bool FDesignerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftControl || Key == EKeys::RightControl)
	{
		if (Event == IE_Pressed)
		{
			CanSpawnActor = true;
		}
		else if (Event == IE_Released)
		{
			CanSpawnActor = false;
		}
	}
	
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == IE_Pressed)
		{
			//UE_LOG(LogDesigner, Log, TEXT("LEFT MOUSE BUTTON PRESSED"));

			if (CanSpawnActor)
			{
				//UE_LOG(LogDesigner, Log, TEXT("TRY TO SPAWN ACTOR"));

				TArray<FAssetData> ContentBrowserSelections;
				GEditor->GetContentBrowserSelections(ContentBrowserSelections);

				UClass* SelectedClass = GEditor->GetSelectedObjects()->GetTop<UClass>();

				bool bPlaceable = true;
				TArray<FAssetData> SelectedAssets;
				AssetSelectionUtils::GetSelectedAssets(SelectedAssets);
				FAssetData TargetAssetData;

				if (TargetAssetData.GetClass() == UClass::StaticClass())
				{
					UClass* Class = Cast<UClass>(TargetAssetData.GetAsset());

					bPlaceable = AssetSelectionUtils::IsClassPlaceable(Class);
				}

				if (SelectedAssets.Num() > 0)
				{
					TargetAssetData = SelectedAssets.Top();
				}

				if (TargetAssetData.GetClass() == UClass::StaticClass())
				{
					UClass* Class = Cast<UClass>(TargetAssetData.GetAsset());

					bPlaceable = AssetSelectionUtils::IsClassPlaceable(Class);
				}
				else if (TargetAssetData.GetClass() == UBlueprint::StaticClass())
				{
					// For blueprints, attempt to determine placeability from its tag information

					const FName NativeParentClassTag = TEXT("NativeParentClass");
					const FName ClassFlagsTag = TEXT("ClassFlags");

					FString TagValue;

					if (TargetAssetData.GetTagValue(NativeParentClassTag, TagValue) && !TagValue.IsEmpty())
					{
						// If the native parent class can't be placed, neither can the blueprint

						UObject* Outer = nullptr;
						ResolveName(Outer, TagValue, false, false);
						UClass* NativeParentClass = FindObject<UClass>(ANY_PACKAGE, *TagValue);

						bPlaceable = AssetSelectionUtils::IsClassPlaceable(NativeParentClass);
					}

					if (bPlaceable && TargetAssetData.GetTagValue(ClassFlagsTag, TagValue) && !TagValue.IsEmpty())
					{
						// Check to see if this class is placeable from its class flags

						const int32 NotPlaceableFlags = CLASS_NotPlaceable | CLASS_Deprecated | CLASS_Abstract;
						uint32 ClassFlags = FCString::Atoi(*TagValue);

						bPlaceable = (ClassFlags & NotPlaceableFlags) == CLASS_None;
					}
				}

				UObject* TargetAsset = TargetAssetData.GetAsset();

				if (bPlaceable && IsValid(TargetAsset))
				{
					//UE_LOG(LogDesigner, Log, TEXT("ACTOR IS PLACABLE"));
					UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAsset);
					if (ActorFactory)
					{
						// Compile an array of selected actors
						FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
							Viewport,
							ViewportClient->GetScene(),
							ViewportClient->EngineShowFlags)
							.SetRealtimeUpdate(ViewportClient->IsRealtime()));
						// SceneView is deleted with the ViewFamily
						FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);

						FViewportCursorLocation MouseViewportRay(SceneView, ViewportClient, Viewport->GetMouseX(), Viewport->GetMouseY());

						FVector TraceStartLocation = MouseViewportRay.GetOrigin();
						FVector TraceDirection = MouseViewportRay.GetDirection();
						FVector TraceEndLocation = TraceStartLocation + TraceDirection * WORLD_MAX;
						if (ViewportClient->IsOrtho())
						{
							TraceStartLocation += -WORLD_MAX * TraceDirection;
						}

						const TArray<AActor*>* IgnoreActors = new TArray<AActor*>();

						FActorPositionTraceResult ActorPositionTraceResult = FActorPositioning::TraceWorldForPositionWithDefault(MouseViewportRay, *SceneView, IgnoreActors);

						if (ActorPositionTraceResult.HitActor != nullptr)
						{
							//UE_LOG(LogDesigner, Warning, TEXT("HIT: %s"), *ActorPositionTraceResult.HitActor->GetName());
						}
						else
						{
							UE_LOG(LogDesigner, Error, TEXT("HIT FAIL"));
							return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
						}
						
						DesignerActorTransformExcludingOffset = CalculateDesignerActorTransform(ActorPositionTraceResult);
						SpawnedDesignerActor = GEditor->UseActorFactory(ActorFactory, TargetAssetData, &DesignerActorTransformExcludingOffset);
						UpdateDesignerActorTransform();
						DefaultDesignerActorExtent = SpawnedDesignerActor->CalculateComponentsBoundingBoxInLocalSpace(true).GetExtent();
						
						// Properly reset data.
						CursorPlaneHitLocation = DesignerActorTransformExcludingOffset.GetLocation();
						PlacementPlane = FPlane();

						// SHOULD BE REPLACED WITH UPDATING THE ENTIRE TRANSFORM LIKE IT'S DONE IN CAPUTURED MOUSE MOVE.
						SpawnedDesignerActor->SetActorScale3D(FVector(KINDA_SMALL_NUMBER));
						
						FTransform PlacementVisualizerTransform = FTransform();
						PlacementVisualizerTransform.SetLocation(DesignerActorTransformExcludingOffset.GetLocation());
						PlacementVisualizerTransform.SetRotation(FRotationMatrix::MakeFromZX(SpawnedDesignerActor->GetActorUpVector(), FVector::ForwardVector).ToQuat());
						PlacementVisualizerTransform.SetScale3D(FVector::OneVector * 100);
						PlacementVisualizerComponent->SetRelativeTransform(PlacementVisualizerTransform);

						if (!PlacementVisualizerComponent->IsRegistered())
						{
							PlacementVisualizerComponent->RegisterComponentWithWorld(ViewportClient->GetWorld());
						}
					}
				}
			}
		}
		/** Left mouse button released */
		else if (Event == IE_Released)
		{
			SpawnedDesignerActor = nullptr;
			DefaultDesignerActorExtent = FVector::ZeroVector;

			if (PlacementVisualizerComponent->IsRegistered())
			{
				PlacementVisualizerComponent->UnregisterComponent();
			}
		}
	}
	
	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

bool FDesignerEdMode::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	if (SpawnedDesignerActor == nullptr)
		return false;

	// Compile an array of selected actors
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

	FVector PlaneNormal;
	switch (DesignerSettings->AxisToAlignWithNormal)
	{
	case EAxisType::None:
		PlaneNormal = FVector::UpVector;
		break;
	case EAxisType::Forward:
		PlaneNormal = DesignerActorTransformExcludingOffset.GetRotation().GetForwardVector();
		break;
	case EAxisType::Backward:
		PlaneNormal = -DesignerActorTransformExcludingOffset.GetRotation().GetForwardVector();
		break;
	case EAxisType::Right:
		PlaneNormal = DesignerActorTransformExcludingOffset.GetRotation().GetRightVector();
		break;
	case EAxisType::Left:
		PlaneNormal = -DesignerActorTransformExcludingOffset.GetRotation().GetRightVector();
		break;
	case EAxisType::Up:
		PlaneNormal = DesignerActorTransformExcludingOffset.GetRotation().GetUpVector();
		break;
	case EAxisType::Down:
		PlaneNormal = -DesignerActorTransformExcludingOffset.GetRotation().GetUpVector();
		break;
	default:
		PlaneNormal = FVector::UpVector;
		break;

	}
	
	PlacementPlane = FPlane(DesignerActorTransformExcludingOffset.GetLocation(), PlaneNormal);
	CursorPlaneHitLocation = FMath::LinePlaneIntersection(TraceStartLocation, TraceEndLocation, PlacementPlane);

	FVector Direction;
	float Length;
	(CursorPlaneHitLocation - DesignerActorTransformExcludingOffset.GetLocation()).ToDirectionAndLength(Direction, Length);
	
	DesignerActorTransformExcludingOffset.SetScale3D(FVector(Length / DefaultDesignerActorExtent.X));

	//SpawnedDesignerActor->SetActorScale3D(Scale);

	/*
	FRotator DesignerActorRotation = FRotator();

	switch (UISettings->AxisToAlignWithNormal)
	{
	case EAxisType::None:
		// Allow random rotation on x, y and z axis.		
		break;
	case EAxisType::Forward:
		DesignerActorRotation = FRotationMatrix::MakeFromXZ(ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on x axis.
		break;
	case EAxisType::Backward:
		DesignerActorRotation = FRotationMatrix::MakeFromXZ(-ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on x axis.
		break;
	case EAxisType::Right:
		DesignerActorRotation = FRotationMatrix::MakeFromYZ(ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on y axis.
		break;
	case EAxisType::Left:
		DesignerActorRotation = FRotationMatrix::MakeFromYZ(-ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on y axis.
		break;
	case EAxisType::Up:
		DesignerActorRotation = FRotationMatrix::MakeFromZX(ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();
		// Allow random rotation on z axis.
		break;
	case EAxisType::Down:
		DesignerActorRotation = FRotationMatrix::MakeFromZX(-ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();
		// Allow random rotation on z axis.
		break;
	default:
		break;
	}
	*/

	DesignerActorTransformExcludingOffset.SetRotation(FRotationMatrix::MakeFromZX(SpawnedDesignerActor->GetActorUpVector(), Direction).ToQuat());

	//FRotator PlacementVisualizerRotation = FRotationMatrix::MakeFromZX(PlaneNormal, SpawnedDesignerActor->GetActorForwardVector()).Rotator();
	//FTransform PlacementVisualizerTransform = FTransform(PlacementVisualizerRotation, SpawnedDesignerActor->GetActorLocation(), SpawnedDesignerActor->GetActorScale3D());
	//PlacementVisualizerComponent->SetRelativeTransform(PlacementVisualizerTransform);

	//PlacementVisualizerMaterialData(MousePlaneHitLocation);

	UpdateDesignerActorTransform();

	return FEdMode::CapturedMouseMove(ViewportClient, Viewport, MouseX, MouseY);
}

bool FDesignerEdMode::CreateDesignerActor(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	return false;
}

bool FDesignerEdMode::UsesTransformWidget() const
{
	// Don't show the default transformation widget.
	// TODO: ONLY SHOW IT WHEN WE ARE NOT ON IN ONE OF THE TOOLS.
	return false;
}

void FDesignerEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{

}

bool FDesignerEdMode::Select(AActor* InActor, bool bInSelected)
{
	return FEdMode::Select(InActor, bInSelected);
}

void FDesignerEdMode::ActorSelectionChangeNotify()
{

}

bool FDesignerEdMode::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	return FEdMode::IsSelectionAllowed(InActor, bInSelection);
}

bool FDesignerEdMode::UsesToolkits() const
{
	return true;
}

FTransform FDesignerEdMode::CalculateDesignerActorTransform(FActorPositionTraceResult ActorPositionTraceResult)
{
	FTransform DesignerActorTransform = FTransform();
	DesignerActorTransform.SetLocation(ActorPositionTraceResult.Location);

	FRotator DesignerActorRotation = FRotator();

	switch (DesignerSettings->AxisToAlignWithNormal)
	{
	case EAxisType::None:
		DesignerActorRotation = FRotationMatrix::MakeFromZX(FVector::UpVector, FVector::ForwardVector).Rotator();
		break;
	case EAxisType::Forward:
		DesignerActorRotation = FRotationMatrix::MakeFromXZ(ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on x axis.
		break;
	case EAxisType::Backward:
		DesignerActorRotation = FRotationMatrix::MakeFromXZ(-ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on x axis.
		break;
	case EAxisType::Right:
		DesignerActorRotation = FRotationMatrix::MakeFromYZ(ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on y axis.
		break;
	case EAxisType::Left:
		DesignerActorRotation = FRotationMatrix::MakeFromYZ(-ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
		// Allow random rotation on y axis.
		break;
	case EAxisType::Up:
		DesignerActorRotation = FRotationMatrix::MakeFromZX(ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();
		// Allow random rotation on z axis.
		break;
	case EAxisType::Down:
		DesignerActorRotation = FRotationMatrix::MakeFromZX(-ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();
		// Allow random rotation on z axis.
		break;
	default:
		break;
	}
	FRotator SpawnRotationSnapped = DesignerActorRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	DesignerActorRotation.Roll = DesignerSettings->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : DesignerActorRotation.Roll;
	DesignerActorRotation.Pitch = DesignerSettings->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : DesignerActorRotation.Pitch;
	DesignerActorRotation.Yaw = DesignerSettings->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : DesignerActorRotation.Yaw;
	DesignerActorTransform.SetRotation(DesignerActorRotation.Quaternion());

	return DesignerActorTransform;
}

void FDesignerEdMode::UpdateDesignerActorTransform()
{
	SpawnedDesignerActor->SetActorTransform(DesignerActorTransformExcludingOffset);
	SpawnedDesignerActor->AddActorWorldOffset(DesignerSettings->SpawnLocationOffsetWorld);
	SpawnedDesignerActor->AddActorLocalOffset(DesignerSettings->SpawnLocationOffsetRelative);
}

void FDesignerEdMode::PlacementVisualizerMaterialData(FVector MouseLocationWorld)
{
	FLinearColor CursorData = FLinearColor(MouseLocationWorld.X, MouseLocationWorld.Y, MouseLocationWorld.Z, 0);
	const FName CursorDataParameterName("CursorData");
	PlacementVisualizerMID->SetVectorParameterValue(CursorDataParameterName, CursorData);
}
