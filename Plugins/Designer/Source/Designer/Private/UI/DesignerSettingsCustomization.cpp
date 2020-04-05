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
#include <SNumericEntryBox.h>

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

TSharedRef<IDetailCustomization> FDesignerSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FDesignerSettingsCustomization());
}

void FDesignerSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	FDesignerEdMode* DesignerEdMode = (FDesignerEdMode*)GLevelEditorModeTools().GetActiveMode(FDesignerEdMode::EM_DesignerEdModeId);
	UDesignerSettings* DesignerSettings = DesignerEdMode->GetDesignerSettings();
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("SpawnSettings");

	//// Create RelativeLocationOffset widget.
	BuildLocationPropertyWidget(DetailBuilder, Category, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RelativeLocationOffset)));

	//// Create WorldLocationOffset widget.
	BuildLocationPropertyWidget(DetailBuilder, Category, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, WorldLocationOffset)));

	//Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, AxisToAlignWithNormal)));

	//Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, AxisToAlignWithCursor)));

	// Create SnapToGridRotation widget.
	//BuildBool3PropertyWidget(DetailBuilder, Category, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, SnapRotationToGrid)));
	
	//Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bApplyRandomRotation)));
	//
	//BuildRandomMinMaxVectorPropertyWidget(DetailBuilder, Category, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RandomRotation)));
	//
	//Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bScaleBoundsTowardsCursor)));
	//
	//Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, MinimalScale)));

	//Category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, bApplyRandomScale)));
	//
	//BuildRandomMinMaxVectorPropertyWidget(DetailBuilder, Category, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RandomScale)));
}

void FDesignerSettingsCustomization::BuildLocationPropertyWidget(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& Category, TSharedRef<IPropertyHandle> PropertyHandle)
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

	Category.AddProperty(PropertyHandle)
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

void FDesignerSettingsCustomization::BuildRandomMinMaxVectorPropertyWidget(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& Category, TSharedRef<IPropertyHandle> PropertyHandle)
{
	TSharedRef<IPropertyHandle> PropertyHandle_X = PropertyHandle->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_X_Min = PropertyHandle_X->GetChildHandle("Min").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_X_Max = PropertyHandle_X->GetChildHandle("Max").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_X_RandomlyNegate = PropertyHandle_X->GetChildHandle("bRandomlyNegate").ToSharedRef();

	TSharedRef<IPropertyHandle> PropertyHandle_Y = PropertyHandle->GetChildHandle("Y").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Y_Min = PropertyHandle_Y->GetChildHandle("Min").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Y_Max = PropertyHandle_Y->GetChildHandle("Max").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Y_RandomlyNegate = PropertyHandle_Y->GetChildHandle("bRandomlyNegate").ToSharedRef();

	TSharedRef<IPropertyHandle> PropertyHandle_Z = PropertyHandle->GetChildHandle("Z").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Z_Min = PropertyHandle_Z->GetChildHandle("Min").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Z_Max = PropertyHandle_Z->GetChildHandle("Max").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Z_RandomlyNegate = PropertyHandle_Z->GetChildHandle("bRandomlyNegate").ToSharedRef();

	float LabelPadding = 2.0f;
	float ComponentPadding = 16.0f;

	Category.AddCustomRow(LOCTEXT("VectorFilter", "Vector"))
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(125.0f * 3.0f) // copied from FComponentTransformDetails
	.MaxDesiredWidth(125.0f * 3.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0.f, 0.f, ComponentPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyHandle_X_Min->CreatePropertyNameWidget(LOCTEXT("MinXLabel","Min X"))
			]
			+ SHorizontalBox::Slot()
			.Padding(LabelPadding, 0.f, 0.f, 0.f)
			.AutoWidth()
			[
				PropertyHandle_X_Min->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, ComponentPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyHandle_X_Max->CreatePropertyNameWidget(LOCTEXT("MaxXLabel","Max X"))
			]
			+ SHorizontalBox::Slot()
			.Padding(LabelPadding, 0.f, 0.f, 0.f)
			.AutoWidth()
			[
				PropertyHandle_X_Max->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, ComponentPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyHandle_X_RandomlyNegate->CreatePropertyNameWidget(LOCTEXT("RandomlyNegateXLabel","Randomly Negate X"))
			]
			+ SHorizontalBox::Slot()
			.Padding(LabelPadding, 0.f, 0.f, 0.f)
			.AutoWidth()
			[
				PropertyHandle_X_RandomlyNegate->CreatePropertyValueWidget()
			]
		]
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

#undef LOCTEXT_NAMESPACE