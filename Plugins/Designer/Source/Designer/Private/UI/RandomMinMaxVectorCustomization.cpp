#include "RandomMinMaxVectorCustomization.h"
#include <DetailLayoutBuilder.h>
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

TSharedRef<IPropertyTypeCustomization> FRandomMinMaxVectorCustomization::MakeInstance()
{
	return MakeShareable(new FRandomMinMaxVectorCustomization());
}

void FRandomMinMaxVectorCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderDetailWidgetRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

}

void FRandomMinMaxVectorCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& DetailChildrenBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{	
	if( PropertyHandle->IsValidHandle() )
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

		FString DisplayNameAsString = PropertyHandle->GetPropertyDisplayName().ToString();
		DisplayNameAsString = DisplayNameAsString.Append(" ").Append(PropertyHandle_X->GetPropertyDisplayName().ToString());
		FText DisplayName = FText::FromString(DisplayNameAsString);

		DetailChildrenBuilder.AddCustomRow(LOCTEXT("MinMaxFilter", "MinMax"))
		.NameContent()
		[			
			PropertyHandle_X->CreatePropertyNameWidget(DisplayName)
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
		
		DetailChildrenBuilder.AddCustomRow(LOCTEXT("MinMaxFilter", "MinMax"))
		.NameContent()
		[
			PropertyHandle_Y->CreatePropertyNameWidget()
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
					PropertyHandle_Y_Min->CreatePropertyNameWidget(LOCTEXT("MinYLabel","Min Y"))
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Y_Min->CreatePropertyValueWidget()
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
					PropertyHandle_Y_Max->CreatePropertyNameWidget(LOCTEXT("MaxXLabel","Max Y"))
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Y_Max->CreatePropertyValueWidget()
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
					PropertyHandle_Y_RandomlyNegate->CreatePropertyNameWidget(LOCTEXT("RandomlyNegateYLabel","Randomly Negate Y"))
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Y_RandomlyNegate->CreatePropertyValueWidget()
				]
			]
		];

		DetailChildrenBuilder.AddCustomRow(LOCTEXT("MinMaxFilter", "MinMax"))
		.NameContent()
		[
			PropertyHandle_Z->CreatePropertyNameWidget()
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
					PropertyHandle_Z_Min->CreatePropertyNameWidget(LOCTEXT("MinZLabel","Min Z"))
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Z_Min->CreatePropertyValueWidget()
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
					PropertyHandle_Z_Max->CreatePropertyNameWidget(LOCTEXT("MaxZLabel","Max Z"))
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Z_Max->CreatePropertyValueWidget()
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
					PropertyHandle_Z_RandomlyNegate->CreatePropertyNameWidget(LOCTEXT("RandomlyNegateZLabel","Randomly Negate Z"))
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Z_RandomlyNegate->CreatePropertyValueWidget()
				]
			]
		];
	}

}

#undef LOCTEXT_NAMESPACE