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

	UStaticMesh* StaticMesh = nullptr;
	if (!IsRunningCommandlet())
	{
		UMaterialInterface* SpawnVisualizerMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Designer/MI_SpawnVisualizer.MI_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(SpawnVisualizerMaterial != nullptr);
		SpawnVisualizerMID = UMaterialInstanceDynamic::Create(SpawnVisualizerMaterial, GetTransientPackage());
		check(SpawnVisualizerMID != nullptr);
		StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Designer/SM_SpawnVisualizer.SM_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(StaticMesh != nullptr);
	}

	SpawnVisualizerComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SpawnVisualizerComponent"));
	SpawnVisualizerComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SpawnVisualizerComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SpawnVisualizerComponent->SetStaticMesh(StaticMesh);
	SpawnVisualizerComponent->SetMaterial(0, SpawnVisualizerMID);
	SpawnVisualizerComponent->SetAbsolute(true, true, true);
	SpawnVisualizerComponent->CastShadow = false;
}

FDesignerEdMode::~FDesignerEdMode()
{

}

void FDesignerEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Call parent implementation
	FEdMode::AddReferencedObjects(Collector);

	Collector.AddReferencedObject(DesignerSettings);
	Collector.AddReferencedObject(SpawnVisualizerComponent);
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

	SpawnVisualizerComponent->SetVisibility(true);
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

	SpawnVisualizerComponent->UnregisterComponent();

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

void FDesignerEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);
	
	if (SpawnedDesignerActor)
	{
		FVector Axis1, Axis2;
		SpawnTracePlane.FindBestAxisVectors(Axis1, Axis2);
		FVector Direction;
		float Length;
		(MousePlaneWorldLocation - MouseDownWorldTransform.GetLocation()).ToDirectionAndLength(Direction, Length);
		DrawCircle(PDI, MouseDownWorldTransform.GetLocation(), Axis1, Axis2, FLinearColor::Blue, Length, 32, 0);
		PDI->DrawLine(MouseDownWorldTransform.GetLocation(), MousePlaneWorldLocation, FLinearColor::Green, 1);
		PDI->DrawPoint(MousePlaneWorldLocation, FLinearColor::Red, 4, 2);
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
	bool bHandled = false;
	
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
			if (CanSpawnActor)
			{
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
						// If the native parent class can't be placed, neither can the blueprint.
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
						// Recalculate mouse down, if it fails, return.
						if (!RecalculateMouseDownWorldTransform(ViewportClient, Viewport))
							return bHandled;
						
						SpawnedDesignerActor = GEditor->UseActorFactory(ActorFactory, TargetAssetData, &MouseDownWorldTransform);
						DefaultDesignerActorExtent = SpawnedDesignerActor->CalculateComponentsBoundingBoxInLocalSpace(true).GetExtent();
												
						// Properly reset data.
						MousePlaneWorldLocation = MouseDownWorldTransform.GetLocation();
						SpawnTracePlane = FPlane();
						
						FTransform SpawnVisualizerTransform = FTransform();
						SpawnVisualizerTransform.SetLocation(MouseDownWorldTransform.GetLocation());
						SpawnVisualizerTransform.SetRotation(FRotationMatrix::MakeFromZX(MouseDownWorldTransform.GetRotation().GetUpVector(), FVector::ForwardVector).ToQuat());
						SpawnVisualizerTransform.SetScale3D(FVector::OneVector * 10000);
						SpawnVisualizerComponent->SetRelativeTransform(SpawnVisualizerTransform);

						if (!SpawnVisualizerComponent->IsRegistered())
						{
							SpawnVisualizerComponent->RegisterComponentWithWorld(ViewportClient->GetWorld());
						}

						UpdateDesignerActorTransform();

						bHandled = true;
					}
				}
			}
		}
		/** Left mouse button released */
		else if (Event == IE_Released)
		{
			SpawnedDesignerActor = nullptr;
			DefaultDesignerActorExtent = FVector::ZeroVector;

			if (SpawnVisualizerComponent->IsRegistered())
			{
				SpawnVisualizerComponent->UnregisterComponent();
			}

			bHandled = false;
		}
	}
	
	return bHandled;
}

bool FDesignerEdMode::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	bool bHandled = false;

	if (SpawnedDesignerActor == nullptr)
		return bHandled;
	
	RecalculateMouseSpawnTracePlaneWorldLocation(ViewportClient, Viewport);

	UpdateDesignerActorTransform();

	bHandled = true;

	return bHandled;
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
	return true;
}

bool FDesignerEdMode::UsesToolkits() const
{
	return true;
}

