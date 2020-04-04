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

#include "Tools/DesignerTool.h"
#include "Tools/SpawnAssetTool.h"

const FEditorModeID FDesignerEdMode::EM_DesignerEdModeId = TEXT("EM_DesignerEdMode");

FDesignerEdMode::FDesignerEdMode()
{
	DesignerSettings = NewObject<UDesignerSettings>(GetTransientPackage(), TEXT("DesignerEdModeSettings"), RF_Transactional);
	DesignerSettings->SetParent(this);

	SpawnAssetTool = new FSpawnAssetTool(DesignerSettings);
}

FDesignerEdMode::~FDesignerEdMode()
{

}

void FDesignerEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Call parent implementation
	FEdMode::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(DesignerSettings);
	
	//Collector.AddReferencedObject(SpawnAssetTool);
}

TSharedPtr<class FModeToolkit> FDesignerEdMode::GetToolkit()
{
	return Toolkit;
}

void FDesignerEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FDesignerEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	SwitchTool(SpawnAssetTool);
}

void FDesignerEdMode::Exit()
{
	SwitchTool(nullptr);

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FDesignerEdMode::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	bool bResult = true;
	FDesignerTool* CurrentDesignerTool = static_cast<FDesignerTool*>(CurrentTool);
	if (CurrentDesignerTool)
	{
		bResult = CurrentDesignerTool->IsSelectionAllowed(InActor, bInSelection);
	}
	return bResult;
}

bool FDesignerEdMode::UsesToolkits() const
{
	return true;
}

void FDesignerEdMode::SwitchTool(FDesignerTool* NewDesignerTool)
{
	UE_LOG(LogDesigner, Log, TEXT("FDesignerEdMode::SwitchTool"));
	FDesignerTool* CurrentDesignerTool = static_cast<FDesignerTool*>(CurrentTool);
	if (CurrentDesignerTool != nullptr)
	{
		CurrentDesignerTool->ExitTool();
	}

	if (NewDesignerTool != nullptr)
	{
		NewDesignerTool->EnterTool();
		SetCurrentTool(NewDesignerTool);
	}
	else
	{
		CurrentTool = nullptr;
	}	
}
