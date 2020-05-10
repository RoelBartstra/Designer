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

#include "SDesignerSettings.h"

#include "DesignerModule.h"
#include "DesignerEdMode.h"
#include "DesignerSettings.h"
#include "UI/DesignerSettingsCustomization.h"

#include "EditorModeManager.h"
#include "EditorModes.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "SlateOptMacros.h"


#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDesignerSettings::Construct(const FArguments & InArgs, TSharedRef<FDesignerEdModeToolkit> InParentToolkit)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs(
		/*bUpdateFromSelection*/	false,
		/*bLockable*/				false,
		/*bAllowSearch*/			false,
		/*InNameAreaSettings*/		FDetailsViewArgs::HideNameArea,
		/*bHideSelectionTip*/		true,
		/*InNotifyHook*/			nullptr,
		/*InSearchInitialKeyFocus*/	false,
		/*InViewIdentifier*/		NAME_None);

	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	DetailsViewArgs.bShowOptions = false;
	//DetailsViewArgs.bAllowMultipleTopLevelObjects = true;

	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SDesignerSettings::GetIsPropertyVisible));
	DetailsPanel->SetDisableCustomDetailLayouts(false);

	DetailsPanel->SetRootObjectCustomizationInstance(MakeShareable(new FDesignerSettingsRootObjectCustomization));
	FDesignerEdMode* DesignerEdMode = GetEditorMode();
	if (DesignerEdMode)
	{
		DetailsPanel->SetObject(DesignerEdMode->GetDesignerSettings(), true);
	}

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(0.0f)
		[
			SNew(SVerticalBox)
			.IsEnabled(this, &SDesignerSettings::GetDesignerSettingsIsEnabled)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0)
			[
				DetailsPanel.ToSharedRef()
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FDesignerEdMode* SDesignerSettings::GetEditorMode() const
{
	return (FDesignerEdMode*)GLevelEditorModeTools().GetActiveMode(FDesignerEdMode::EM_DesignerEdModeId);
}

FText SDesignerSettings::GetErrorText() const
{
	return FText();
}

bool SDesignerSettings::GetDesignerSettingsIsEnabled() const
{
	return true;
}

bool SDesignerSettings::GetIsPropertyVisible(const FPropertyAndParent & PropertyAndParent) const
{
	return true;
}
