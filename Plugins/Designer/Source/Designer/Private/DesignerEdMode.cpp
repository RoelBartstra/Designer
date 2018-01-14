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

	bCanSpawnActor = false;
	ControlledActor = nullptr;
	
	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FDesignerEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	SpawnVisualizerComponent->SetVisibility(true);
}

void FDesignerEdMode::Exit()
{
	bCanSpawnActor = false;
	ControlledActor = nullptr;

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
	
	// Leave this here in case we use it again in the near future.
	//if (SpawnedDesignerActor)
	//{
		//FVector Axis1, Axis2;
		//SpawnTracePlane.FindBestAxisVectors(Axis1, Axis2);
		//FVector Direction;
		//float Length;
		//(CursorPlaneWorldLocation - CursorInputDownWorldTransform.GetLocation()).ToDirectionAndLength(Direction, Length);
		//float CircleThickness = Length * 0.1F;
		//DrawCircle(PDI, CursorInputDownWorldTransform.GetLocation(), Axis1, Axis2, FLinearColor::White, Length, 32, 0, CircleThickness, SDPG_Foreground, false);
		//DrawDirectionalArrow(PDI,)
		//PDI->DrawLine(CursorInputDownWorldTransform.GetLocation(), CursorPlaneWorldLocation, FLinearColor::Green, 1);
		//PDI->DrawPoint(CursorPlaneWorldLocation, FLinearColor::Red, 4, 2);
	//}
}

bool FDesignerEdMode::LostFocus(FEditorViewportClient * ViewportClient, FViewport * Viewport)
{
	// Can not spawn actor any more after losing focus to make sure the user has to press ctrl to allow spawning actors.
	bCanSpawnActor = false;
	ControlledActor = nullptr;

	return FEdMode::LostFocus(ViewportClient, Viewport);
}

bool FDesignerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	FEdMode::InputKey(ViewportClient, Viewport, Key, Event);

	bool bHandled = false;
	
	if (Key == EKeys::LeftControl || Key == EKeys::RightControl)
	{
		if (Event == IE_Pressed)
		{
			bCanSpawnActor = true;

			bHandled = true;
		}
		else if (Event == IE_Released)
		{
			bCanSpawnActor = false;

			bHandled = true;
		}

		if (Event == IE_Pressed && ControlledActor != nullptr)
		{
			RegenerateRandomRotationOffset();
			RegenerateRandomScale();

			UpdateDesignerActorTransform();

			bHandled = true;
		}
	}
	
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == IE_Pressed)
		{
			if (bCanSpawnActor)
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
					UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAsset);
					if (ActorFactory)
					{
						// Recalculate mouse down, if it fails, return.
						if (!RecalculateMouseDownWorldTransform(ViewportClient, Viewport))
							return bHandled;
						
						ControlledActor = GEditor->UseActorFactory(ActorFactory, TargetAssetData, &CursorInputDownWorldTransform);
						
						DefaultDesignerActorExtent = ControlledActor->CalculateComponentsBoundingBoxInLocalSpace(true).GetExtent();
						
						// Properly reset data.
						CursorPlaneWorldLocation = CursorInputDownWorldTransform.GetLocation();
						SpawnTracePlane = FPlane();
						
						FTransform SpawnVisualizerTransform = FTransform();
						SpawnVisualizerTransform.SetLocation(CursorInputDownWorldTransform.GetLocation());
						SpawnVisualizerTransform.SetRotation(FRotationMatrix::MakeFromZX(CursorInputDownWorldTransform.GetRotation().GetUpVector(), FVector::ForwardVector).ToQuat());
						SpawnVisualizerTransform.SetScale3D(FVector::OneVector * 10000);
						SpawnVisualizerComponent->SetRelativeTransform(SpawnVisualizerTransform);

						if (!SpawnVisualizerComponent->IsRegistered())
						{
							SpawnVisualizerComponent->RegisterComponentWithWorld(ViewportClient->GetWorld());
						}

						RegenerateRandomRotationOffset();

						RegenerateRandomScale();

						UpdateDesignerActorTransform();

						UpdateSpawnVisualizerMaterialParameters();

						bHandled = true;
					}
				}
			}
		}
		/** Left mouse button released */
		else if (Event == IE_Released)
		{
			if (ControlledActor != nullptr && FMath::IsNearlyZero(ControlledActor->GetActorScale3D().Size()))
			{
				ControlledActor->Destroy(false, false);
			}

			ControlledActor = nullptr;
			DefaultDesignerActorExtent = FVector::ZeroVector;

			if (SpawnVisualizerComponent->IsRegistered())
			{
				SpawnVisualizerComponent->UnregisterComponent();
			}

			// Makes sure we can click the transform widget after the user has spawned the actor.
			GEditor->RedrawLevelEditingViewports();

			bHandled = false;
		}
	}
	
	return bHandled;
}

bool FDesignerEdMode::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	bool bHandled = false;

	if (ControlledActor == nullptr)
		return bHandled;

	RecalculateMouseSpawnTracePlaneWorldLocation(ViewportClient, Viewport);

	UpdateDesignerActorTransform();

	UpdateSpawnVisualizerMaterialParameters();

	bHandled = true;

	return bHandled;
}

bool FDesignerEdMode::DisallowMouseDeltaTracking() const
{
	return ControlledActor != nullptr;
}

