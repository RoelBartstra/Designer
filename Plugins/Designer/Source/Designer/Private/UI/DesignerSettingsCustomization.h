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

#pragma once

#include "CoreMinimal.h"

#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "IDetailRootObjectCustomization.h"

class IPropertyHandle;
class UDesignerSettings;

/**
 * 
 */
class FDesignerSettingsRootObjectCustomization : public IDetailRootObjectCustomization
{
public:
	/** IDetailRootObjectCustomization interface */
    virtual TSharedPtr<SWidget> CustomizeObjectHeader(const FDetailsObjectSet& InRootObjectSet, const TSharedPtr<ITableRow>& InTableRow) override;
    virtual bool AreObjectsVisible(const FDetailsObjectSet& InRootObjectSet) const override { return true; }
    virtual bool ShouldDisplayHeader(const FDetailsObjectSet& InRootObjectSet) const override { return false; }
 };

/**
 * 
 */
class FDesignerSettingsCustomization : public IDetailCustomization
{
public:	
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

    void BuildLocationPropertyWidget(IDetailLayoutBuilder& DetailBuilder, IDetailGroup& Group, TSharedRef<IPropertyHandle> PropertyHandle);
    
	void OnPaintTypeChanged(IDetailLayoutBuilder* LayoutBuilder);

    template<typename type>
    static TOptional<type> GetOptionalPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle);

    template<typename type>
    static void SetPropertyValue(type NewValue, ETextCommit::Type CommitInfo, TSharedRef<IPropertyHandle> PropertyHandle);
	
	UDesignerSettings* DesignerSettings;

	/** Error visibility for UI. */
	EVisibility AxisErrorVisibilityUI() const;
};

template<typename type>
TOptional<type> FDesignerSettingsCustomization::GetOptionalPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle)
{
    type Value;
    if (PropertyHandle->GetValue(Value) == FPropertyAccess::Success)
    {
        return Value;
    }

    // Couldn't get, return unset optional
    return TOptional<type>();
}

template<typename type>
void FDesignerSettingsCustomization::SetPropertyValue(type NewValue, ETextCommit::Type CommitInfo, TSharedRef<IPropertyHandle> PropertyHandle)
{
    ensure(PropertyHandle->SetValue(NewValue) == FPropertyAccess::Success);
}