bool FDesignerEdMode::RecalculateMouseDownWorldTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	FTransform NewMouseDownTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector);

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

	if (ActorPositionTraceResult.HitActor == nullptr)
	{
		UE_LOG(LogDesigner, Warning, TEXT("Mouse click down did not hit anything."));
		return false;
	}
	
	NewMouseDownTransform.SetLocation(ActorPositionTraceResult.Location);

	FRotator MouseDownWorldRotation = FRotationMatrix::MakeFromZX(DesignerSettings->AxisToAlignWithNormal == EAxisType::None ? FVector::UpVector : ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();

	FRotator SpawnRotationSnapped = MouseDownWorldRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	MouseDownWorldRotation.Roll = DesignerSettings->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : MouseDownWorldRotation.Roll;
	MouseDownWorldRotation.Pitch = DesignerSettings->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : MouseDownWorldRotation.Pitch;
	MouseDownWorldRotation.Yaw = DesignerSettings->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : MouseDownWorldRotation.Yaw;
	NewMouseDownTransform.SetRotation(MouseDownWorldRotation.Quaternion());

	MouseDownWorldTransform = NewMouseDownTransform;

	return true;
}

bool FDesignerEdMode::RecalculateMouseSpawnTracePlaneWorldLocation(FEditorViewportClient* ViewportClient, FViewport* Viewport)
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
	
	SpawnTracePlane = FPlane(MouseDownWorldTransform.GetLocation(), MouseDownWorldTransform.GetRotation().GetUpVector());
	MousePlaneWorldLocation = FMath::LinePlaneIntersection(TraceStartLocation, TraceEndLocation, SpawnTracePlane);

	return true;
}

void FDesignerEdMode::UpdateDesignerActorTransform()
{
	FTransform NewDesignerActorTransform = MouseDownWorldTransform;

	FVector Direction;
	float Length;
	(MousePlaneWorldLocation - MouseDownWorldTransform.GetLocation()).ToDirectionAndLength(Direction, Length);
	
	FVector NewScale = FVector(Length / DefaultDesignerActorExtent.X);
	if (NewScale.ContainsNaN())
	{
		NewScale = FVector::OneVector;
		UE_LOG(LogDesigner, Warning, TEXT("New scale contained NaN, so it is set to one. DefaultDesignerActorExtent = %s."), *DefaultDesignerActorExtent.ToString());
	}

	// Make sure the scale won't be to close to 0.
	NewScale.X = FMath::Max(NewScale.X, 0.0001F);
	NewScale.Y = FMath::Max(NewScale.Y, 0.0001F);
	NewScale.Z = FMath::Max(NewScale.Z, 0.0001F);

	NewDesignerActorTransform.SetScale3D(NewScale);

	FRotator DesignerActorRotation = FRotator();

	//switch (DesignerSettings->AxisToAlignWithNormal)
	//{
	//case EAxisType::None:
	//	// Allow random rotation on x, y and z axis.		
	//	break;
	//case EAxisType::Forward:
	//	DesignerActorRotation = FRotationMatrix::MakeFromXZ(ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
	//	// Allow random rotation on x axis.
	//	break;
	//case EAxisType::Backward:
	//	DesignerActorRotation = FRotationMatrix::MakeFromXZ(-ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
	//	// Allow random rotation on x axis.
	//	break;
	//case EAxisType::Right:
	//	DesignerActorRotation = FRotationMatrix::MakeFromYZ(ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
	//	// Allow random rotation on y axis.
	//	break;
	//case EAxisType::Left:
	//	DesignerActorRotation = FRotationMatrix::MakeFromYZ(-ActorPositionTraceResult.SurfaceNormal, FVector::UpVector).Rotator();
	//	// Allow random rotation on y axis.
	//	break;
	//case EAxisType::Up:
	//	DesignerActorRotation = FRotationMatrix::MakeFromZX(ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();
	//	// Allow random rotation on z axis.
	//	break;
	//case EAxisType::Down:
	//	DesignerActorRotation = FRotationMatrix::MakeFromZX(-ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();
	//	// Allow random rotation on z axis.
	//	break;
	//default:
	//	break;
	//}

	DesignerActorRotation = FRotationMatrix::MakeFromZX(MouseDownWorldTransform.Rotator().Quaternion().GetUpVector(), Direction).Rotator();
	
	FRotator SpawnRotationSnapped = DesignerActorRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	DesignerActorRotation.Roll = DesignerSettings->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : DesignerActorRotation.Roll;
	DesignerActorRotation.Pitch = DesignerSettings->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : DesignerActorRotation.Pitch;
	DesignerActorRotation.Yaw = DesignerSettings->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : DesignerActorRotation.Yaw;

	NewDesignerActorTransform.SetRotation(DesignerActorRotation.Quaternion());

	SpawnedDesignerActor->SetActorTransform(NewDesignerActorTransform);
	SpawnedDesignerActor->AddActorWorldOffset(DesignerSettings->SpawnLocationOffsetWorld);
	SpawnedDesignerActor->AddActorLocalOffset(DesignerSettings->SpawnLocationOffsetRelative);
}

void FDesignerEdMode::SpawnVisualizerMaterialData(FVector MouseLocationWorld)
{
	FLinearColor CursorData = FLinearColor(MouseLocationWorld.X, MouseLocationWorld.Y, MouseLocationWorld.Z, 0);
	const FName CursorDataParameterName("CursorData");
	SpawnVisualizerMID->SetVectorParameterValue(CursorDataParameterName, CursorData);
}
