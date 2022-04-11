// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class UDesignerSettings;
class FSpawnAssetTool;
class FDesignerTool;

class FDesignerEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_DesignerEdModeId;

private:
	UDesignerSettings* DesignerSettings;
	FSpawnAssetTool* SpawnAssetTool;

public:
	FDesignerEdMode();
	virtual ~FDesignerEdMode();

	/** The settings available to the user */
	FORCEINLINE UDesignerSettings* GetDesignerSettings() const { return DesignerSettings; }

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	TSharedPtr<class FModeToolkit> GetToolkit();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const;

	/** True if this mode uses a toolkit mode (eventually they all should) */
	bool UsesToolkits() const override;

	/** Set the current tool to the new designer tool while also calling ExitTool on the previous DesignerTool and EnterTool on the NewDesignerTool */
	void SwitchTool(FDesignerTool* NewDesignerTool);
};
