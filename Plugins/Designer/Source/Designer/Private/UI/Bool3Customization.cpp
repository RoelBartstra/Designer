#include "Bool3Customization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

TSharedRef<IPropertyTypeCustomization> FBool3Customization::MakeInstance()
{
	return MakeShareable(new FBool3Customization());
}

void FBool3Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderDetailWidgetRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

}

void FBool3Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& DetailChildrenBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{	
	if( PropertyHandle->IsValidHandle() )
	{
		const TSharedRef<IPropertyHandle> PropertyHandle_X = PropertyHandle->GetChildHandle("X").ToSharedRef();
		const TSharedRef<IPropertyHandle> PropertyHandle_Y = PropertyHandle->GetChildHandle("Y").ToSharedRef();
		const TSharedRef<IPropertyHandle> PropertyHandle_Z = PropertyHandle->GetChildHandle("Z").ToSharedRef();
		
		float LabelPadding = 4.0f;
		float ComponentPadding = 8.0f;

		DetailChildrenBuilder.AddProperty(PropertyHandle)
		.CustomWidget()
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(0.f, 0.f, ComponentPadding, 0.f)
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					PropertyHandle_X->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_X->CreatePropertyValueWidget()
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
					PropertyHandle_Y->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Y->CreatePropertyValueWidget()
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
					PropertyHandle_Z->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				.Padding(LabelPadding, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					PropertyHandle_Z->CreatePropertyValueWidget()
				]
			]
		];
	}

}

#undef LOCTEXT_NAMESPACE