// Copyright Epic Games, Inc. All Rights Reserved.

#include "DesignerModule.h"
#include "DesignerEdMode.h"

#include "DesignerSlateStyle.h"
#include "UI/DesignerSettingsCustomization.h"
#include "UI/Bool3Customization.h"
#include "UI/RandomMinMaxFloatCustomization.h"

#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FDesignerModule"

DEFINE_LOG_CATEGORY(LogDesigner);

void FDesignerModule::StartupModule()
{
	FDesignerSlateStyle::Initialize();

	FSlateIcon DesignerIcon = FSlateIcon(FDesignerSlateStyle::Get()->GetStyleSetName(), "Designer.Icon", "Designer.Icon.Small");
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FDesignerEdMode>(FDesignerEdMode::EM_DesignerEdModeId, LOCTEXT("DesignerEdModeName", "Designer"), DesignerIcon, true);

	/** Register detail/property customization */
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("Bool3", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBool3Customization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("RandomMinMaxFloat", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FRandomMinMaxFloatCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("DesignerSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FDesignerSettingsCustomization::MakeInstance));
}

void FDesignerModule::ShutdownModule()
{
	FDesignerSlateStyle::Shutdown();
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FDesignerEdMode::EM_DesignerEdModeId);

	/** De-register detail/property customization */
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout("DesignerSettings");
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDesignerModule, Designer)