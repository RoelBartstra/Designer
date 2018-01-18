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

#pragma once

#include "CoreMinimal.h"
#include "Tools/DesignerTool.h"
#include "UObject/GCObject.h"

class AActor;
class UDesignerSettings;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/**
 * Tool for spawning assets from the content browser.
 */
class FSpawnAssetTool : public FDesignerTool
{
private:
	/** The static mesh of the Spawn visualizer component */
	UStaticMeshComponent* SpawnVisualizerComponent;

	/** The material instance dynamic of the Spawn visualizer component */
	UMaterialInstanceDynamic* SpawnVisualizerMID;

	/** The plane we trace against when transforming the placed actor */
	FPlane SpawnTracePlane;

	/** The world transform stored on mouse click down */
	FTransform CursorInputDownWorldTransform;

	/** When spawning an object the mouse traces with a plane to determine the size and rotation. This is the world space hit location on that plane */
	FVector CursorPlaneWorldLocation;

	/** The settings available to the user */
	UDesignerSettings* DesignerSettings;

	/** The actor currently controlled by the designer editor mode */
	AActor* ControlledActor;

	/** The local box extent of the selected designer actor in cm when scale is uniform 1 */
	FVector DefaultDesignerActorExtent;

public:
	FSpawnAssetTool(UDesignerSettings* DesignerSettings);

	~FSpawnAssetTool();

	virtual void AddReferencedObjects(FReferenceCollector& Collector);

	/** Returns the name that gets reported to the editor. */
	virtual FString GetName() const;

	/** Called by the designer ed mode when switching to this tool */
	virtual void EnterTool();

	/** Called by the designer ed mode when switching to another tool from this tool */
	virtual void ExitTool();

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const;

	// User input

	virtual bool MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y);

	virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport);

	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y);

	virtual bool ReceivedFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport);

	virtual bool LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport);

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
	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY);


	/**
	 * @return		true if the delta was handled by this editor mode tool.
	 */
	virtual bool InputAxis(FEditorViewportClient* InViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime);

	/**
	 * @return		true if the delta was handled by this editor mode tool.
	 */
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale);

	/**
	 * @return		true if the key was handled by this editor mode tool.
	 */
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event);

	virtual void Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI);
	virtual void DrawHUD(FEditorViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas);

	//@{
	virtual bool StartModify();
	virtual bool EndModify();
	//@}

	//@{
	virtual void StartTrans();
	virtual void EndTrans();
	//@}

	// Tick
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime);

	/** @name Selections */
	//@{
	virtual void SelectNone();
	/** @return		true if something was selected/deselected, false otherwise. */
	virtual bool BoxSelect(FBox& InBox, bool InSelect = true);
	//@}

	virtual bool FrustumSelect(const FConvexVolume& InFrustum, bool InSelect = true);

	/** The settings available to the user */
	FORCEINLINE UDesignerSettings* GetDesignerSettings() const { return DesignerSettings; }

	/** The actor currently controlled by the designer editor mode */
	FORCEINLINE AActor* GetControlledActor() const { return ControlledActor; }

private:
	/** Update the material parameters for the spawn visualizer component. Returns true if it was successful */
	bool UpdateSpawnVisualizerMaterialParameters();

	/** Calculate the world transform for the mouse and store it in MouseDownWorldTransform. Returns true if it was successful */
	bool RecalculateMouseDownWorldTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport);
	
	/** Recalculate the world transform of the mouse and store it in the CurrentMouseWorldTransform. Returns true if it was successful */
	void RecalculateMouseSpawnTracePlaneWorldLocation(FEditorViewportClient* ViewportClient, FViewport* Viewport);

	/** Updates the designer actor transform so it matches with all the changes made to DesignerActorTransformExcludingOffset */
	void UpdateDesignerActorTransform();

	/** Generate new random rotation offset */
	void RegenerateRandomRotationOffset();

	/** Get the random rotation applied to the designer actor */
	FRotator GetRandomRotationOffset() const;

	/** Generate new random scale */
	void RegenerateRandomScale();

	/** The random scale applied to the designer actor */
	FVector GetRandomScale() const;

	/** Get the designer actor rotation with all settings applied to it */
	FRotator GetDesignerActorRotation();
};
