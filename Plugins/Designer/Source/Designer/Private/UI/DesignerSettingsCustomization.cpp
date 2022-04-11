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

#include "DesignerSettingsCustomization.h"
#include "DesignerModule.h"

#include "DesignerEdMode.h"
#include "DesignerSettings.h"

#include "DetailLayoutBuilder.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "PropertyHandle.h"
#include "Misc/Optional.h"
#include "Fonts/SlateFontInfo.h"

#include "EditorModeManager.h"

#include "SlateOptMacros.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

TSharedRef<IDetailCustomization> FDesignerSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FDesignerSettingsCustomization());
}

void FDesignerSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	FDesignerEdMode* DesignerEdMode = (FDesignerEdMode*)GLevelEditorModeTools().GetActiveMode(FDesignerEdMode::EM_DesignerEdModeId);
	DesignerSettings = DesignerEdMode->GetDesignerSettings();
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("AxisAlignment");

	Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, AxisToAlignWithNormal)));
	Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, AxisToAlignWithCursor)));
	TAttribute<EVisibility> AxisErrorVisibilityAttribute = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDesignerSettingsCustomization::AxisErrorVisibilityUI));
	Category.AddCustomRow(FText::FromString("DesignerAxisError"))
	.Visibility(AxisErrorVisibilityAttribute)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.ColorAndOpacity(FSlateColor(FLinearColor::Red))
			.Text(LOCTEXT("AxisError", "\"Axis to Align with Normal\" and \"Axis to Align with Cursor\" are parallel, this is not allowed. Default rotation will be used instead."))
		]
	];
}

void FDesignerSettingsCustomization::OnPaintTypeChanged(IDetailLayoutBuilder* LayoutBuilder)
{
	LayoutBuilder->ForceRefreshDetails();
}

TSharedPtr<SWidget> FDesignerSettingsRootObjectCustomization::CustomizeObjectHeader(const FDetailsObjectSet& InRootObjectSet, const TSharedPtr<ITableRow>& InTableRow)
{
	return SNullWidget::NullWidget;
}

EVisibility FDesignerSettingsCustomization::AxisErrorVisibilityUI() const
{
   return ((int)DesignerSettings->AxisToAlignWithNormal && ((int)DesignerSettings->AxisToAlignWithNormal >> 1) == ((int)DesignerSettings->AxisToAlignWithCursor >> 1)) ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE