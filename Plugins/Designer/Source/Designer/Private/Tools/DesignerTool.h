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
#include "EditorModeTools.h"
#include "UObject/GCObject.h"

/**
 * Tool for spawning assets from the content browser.
 */
class FDesignerTool : public FModeTool, public FGCObject
{
public:
	FDesignerTool() {}

	~FDesignerTool() {}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) {}

	/** Returns the name that gets reported to the editor */
	virtual FString GetName() const { return TEXT("DesignerTool"); }

	/** Called by the designer ed mode when switching to this tool */
	virtual void EnterTool() {}

	/** Called by the designer ed mode when switching to another tool from this tool */
	virtual void ExitTool() {}

    /** Determines whether the tool is currently in use or not */
    virtual void SetToolActive(bool IsActive) {}

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const { return true; }

    bool IsToolActive;
};