bool FDesignerEdMode::UsesTransformWidget() const
{
	// Only show transform widget when we are not spawning a new actor.
	return ControlledActor == nullptr;
}

bool FDesignerEdMode::UsesToolkits() const
{
	return true;
}

bool FDesignerEdMode::UpdateSpawnVisualizerMaterialParameters()
{
	if (SpawnVisualizerMID)
	{
		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorInputDownWorldLocation"), FLinearColor(CursorInputDownWorldTransform.GetLocation()));
		
		FVector Extent = DefaultDesignerActorExtent * ControlledActor->GetActorScale3D();
		EAxisType PositiveAxis = DesignerSettings->GetPositiveAxisToAlignWithCursor();
		float ActorRadius = PositiveAxis == EAxisType::Right ? Extent.Y : PositiveAxis == EAxisType::Up ? Extent.Z : Extent.X;
		
		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorPlaneWorldLocation"), FLinearColor(CursorPlaneWorldLocation.X, CursorPlaneWorldLocation.Y, CursorPlaneWorldLocation.Z, ActorRadius));

		FLinearColor ForwardVectorColor = PositiveAxis == EAxisType::Up ? FLinearColor::Blue : PositiveAxis == EAxisType::Right ? FLinearColor::Green : FLinearColor::Red;
		SpawnVisualizerMID->SetVectorParameterValue(FName("ForwardAxisColor"), ForwardVectorColor);

		return true;
	}

	return false;
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

	FRotator MouseDownWorldRotation = FRotationMatrix::MakeFromZX(GetDesignerSettings()->AxisToAlignWithNormal == EAxisType::None ? FVector::UpVector : ActorPositionTraceResult.SurfaceNormal, FVector::ForwardVector).Rotator();

	FRotator SpawnRotationSnapped = MouseDownWorldRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	MouseDownWorldRotation.Roll = GetDesignerSettings()->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : MouseDownWorldRotation.Roll;
	MouseDownWorldRotation.Pitch = GetDesignerSettings()->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : MouseDownWorldRotation.Pitch;
	MouseDownWorldRotation.Yaw = GetDesignerSettings()->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : MouseDownWorldRotation.Yaw;
	NewMouseDownTransform.SetRotation(MouseDownWorldRotation.Quaternion());

	CursorInputDownWorldTransform = NewMouseDownTransform;

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
	
	SpawnTracePlane = FPlane(CursorInputDownWorldTransform.GetLocation(), CursorInputDownWorldTransform.GetRotation().GetUpVector());
	CursorPlaneWorldLocation = FMath::LinePlaneIntersection(TraceStartLocation, TraceEndLocation, SpawnTracePlane);

	return true;
}

void FDesignerEdMode::UpdateDesignerActorTransform()
{
	FTransform NewDesignerActorTransform = CursorInputDownWorldTransform;

	FVector CursorDirection;
	float CursorDistance;
	(CursorPlaneWorldLocation - CursorInputDownWorldTransform.GetLocation()).ToDirectionAndLength(CursorDirection, CursorDistance);

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

	NewDesignerActorTransform.SetScale3D(NewScale);
	
	NewDesignerActorTransform.SetRotation(GetDesignerActorRotation().Quaternion());
	ControlledActor->SetActorTransform(NewDesignerActorTransform);
	ControlledActor->AddActorWorldOffset(GetDesignerSettings()->WorldLocationOffset);
	ControlledActor->AddActorLocalOffset(GetDesignerSettings()->RelativeLocationOffset);
}

void FDesignerEdMode::RegenerateRandomRotationOffset()
{
	GetDesignerSettings()->RandomRotationX.RegenerateRandomValue();
	GetDesignerSettings()->RandomRotationY.RegenerateRandomValue();
	GetDesignerSettings()->RandomRotationZ.RegenerateRandomValue();
}

FRotator FDesignerEdMode::GetRandomRotationOffset() const
{
	return FRotator( // Pitch, Yaw, Roll = Y, Z, X.
		GetDesignerSettings()->RandomRotationY.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotationZ.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotationX.GetCurrentRandomValue()
	);
}

void FDesignerEdMode::RegenerateRandomScale()
{
	GetDesignerSettings()->RandomScaleX.RegenerateRandomValue();
	GetDesignerSettings()->RandomScaleY.RegenerateRandomValue();
	GetDesignerSettings()->RandomScaleZ.RegenerateRandomValue();
}

FVector FDesignerEdMode::GetRandomScale() const
{
	return FVector(
		GetDesignerSettings()->RandomScaleX.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomScaleY.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomScaleZ.GetCurrentRandomValue()
	);
}

FRotator FDesignerEdMode::GetDesignerActorRotation()
{
	FVector MouseDirection;
	float MouseDistance;
	(CursorPlaneWorldLocation - CursorInputDownWorldTransform.GetLocation()).ToDirectionAndLength(MouseDirection, MouseDistance);

	// If the mouse is exactly at the CursorInputDownWorldTransform, which happens on mouse click down.
	if (MouseDirection.IsNearlyZero())
		MouseDirection = CursorInputDownWorldTransform.GetRotation().GetForwardVector();

	FVector ForwardVector = GetDesignerSettings()->AxisToAlignWithCursor == EAxisType::None ? CursorInputDownWorldTransform.GetRotation().GetForwardVector() : MouseDirection;
	FVector UpVector = CursorInputDownWorldTransform.GetRotation().GetUpVector();
	
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
