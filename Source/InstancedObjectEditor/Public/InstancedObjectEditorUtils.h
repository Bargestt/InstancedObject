#pragma once


struct FInstancedObjectMeta
{
	static const FName MD_ShowOnlyInnerProperties;
	static const FName MD_AdvancedWidget;
	static const FName MD_IndentSize;
	static const FName MD_BaseClass;
	static const FName MD_MustImplement;
	static const FName MD_InheritFilters;
	static const FName MD_HideChild;

	
	static const FName MD_Header;
	static const FName MD_MinWidth;
	static const FName MD_MaxWidth;
	static const FName MD_MinHeight;
	static const FName MD_MaxHeight;
	
	static const FName MD_Padding;
	static const FName MD_PaddingL;
	static const FName MD_PaddingR;
	static const FName MD_PaddingT;
	static const FName MD_PaddingB;
	
	static const FName MD_HideName;

	template<typename T>
	static T ReadMetaFromMap(const TMap<FName, FString>& Map, const FName& Meta, T DefaultValue)
	{
		T OutValue = DefaultValue;
		if (const FString* ValuePtr = Map.Find(Meta))
		{
			LexFromString(OutValue, **ValuePtr);
		}
		return OutValue;
	}
};

struct FInstancedObjectEditorUtils
{
	/**
	 * Check in order:
	 *  - Container
	 *  - ObjectHandle's owning Struct
	 *  - ObjectHandle
	 *  - UStruct
	 */
	static const FString* FindMetaData(const TSharedPtr<IPropertyHandle>& ObjectHandle, const FName& Key);

	static TSharedPtr<IClassViewerFilter> CreateClassFilerFrom(const TSharedRef<IPropertyHandle>& SourceHandle);
	static void CreateClassRestrictions(const TSharedRef<IPropertyHandle>& ObjectHandle, const TSharedPtr<IPropertyHandle>& SourceOverride);
	static void CreateInheritedClassRestrictions(const TSharedRef<IPropertyHandle>& ObjectHandle, const TSharedPtr<IPropertyHandle>& StartSearchFrom);

	/**
	 * Hide Protected and Private values
	 */
	static bool CanDisplayChild(const TSharedRef<IPropertyHandle>& ChildHandle, bool bAllowCategories = true);
	
	static FText ReadObjectTitle(const TSharedPtr<IPropertyHandle>& ObjectHandle, bool bFull, bool bRemoveStyle = false);

	
	static TSharedRef<SWidget> CreateHeader_ChildrenEditors(const TSharedPtr<IPropertyHandle>& ObjectHandle, IDetailChildrenBuilder& ChildBuilder);
};
