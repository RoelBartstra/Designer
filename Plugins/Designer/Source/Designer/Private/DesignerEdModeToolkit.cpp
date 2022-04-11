// Copyright Epic Games, Inc. All Rights Reserved.

#include "DesignerEdModeToolkit.h"
#include "DesignerEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"

#include "SDesignerSettings.h"

#define LOCTEXT_NAMESPACE "FDesignerEdModeToolkit"

FDesignerEdModeToolkit::FDesignerEdModeToolkit()
{
}

void FDesignerEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	ToolkitWidget = SNew(SDesignerSettings, SharedThis(this));

	FModeToolkit::Init(InitToolkitHost);
}

FName FDesignerEdModeToolkit::GetToolkitFName() const
{
	return FName("DesignerEdMode");
}

FText FDesignerEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("DesignerEdModeToolkit", "DisplayName", "DesignerEdMode Tool");
}

class FEdMode* FDesignerEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FDesignerEdMode::EM_DesignerEdModeId);
}

#undef LOCTEXT_NAMESPACE
