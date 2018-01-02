//  Copyright 2017 Roel Bartstra.

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

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

#include "Math/Axis.h"

class UDesignerSettings;
class AActor;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
struct FActorPositionTraceResult;

class FDesignerEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_DesignerEdModeId;

private:
	UStaticMeshComponent* PlacementVisualizerComponent;
	UMaterialInstanceDynamic* PlacementVisualizerMID;

	/** The plane we trace against when transforming the placed actor. */
	FPlane PlacementPlane;

	/* When spawning an object the mouse traces with a plane to determine the size and rotation. This is the world space hit location on that plane. */
	FVector MousePlaneHitLocation;

public:
	FDesignerEdMode();
	virtual ~FDesignerEdMode();

	bool CanSpawnActor;
	UDesignerSettings* UISettings;

	/** The selected actor according to designer. */
	AActor* SpawnedDesignerActor;

	/** The local box extent of the selected designer actor in cm when scale is uniform 1. */
	FVector DefaultDesignerActorExtent;

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	TSharedPtr<class FModeToolkit> GetToolkit();

	virtual void Enter() override;
	virtual void Exit() override;

	/** Draws translucent polygons on brushes and volumes. */
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI);

	virtual bool GetCursor(EMouseCursor::Type& OutCursor) const;
	
	bool LostFocus(FEditorViewportClient * ViewportClient, FViewport * Viewport);
	bool InputKey(FEditorViewportClient * ViewportClient, FViewport * Viewport, FKey Key, EInputEvent Event);
	
	/**
	 * Called when the mouse is moved while a window input capture is in effect
	 *
	 * @param	InViewportClient	Level editor viewport client that captured the mouse input
	 * @param	InViewport			Viewport that captured the mouse input
	 * @param	InMouseX			New mouse cursor X coordinate
	 * @param	InMouseY			New mouse cursor Y coordinate
	 *
	 * @return	true if input was handled
	 */
	virtual bool CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY);

	bool CreateDesignerActor(FEditorViewportClient* ViewportClient, FViewport* Viewport);

	bool UsesTransformWidget() const;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	//virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	
	virtual bool Select(AActor* InActor, bool bInSelected);

	virtual void ActorSelectionChangeNotify() override;

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const;

	/** True if this mode uses a toolkit mode (eventually they all should) */
	bool UsesToolkits() const override;

	FTransform CalculateDesignerActorTransform(FActorPositionTraceResult ActorPositionTraceResult);

private:
	void PlacementVisualizerMaterialData(FVector MouseLocationWorld);

};
