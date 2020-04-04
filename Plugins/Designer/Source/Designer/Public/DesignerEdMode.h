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
#include "EdMode.h"

#include "Tools/SpawnAssetTool.h"

class UDesignerSettings;
class FSpawnAssetTool;

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

	virtual void Enter() override;
	virtual void Exit() override;

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const;
	
	/** True if this mode uses a toolkit mode (eventually they all should) */
	bool UsesToolkits() const override;

	/** Set the current tool to the new designer tool while also calling ExitTool on the previous DesignerTool and EnterTool on the NewDesignerTool */
	void SwitchTool(FDesignerTool* NewDesignerTool);
};
