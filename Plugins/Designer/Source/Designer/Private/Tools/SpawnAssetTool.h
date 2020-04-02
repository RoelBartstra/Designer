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
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	TArray<AActor*> PreviewActorArray;

	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	AActor* PreviewActor;

	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	AActor* PreviewActorPulsing;

	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	UMaterialInstanceDynamic* PreviewActorMID;

	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	UMaterialInstanceDynamic* PreviewActorPulsingMID;

	/** The static mesh of the Spawn visualizer component */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	UStaticMeshComponent* SpawnPlaneComponent;

	/** The material instance dynamic of the Spawn visualizer component */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	UMaterialInstanceDynamic* SpawnVisualizerMID;

	/** The plane we trace against when transforming the placed actor */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	FPlane SpawnTracePlane;

	/** The world transform stored on mouse click down */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	FTransform SpawnWorldTransform;

	/** When spawning an object the mouse traces with a plane to determine the size and rotation. This is the world space hit location on that plane */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	FVector CursorPlaneIntersectionWorldLocation;

	/** The settings available to the user */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	UDesignerSettings* DesignerSettings;

	/** The actor currently controlled by the designer editor mode */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	AActor* SpawnedActorPreview;

	/** The actor currently controlled by the designer editor mode */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	AActor* SpawnedActor;

	/** The local box extent of the selected designer actor in cm when scale is uniform 1 */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	FVector DefaultDesignerActorExtent;

	/** The array of assets which is selected in the content browser and is actually placeable */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	TArray<FAssetData> PlaceableSelectedAssets;

	/** The asset which should be spawned and is currently being previewed */
	UPROPERTY(Category = "SpawnAsset", NonTransactional)
	FAssetData TargetAssetDataToSpawn;
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

	virtual bool FrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect = true);

	/** The settings available to the user */
	FORCEINLINE UDesignerSettings* GetDesignerSettings() const { return DesignerSettings; }

	/** The actor currently controlled by the designer editor mode */
	FORCEINLINE AActor* GetControlledActor() const { return SpawnedActor; }

private:
	virtual void SetToolActive(bool IsActive) override;

	void SetAllMaterialsForActor(AActor* Actor, UMaterialInterface* Material);

	/** Create all preview actors */
	void RefreshPreviewActors();

	/** Destroy all preview actors */
	void DestroyPreviewActors();

	/** Non transactional version of UEditorEngine::UseActorFactory */
	AActor* SpawnPreviewActorFromFactory(UActorFactory* Factory, const FAssetData& AssetData, const FTransform* InActorTransform, EObjectFlags InObjectFlags);

	/** Clears the PlaceableSelectedAssets array and fills it again with the placeable assets currently selected in the content browser */
	void RefreshPlaceableAsset();

	/** Helper function to see if asset data can be placed in the world */
	bool IsAssetDataPlaceable(FAssetData AssetData);

	/** Update the material parameters for the spawn visualizer component. Returns true if it was successful */
	bool UpdateSpawnVisualizerMaterialParameters();

	/** Calculate the world transform for the mouse and store it in MouseDownWorldTransform. Returns true if it was successful */
	bool RecalculateSpawnTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport);
	
	/** Recalculate the world transform of the mouse and store it in the CurrentMouseWorldTransform. Returns true if it was successful */
	void RecalculateMousePlaneIntersectionWorldLocation(FEditorViewportClient* ViewportClient, FViewport* Viewport);

	/** Update the preview actors transform */
	void UpdatePreviewActorTransform();

	/** Updates the spawned actor transform */
	void UpdateSpawnedActorTransform();

	/** Generate new random rotation offset */
	void RegenerateRandomRotationOffset();

	/** Get the random rotation applied to the designer actor */
	FRotator GetRandomRotationOffset() const;

	/** Generate new random scale */
	void RegenerateRandomScale();

	/** The random scale applied to the designer actor */
	FVector GetSpawnActorScale() const;

	/** Get the designer actor rotation with all settings applied to it */
	FRotator GetSpawnActorRotation();
};
