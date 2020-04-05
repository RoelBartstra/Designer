#include "RandomMinMaxFloatCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

TSharedRef<IPropertyTypeCustomization> FRandomMinMaxFloatCustomization::MakeInstance()
{
	return MakeShareable(new FRandomMinMaxFloatCustomization());
}

void FRandomMinMaxFloatCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderDetailWidgetRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

}

void FRandomMinMaxFloatCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& DetailChildrenBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{	
	if( PropertyHandle->IsValidHandle() )
	{
		TSharedRef<IPropertyHandle> PropertyHandle_Min = PropertyHandle->GetChildHandle("Min").ToSharedRef();
		TSharedRef<IPropertyHandle> PropertyHandle_Max = PropertyHandle->GetChildHandle("Max").ToSharedRef();
		TSharedRef<IPropertyHandle> PropertyHandle_RandomlyNegate = PropertyHandle->GetChildHandle("bRandomlyNegate").ToSharedRef();

		float LabelPadding = 4.0f;
		float ComponentPadding = 8.0f;

		float ValueMin;
		PropertyHandle_Min->GetValue(ValueMin);

		float ValueMax;
		PropertyHandle_Max->GetValue(ValueMax);

		float ValueRandomlyNegate;
		PropertyHandle_RandomlyNegate->GetValue(ValueRandomlyNegate);

		DetailChildrenBuilder.AddCustomRow(LOCTEXT("MinMaxFilter", "MinMax"))
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
			.Padding(0.f, 0.f, ComponentPadding, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					PropertyHandle_Min->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				[
					PropertyHandle_Min->CreatePropertyValueWidget()
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(0.f, 0.f, ComponentPadding, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					PropertyHandle_Max->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				[
					PropertyHandle_Max->CreatePropertyValueWidget()
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(0.f, 0.f, ComponentPadding, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					PropertyHandle_RandomlyNegate->CreatePropertyNameWidget(FText::FromString("Can Negate"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				[
					PropertyHandle_RandomlyNegate->CreatePropertyValueWidget()
				]
			]
		];
	}

}

#undef LOCTEXT_NAMESPACE