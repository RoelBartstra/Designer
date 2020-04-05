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

#include "DesignerModule.h"
#include "DesignerEdMode.h"

#include "DesignerSlateStyle.h"
#include "UI/DesignerSettingsCustomization.h"
#include "UI/Bool3Customization.h"
#include "UI/RandomMinMaxFloatCustomization.h"

#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FDesignerEditorMode"

DEFINE_LOG_CATEGORY(LogDesigner);

void FDesignerModule::StartupModule()
{
	FDesignerSlateStyle::Initialize();

	FSlateIcon DesignerIcon = FSlateIcon(FDesignerSlateStyle::Get()->GetStyleSetName(), "Designer.Icon", "Designer.Icon.Small");

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FDesignerEdMode>(FDesignerEdMode::EM_DesignerEdModeId, LOCTEXT("DesignerEdModeName", "Designer"), DesignerIcon, true, 100);

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