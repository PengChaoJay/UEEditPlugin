// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"

#include "AssetToolsModule.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"


void UQuickAssetAction::DuplicateAssets(int32 iNumOfDuplicates)
{
	if (iNumOfDuplicates <= 0)
	{
		//DebugHeader::Print(TEXT("please enter a valid number"),FColor::Green);
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("please enter a valid number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 iCounter = 0;
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < iNumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.GetObjectPathString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") +
				FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(),
			                                            NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				iCounter++;
			}
		}
	}
	if (iCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("deplicate Asset "+FString::FromInt(iCounter)+"files"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject) continue;;
		FString* prefixFound = prefixMap.Find(SelectedObject->GetClass());

		if (!prefixFound || prefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Filed to find prefix for class ") + SelectedObject->GetClass()->GetName(),
			                   FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();
		//去除重复开头命名
		if (OldName.StartsWith(*prefixFound))
		{
			DebugHeader::Print(OldName + TEXT("alredy has prefix added"), FColor::Red);
			continue;
		}
		//去除材质实例的的inst结尾以M开头
		if (SelectedObject->IsA<UMaterialInstance>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}
		const FString NewNameWithPrefix = *prefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		++Counter;
	}
	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed") + FString::FromInt(Counter) + "Assets");
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnsedAssetsData;
	FixUpRedirectors();
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(
			SelectedAssetData.GetObjectPathString());
		if (AssetReferences.Num() == 0)
		{
			UnsedAssetsData.Add(SelectedAssetData);
		}
	}
	if (UnsedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("no unused among assets "), false);
		return;
	}

	const int32& NumofAssetsDeleted = ObjectTools::DeleteAssets(UnsedAssetsData, true);
	if (NumofAssetsDeleted == 0) return;
	DebugHeader::ShowNotifyInfo("unsed assets has delete"+FString::FromInt(NumofAssetsDeleted));
}

void UQuickAssetAction::FixUpRedirectors()
{
	//创建重定向器数组
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryMoudle = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//过滤查找重定向器
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;

	AssetRegistryMoudle.Get().GetAssets(Filter,OutRedirectors);

	//找到所有的重定向
	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if(UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}
