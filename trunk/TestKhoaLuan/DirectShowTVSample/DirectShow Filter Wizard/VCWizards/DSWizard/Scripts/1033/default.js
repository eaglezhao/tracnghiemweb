
function OnFinish(selProj, selObj)
{
	try
	{
		var strProjectPath = wizard.FindSymbol('PROJECT_PATH');
		var strProjectName = wizard.FindSymbol('PROJECT_NAME');

		selProj = CreateCustomProject(strProjectName, strProjectPath);
		AddConfig(selProj, strProjectName);
		AddFilters(selProj);

		var InfFile = CreateCustomInfFile();
		AddFilesToCustomProj(selProj, strProjectName, strProjectPath, InfFile);
		PchSettings(selProj);
		InfFile.Delete();

		selProj.Object.Save();
	}
	catch(e)
	{
		if (e.description.length != 0)
			SetErrorInfo(e);
		return e.number
	}
}

function CreateCustomProject(strProjectName, strProjectPath)
{
	try
	{
		var strProjTemplatePath = wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
		var strProjTemplate = '';
		strProjTemplate = strProjTemplatePath + '\\default.vcproj';

		var Solution = dte.Solution;
		var strSolutionName = "";
		if (wizard.FindSymbol("CLOSE_SOLUTION"))
		{
			Solution.Close();
			strSolutionName = wizard.FindSymbol("VS_SOLUTION_NAME");
			if (strSolutionName.length)
			{
				var strSolutionPath = strProjectPath.substr(0, strProjectPath.length - strProjectName.length);
				Solution.Create(strSolutionPath, strSolutionName);
			}
		}

		var strProjectNameWithExt = '';
		strProjectNameWithExt = strProjectName + '.vcproj';

		var oTarget = wizard.FindSymbol("TARGET");
		var prj;
		if (wizard.FindSymbol("WIZARD_TYPE") == vsWizardAddSubProject)  // vsWizardAddSubProject
		{
			var prjItem = oTarget.AddFromTemplate(strProjTemplate, strProjectNameWithExt);
			prj = prjItem.SubProject;
		}
		else
		{
			prj = oTarget.AddFromTemplate(strProjTemplate, strProjectPath, strProjectNameWithExt);
		}
		return prj;
	}
	catch(e)
	{
		throw e;
	}
}

function AddFilters(proj)
{
	try
	{
		// Add the folders to your project
		var strSrcFilter = wizard.FindSymbol('SOURCE_FILTER');
		var group = proj.Object.AddFilter('Source Files');
		group.Filter = strSrcFilter;
	}
	catch(e)
	{
		throw e;
	}
}

function AddConfig(proj, strProjectName)
{
	try
	{
		var config = proj.Object.Configurations('Debug');
		config.IntermediateDirectory = 'Debug';
		config.OutputDirectory = 'Debug';
		config.ConfigurationType = ConfigurationTypes.typeDynamicLibrary;
		
		var CLTool = config.Tools('VCCLCompilerTool');
		CLTool.AdditionalIncludeDirectories = 'C:\\DXSDK\\Samples\\C++\\DirectShow\\BaseClasses,C:\\DXSDK\\Include';
		CLTool.PreprocessorDefinitions = "DEBUG" + CLTool.PreprocessorDefinitions;
		
		CLTool.DebugInformationFormat = debugOption.debugEnabled;
		CLTool.WarningLevel = warningLevelOption.warningLevel_0;
		CLTool.Optimization = optimizeOption.optimizeDisabled;
		CLTool.RuntimeLibrary = runtimeLibraryOption.rtMultiThreadedDLL;
		CLTool.CallingConvention = callingConventionOption.callConventionStdCall;
		CLTool.CompileAs = CompileAsOptions.compileAsDefault;
		CLTool.UsePrecompiledHeader = pchOption.pchGenerateAuto;
		CLTool.PrecompiledHeaderThrough = 'streams.h';
		CLTool.PrecompiledHeaderFile = './Debug/' + strProjectName + '.pch';

		var LinkTool = config.Tools('VCLinkerTool');
		LinkTool.AdditionalDependencies = 'C:\\DXSDK\\Samples\\C++\\DirectShow\\BaseClasses\\debug\\strmbasd.lib msvcrtd.lib quartz.lib vfw32.lib winmm.lib';
		LinkTool.OutputFile = 'Debug\\' + strProjectName + '.ax';
		LinkTool.IgnoreAllDefaultLibraries = true;
		LinkTool.AdditionalLibraryDirectories = 'C:\\DXSDK\\Lib';
		LinkTool.EntryPointSymbol = 'DllEntryPoint@12';
		LinkTool.ModuleDefinitionFile = '$(SolutionDir)' + strProjectName + '.def';
		LinkTool.LinkIncremental = linkIncrementalType.linkIncrementalYes;
		LinkTool.SuppressStartupBanner = true;
		LinkTool.GenerateDebugInformation = true;
		LinkTool.ProgramDatabaseFile = '$(OutDir)/$(ProjectName).pdb';

		// TODO: remove odbc32.lib odbccp32.lib
		
		config = proj.Object.Configurations('Release');
		config.IntermediateDirectory = 'Release';
		config.OutputDirectory = 'Release';
		config.ConfigurationType = ConfigurationTypes.typeDynamicLibrary;

		var CLTool = config.Tools('VCCLCompilerTool');
		CLTool.AdditionalIncludeDirectories = 'C:\\DXSDK\\Samples\\C++\\DirectShow\\BaseClasses,C:\\DXSDK\\Include';
		CLTool.WarningLevel = warningLevelOption.warningLevel_0;
		CLTool.RuntimeLibrary = runtimeLibraryOption.rtMultiThreadedDLL;
		CLTool.CallingConvention = callingConventionOption.callConventionStdCall;
		CLTool.CompileAs = CompileAsOptions.compileAsDefault;
		CLTool.UsePrecompiledHeader = pchOption.pchGenerateAuto;
		CLTool.PrecompiledHeaderThrough = 'streams.h';
		CLTool.PrecompiledHeaderFile = './Release/' + strProjectName + '.pch';

		var LinkTool = config.Tools('VCLinkerTool');
		LinkTool.AdditionalDependencies = 'C:\\DXSDK\\Samples\\C++\\DirectShow\\BaseClasses\\release\\strmbase.lib msvcrt.lib quartz.lib vfw32.lib winmm.lib';
		LinkTool.OutputFile = 'Release\\' + strProjectName + '.ax';
		LinkTool.IgnoreAllDefaultLibraries = true;
		LinkTool.AdditionalLibraryDirectories = 'C:\\DXSDK\\Lib';
		LinkTool.EntryPointSymbol = 'DllEntryPoint@12';
		LinkTool.ModuleDefinitionFile = '$(SolutionDir)' + strProjectName + '.def';
		LinkTool.LinkIncremental = linkIncrementalType.linkIncrementalYes;
		LinkTool.SuppressStartupBanner = true;

		// TODO: remove odbc32.lib odbccp32.lib
	}
	catch(e)
	{
		throw e;
	}
}

function PchSettings(proj)
{
	// TODO: specify pch settings
}

function DelFile(fso, strWizTempFile)
{
	try
	{
		if (fso.FileExists(strWizTempFile))
		{
			var tmpFile = fso.GetFile(strWizTempFile);
			tmpFile.Delete();
		}
	}
	catch(e)
	{
		throw e;
	}
}

function CreateCustomInfFile()
{
	try
	{
		var fso, TemplatesFolder, TemplateFiles, strTemplate;
		fso = new ActiveXObject('Scripting.FileSystemObject');

		var TemporaryFolder = 2;
		var tfolder = fso.GetSpecialFolder(TemporaryFolder);
		var strTempFolder = tfolder.Drive + '\\' + tfolder.Name;

		var strWizTempFile = strTempFolder + "\\" + fso.GetTempName();

		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
		var strInfFile = strTemplatePath + '\\Templates.inf';
		wizard.RenderTemplate(strInfFile, strWizTempFile);

		var WizTempFile = fso.GetFile(strWizTempFile);
		return WizTempFile;
	}
	catch(e)
	{
		throw e;
	}
}

function GetTargetName(strName, strProjectName)
{
	try
	{
		var strTarget = strName;

		// replace root with strProjectName
		strTarget = strTarget.replace(/root/g, strProjectName);

		// remove t_		
		if (strTarget.substr(0,2) == "t_")
			strTarget = strTarget.substr(2,strTarget.length-2);
		
		// remove ip_
		if (strTarget.substr(0,3) == "ip_")
			strTarget = strTarget.substr(3,strTarget.length-3);
			
		return strTarget; 
	}
	catch(e)
	{
		throw e;
	}
}

function AddFilesToCustomProj(proj, strProjectName, strProjectPath, InfFile)
{
	try
	{
		var projItems = proj.ProjectItems

		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');

		var strTpl = '';
		var strName = '';

		// add symbols accessable by the templates
		var strRawGUID = wizard.CreateGuid();
		var strFormattedGUID = wizard.FormatGuid(strRawGUID, Format2);
		wizard.AddSymbol("FILTER_GUID", strFormattedGUID);
		
		strFormattedGUID = wizard.FormatGuid(strRawGUID, Format1);	
		wizard.AddSymbol("FILTER_GUID_REG", strFormattedGUID);
		
		strRawGUID = wizard.CreateGuid();
		strFormattedGUID = wizard.FormatGuid(strRawGUID, Format2);	
		wizard.AddSymbol("FILTER_PROPERTY_PAGE_GUID", strFormattedGUID);

		strFormattedGUID = wizard.FormatGuid(strRawGUID, Format1);	
		wizard.AddSymbol("FILTER_PROPERTY_PAGE_GUID_REG", strFormattedGUID);

		strRawGUID = wizard.CreateGuid();
		strFormattedGUID = wizard.FormatGuid(strRawGUID, Format2);	
		wizard.AddSymbol("FILTER_INTERFACE_GUID", strFormattedGUID);
   
		strFormattedGUID = wizard.FormatGuid(strRawGUID, Format1);	
		wizard.AddSymbol("FILTER_INTERFACE_GUID_REG", strFormattedGUID);

		var strTextStream = InfFile.OpenAsTextStream(1, -2);
		while (!strTextStream.AtEndOfStream)
		{
			strTpl = strTextStream.ReadLine();
			if (strTpl != '')
			{
				strName = strTpl;
				var strTarget = GetTargetName(strName, strProjectName);
				var strTemplate = strTemplatePath + '\\' + strTpl;
				var strFile = strProjectPath + '\\' + strTarget;

				var bCopyOnly = false;  //"true" will only copy the file from strTemplate to strTarget without rendering/adding to the project
				var strExt = strName.substr(strName.lastIndexOf("."));
				if(strExt==".bmp" || strExt==".ico" || strExt==".gif" || strExt==".rtf" || strExt==".css")
					bCopyOnly = true;
				wizard.RenderTemplate(strTemplate, strFile, bCopyOnly);
				proj.Object.AddFile(strFile);
			}
		}
		strTextStream.Close();
	}
	catch(e)
	{
		throw e;
	}
}
