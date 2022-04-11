// Copyright Epic Games, Inc. All Rights Reserved.

#include "DesignerEdMode.h"
#include "DesignerEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

#include "DesignerSettings.h"
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
	//UE_LOG(LogDesigner, Log, TEXT("FDesignerEdMode::SwitchTool"));
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
