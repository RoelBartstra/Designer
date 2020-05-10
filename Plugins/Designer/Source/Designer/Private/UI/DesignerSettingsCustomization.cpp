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
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("SpawnSettings");

	IDetailGroup& AxisAlignmentGroup = Category.AddGroup("AxisAlignmentGroup", LOCTEXT("AxisAlignment", "Axis Alignment"), false, true);
	AxisAlignmentGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, AxisToAlignWithNormal)));
	AxisAlignmentGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, AxisToAlignWithCursor)));	
	TAttribute<EVisibility> AxisErrorVisibilityAttribute = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDesignerSettingsCustomization::AxisErrorVisibilityUI));
	AxisAlignmentGroup.AddWidgetRow()
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

	IDetailGroup& LocationSettingsGroup = Category.AddGroup("LocationOffsetsGroup", LOCTEXT("LocationSettings", "Location Settings"), false, true);	
	BuildLocationPropertyWidget(DetailBuilder, LocationSettingsGroup, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RelativeLocationOffset)));
	LocationSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bScaleRelativeLocationOffset)));
	BuildLocationPropertyWidget(DetailBuilder, LocationSettingsGroup, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, WorldLocationOffset)));
	LocationSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bScaleWorldLocationOffset)));

	IDetailGroup& RotationSettingsGroup = Category.AddGroup("RotationSettingsGroup", LOCTEXT("RotationSettings", "Rotation Settings"), false, true);
	RotationSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, SnapRotationToGrid)));
	RotationSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bApplyRandomRotation)));
	RotationSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RandomRotation)));

	IDetailGroup& ScaleSettingsGroup = Category.AddGroup("ScaleSettingsGroup", LOCTEXT("ScaleSettings", "Scale Settings"), false, true);
	ScaleSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bScaleBoundsTowardsCursor)));
	ScaleSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, MinimalScale)));
	ScaleSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bApplyRandomScale)));
	ScaleSettingsGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RandomScale)));
}

void FDesignerSettingsCustomization::BuildLocationPropertyWidget(IDetailLayoutBuilder& DetailBuilder, IDetailGroup& Group, TSharedRef<IPropertyHandle> PropertyHandle)
{
	TSharedRef<IPropertyHandle> PropertyHandle_Location_X = PropertyHandle->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Location_Y = PropertyHandle->GetChildHandle("Y").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Location_Z = PropertyHandle->GetChildHandle("Z").ToSharedRef();

	float Value_X;
	PropertyHandle_Location_X->GetValue(Value_X);
	float Value_Y;
	PropertyHandle_Location_Y->GetValue(Value_Y);
	float Value_Z;
	PropertyHandle_Location_Z->GetValue(Value_Z);

	Group.AddPropertyRow(PropertyHandle)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(125.0f * 3.0f) // copied from FComponentTransformDetails
	.MaxDesiredWidth(125.0f * 3.0f)
	[
		SNew(SVectorInputBox)
		.bColorAxisLabels(true)
		.AllowSpin(false)
		.Font(DetailBuilder.GetDetailFont())
		.X_Static(&GetOptionalPropertyValue<float>, PropertyHandle_Location_X)
		.Y_Static(&GetOptionalPropertyValue<float>, PropertyHandle_Location_Y)
		.Z_Static(&GetOptionalPropertyValue<float>, PropertyHandle_Location_Z)
		.OnXCommitted_Static(&SetPropertyValue<float>, PropertyHandle_Location_X)
		.OnYCommitted_Static(&SetPropertyValue<float>, PropertyHandle_Location_Y)
		.OnZCommitted_Static(&SetPropertyValue<float>, PropertyHandle_Location_Z)
		.OnXChanged_Lambda([=](float NewValue) { ensure(PropertyHandle_Location_X->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.OnYChanged_Lambda([=](float NewValue) { ensure(PropertyHandle_Location_Y->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.OnZChanged_Lambda([=](float NewValue) { ensure(PropertyHandle_Location_Z->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.AllowSpin(true)
	];
}

void FDesignerSettingsCustomization::OnPaintTypeChanged(IDetailLayoutBuilder* LayoutBuilder)
{
	//UE_LOG(LogDesigner, Error, TEXT("FDesignerSettingsCustomization::OnPaintTypeChanged"));
	LayoutBuilder->ForceRefreshDetails();
}

TSharedPtr<SWidget> FDesignerSettingsRootObjectCustomization::CustomizeObjectHeader(const FDetailsObjectSet& InRootObjectSet, const TSharedPtr<ITableRow>& InTableRow)
{
	//UE_LOG(LogDesigner, Error, TEXT("FDesignerSettingsRootObjectCustomization::CustomizeObjectHeader"));
	return SNullWidget::NullWidget;
}

EVisibility FDesignerSettingsCustomization::AxisErrorVisibilityUI() const
{
   return ((int)DesignerSettings->AxisToAlignWithNormal && ((int)DesignerSettings->AxisToAlignWithNormal >> 1) == ((int)DesignerSettings->AxisToAlignWithCursor >> 1)) ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE