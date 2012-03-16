#include "cmCodeLiteTargetGenerator.h"
#include "cmGlobalCodeLiteGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmLocalGenerator.h"
#include "cmCustomCommandGenerator.h"
cmCodeLiteTargetGenerator::cmCodeLiteTargetGenerator(cmTarget* target, cmGlobalCodeLiteGenerator* gg )
{
	this->Target = target;
	this->GlobalGenerator = gg;
	Makefile = Target->GetMakefile();
	doc = new TiXmlDocument();
}
cmCodeLiteTargetGenerator::~cmCodeLiteTargetGenerator()
{
	delete doc;
}
void cmCodeLiteTargetGenerator::Generate()
{
	TiXmlElement *RootElement = new TiXmlElement("CodeLite_Project");
	doc->LinkEndChild(RootElement);

	switch(Target->GetType())
	{
	case cmTarget::EXECUTABLE:
	case cmTarget::STATIC_LIBRARY:
	case cmTarget::SHARED_LIBRARY:
	case cmTarget::UTILITY:
		{
			std::string outputDir=Makefile->GetStartOutputDirectory();
			std::string projectName = Target->GetName() ;
			std::string projectPath = outputDir+"/" + projectName ;
			projectPath+=".project";
			RootElement->SetAttribute("Name", projectName.c_str());
			RootElement->SetAttribute("InternalType", "");

			WriteGroupSources(RootElement);
			RootElement->LinkEndChild( WriteSettings() );


			cmTargetDependSet const & depends = Makefile->GetLocalGenerator()->GetGlobalGenerator()->GetTargetDirectDepends(*Target);

			const char* default_order[] = {"Release", "MinSizeRel",
				"RelWithDebugInfo", "Debug", 0};
			for(const char** c = default_order; *c; ++c)
			{
				TiXmlElement *node = new TiXmlElement("Dependencies");
				node->SetAttribute("Name",*c );
				RootElement->LinkEndChild(node);
				for (cmTargetDependSet::const_iterator it = depends.begin() ; it!=depends.end() ; it++)
				{
					//(*it)->GetName()
					if(std::string( (*it)->GetName() ) == "ClangDiagnosticAnalysis")
					{
						int aa = 0;
					}
					TiXmlElement* Project = new TiXmlElement("Project");
					Project->SetAttribute("Name",(*it)->GetName());
					node->LinkEndChild(Project);
				}
			}

			doc->SaveFile(projectPath.c_str());
		}
		break;
	}


}
void cmCodeLiteTargetGenerator::WriteGroupSources(TiXmlElement* parent)
{
	// find all virtual directories for this target
	std::map< std::string , TiXmlElement* > groupSources;
	std::vector<cmSourceGroup>  sourceGroups = Makefile->GetSourceGroups();
	const std::vector<cmSourceFile*>& sources=Target->GetSourceFiles();

	for (std::vector<cmSourceFile*>::const_iterator it = sources.begin() ; it!=sources.end() ; it++)
	{
		std::string const& path = (*it)->GetFullPath() ;
		cmSourceGroup& sourceGroup =  Makefile->FindSourceGroup(path.c_str() , sourceGroups);
		std::string filter = sourceGroup.GetFullName();
		TiXmlElement *node = groupSources[filter];
		if( node == NULL )
		{
			node = new TiXmlElement("VirtualDirectory");
			node->SetAttribute("Name", filter.c_str() );
			parent->LinkEndChild(node);
			groupSources[filter] = node;
		}
		TiXmlElement* file = new TiXmlElement("File");
		file->SetAttribute("Name",path.c_str() );
		node->LinkEndChild(file);
	}

}
TiXmlElement* cmCodeLiteTargetGenerator::WriteSettings()
{
	TiXmlElement* setting = new TiXmlElement("Settings");
	std::string Type;
	switch(Target->GetType())
	{
	case cmTarget::EXECUTABLE: {Type = "Executable"; }break;
	case cmTarget::STATIC_LIBRARY: {Type = "Static Library";}break;
	case cmTarget::SHARED_LIBRARY: {Type = "Dynamic Library";}break;
	case cmTarget::MODULE_LIBRARY: {Type = "Dynamic Library";}break;
	default:  // intended fallthrough
		break;
	}

	setting->SetAttribute("Type", Type.c_str() );

	TiXmlElement* globalSettings = new TiXmlElement("GlobalSettings");
	setting->LinkEndChild(globalSettings);

	TiXmlElement* compiler = new TiXmlElement("Compiler");
	compiler->SetAttribute("Options","");
	compiler->SetAttribute("C_Options","");
	globalSettings->LinkEndChild(compiler);

	TiXmlElement* includePath = new TiXmlElement("IncludePath");
	includePath->SetAttribute("Value",".");
	compiler->LinkEndChild(includePath);

	TiXmlElement* Linker = new TiXmlElement("Linker");
	Linker->SetAttribute("Options","");
	globalSettings->LinkEndChild(Linker);

	TiXmlElement* LibraryPath = new TiXmlElement("LibraryPath");

	LibraryPath->SetAttribute("Value",".");
	Linker->LinkEndChild(LibraryPath);

	TiXmlElement* ResourceCompiler = new TiXmlElement("ResourceCompiler");
	ResourceCompiler->SetAttribute("Value","");
	globalSettings->LinkEndChild(ResourceCompiler);
	
	const char* default_order[] = {"Release", "MinSizeRel",
		"RelWithDebugInfo", "Debug", 0};
	for(const char** c = default_order; *c; ++c)
	{
		setting->LinkEndChild( WriteConfiguration(*c,Type) );
	}
	
	return setting;

}
TiXmlElement* cmCodeLiteTargetGenerator::WriteConfiguration(std::string const& name , std::string const& type)
{
	TiXmlElement* Configuration = new TiXmlElement("Configuration");

	std::string CompilerType = "gnu g++";
	std::string DebuggerType = "GNU gdb debugger";
	
	Configuration->SetAttribute("Name",name);
	Configuration->SetAttribute("CompilerType",CompilerType.c_str());
	Configuration->SetAttribute("DebuggerType",DebuggerType.c_str());
	Configuration->SetAttribute("Type",type.c_str());
	Configuration->SetAttribute("BuildCmpWithGlobalSettings","append" );
	Configuration->SetAttribute("BuildLnkWithGlobalSettings","append" );
	Configuration->SetAttribute("BuildResWithGlobalSettings","append" );

	Configuration->LinkEndChild( WriteCompiler( Target->GetType() !=cmTarget::UTILITY ) );

	Configuration->LinkEndChild( WriteLinker( Target->GetType() !=cmTarget::UTILITY ) );

	TiXmlElement* ResourceCompiler = new TiXmlElement("ResourceCompiler");
	ResourceCompiler->SetAttribute("Options","append" );
	ResourceCompiler->SetAttribute("Required","no" );
	Configuration->LinkEndChild(ResourceCompiler);

	std::string config ;
	switch(Target->GetType())
	{
		case cmTarget::EXECUTABLE: 
			{
				config = "RUNTIME_OUTPUT_DIRECTORY";

			}
			break;
		case cmTarget::STATIC_LIBRARY: 
		case cmTarget::SHARED_LIBRARY: 
			{ 
				config = "LIBRARY_OUTPUT_DIRECTORY";

			}
			break;
		default:  // intended fallthrough
			break;
	}
	//Target->GetProperty()
	std::string OutputDir;
	std::string OutputFile ;
	if(!config.empty())
	{
		std::string config1 = config + "_"+ cmSystemTools::UpperCase(name) ;
		const char * dir = Target->GetProperty( config1.c_str() );
		if(!dir)
			dir = Target->GetProperty( config.c_str() );

		OutputDir = dir != NULL ? dir : "";

	}

	OutputFile =  OutputDir +"/"  + Target->GetFullName();
	std::string IntermediateDirectory =  Makefile->GetStartOutputDirectory();
	TiXmlElement* General = new TiXmlElement("General");
	General->SetAttribute("OutputFile",OutputFile );
	General->SetAttribute("IntermediateDirectory",IntermediateDirectory );
	General->SetAttribute("Command",Target->GetName() );
	General->SetAttribute("CommandArguments","");
	General->SetAttribute("DebugArguments","");
	General->SetAttribute("WorkingDirectory",OutputDir);
	General->SetAttribute("PauseExecWhenProcTerminates","yes");

	Configuration->LinkEndChild(General);

	TiXmlElement* Environment = new TiXmlElement("Environment");

	Environment->SetAttribute("EnvVarSetName" ,"&lt;Use Defaults&gt;" );

	Environment->SetAttribute("DbgSetName" ,"&lt;Use Defaults&gt;" );
	Configuration->LinkEndChild(Environment);

	TiXmlElement* Debugger = new TiXmlElement("Debugger");
	Debugger->SetAttribute("IsRemote" ,"no" );
	Debugger->SetAttribute("RemoteHostName","");
	Debugger->SetAttribute("RemoteHostPort" ,"");
	Debugger->SetAttribute("DebuggerPath" ,"");;

	TiXmlElement* PostConnectCommands = new TiXmlElement("PostConnectCommands");
	TiXmlElement* StartupCommands = new TiXmlElement("StartupCommands");

	Debugger->LinkEndChild(PostConnectCommands);
	Debugger->LinkEndChild(StartupCommands);

	Configuration->LinkEndChild(Debugger);

	
	TiXmlElement* PreBuild  = WriteBuildCommands("PreBuild" , Target->GetPreBuildCommands() );
	WriteCustomCommands(PreBuild);
	Configuration->LinkEndChild( PreBuild  );
	Configuration->LinkEndChild(  WriteBuildCommands("PostBuild" , Target->GetPostBuildCommands() ) );

	//<Command Enabled="yes">dir</Command>

	Configuration->LinkEndChild( WriteCustomBuild() );

	TiXmlElement* AdditionalRules = new TiXmlElement("AdditionalRules");
	
	TiXmlElement* CustomPreBuild = new TiXmlElement("CustomPreBuild");
	TiXmlElement* CustomPostBuild = new TiXmlElement("CustomPostBuild");
	AdditionalRules->LinkEndChild(CustomPreBuild);
	AdditionalRules->LinkEndChild(CustomPostBuild);
	
	Configuration->LinkEndChild( AdditionalRules );

	TiXmlElement* Completion = new TiXmlElement("Completion");
	TiXmlElement* ClangCmpFlags = new TiXmlElement("ClangCmpFlags");
	TiXmlElement* ClangPP = new TiXmlElement("ClangPP");
	TiXmlElement* SearchPaths = new TiXmlElement("SearchPaths");
	Completion->LinkEndChild(ClangCmpFlags);
	Completion->LinkEndChild(ClangPP);
	Completion->LinkEndChild(SearchPaths);

	Configuration->LinkEndChild( Completion );

	return Configuration;
}
TiXmlElement* cmCodeLiteTargetGenerator::WriteBuildCommands(std::string const & buildType, std::vector<cmCustomCommand> & commands)
{
	TiXmlElement* Build = new TiXmlElement( buildType );

	for (std::vector<cmCustomCommand>::const_iterator it = commands.begin() ; it !=commands.end();it++)
	{
		const cmCustomCommandLines& commandLines  = it->GetCommandLines();
		for(cmCustomCommandLines::const_iterator cl = commandLines.begin() ; cl!=commandLines.end();cl++ )
		{
			std::string cmd;
			for (std::vector<std::string>::const_iterator cc = cl->begin() ; cc !=cl->end();cc++)
			{
				cmd += " " + (*cc);
			}
			TiXmlElement* Command = new TiXmlElement( "Command" );
			Command->SetAttribute("Enabled" ,"yes");
            TiXmlText *Content = new TiXmlText(cmd);
            Command->LinkEndChild(Content);
			Build->LinkEndChild(Command);
		}

	}
	return Build;
}
TiXmlElement* cmCodeLiteTargetGenerator::WriteCustomBuild()
{
	TiXmlElement* CustomBuild = new TiXmlElement("CustomBuild");

	std::string EnabledCustomBuild = "no";
	CustomBuild->SetAttribute("Enabled" ,EnabledCustomBuild);

	CustomBuild->LinkEndChild( new TiXmlElement("RebuildCommand") );
	CustomBuild->LinkEndChild( new TiXmlElement("CleanCommand") );
	CustomBuild->LinkEndChild( new TiXmlElement("BuildCommand") );
	CustomBuild->LinkEndChild( new TiXmlElement("RebuildCommand") );
	CustomBuild->LinkEndChild( new TiXmlElement("PreprocessFileCommand") );
	CustomBuild->LinkEndChild( new TiXmlElement("SingleFileCommand") );
	CustomBuild->LinkEndChild( new TiXmlElement("MakefileGenerationCommand") );

	TiXmlElement* ThirdPartyToolName =  new TiXmlElement("ThirdPartyToolName") ;
	CustomBuild->LinkEndChild(ThirdPartyToolName);
	TiXmlText *Content = new TiXmlText("None");
	ThirdPartyToolName->LinkEndChild(Content);
	

	CustomBuild->LinkEndChild( new TiXmlElement("WorkingDirectory") );

	return CustomBuild;
}
TiXmlElement* cmCodeLiteTargetGenerator::WriteCompiler(bool isEnable)
{

	const char* cflags = Makefile->GetProperty("COMPILE_FLAGS");

	TiXmlElement* Compiler = new TiXmlElement("Compiler");
	
	std::string CompilerOptions ;
	std::string C_Options;
	std::string Required = isEnable ? "yes" : "no";
	std::string PreCompiledHeader = "" ;
	std::string PCHInCommandLine = "yes";
	std::string UseDifferentPCHFlags = "yes";
	if(cflags)
		CompilerOptions+=cflags;
	Compiler->SetAttribute("Options", CompilerOptions.c_str() );
	Compiler->SetAttribute("C_Options", C_Options.c_str() );
	Compiler->SetAttribute("Required", Required.c_str() );
	Compiler->SetAttribute("PreCompiledHeader", PreCompiledHeader.c_str() );
	Compiler->SetAttribute("PCHInCommandLine", PCHInCommandLine.c_str() );
	Compiler->SetAttribute("UseDifferentPCHFlags", UseDifferentPCHFlags.c_str() );
	Compiler->SetAttribute("PCHFlags","" );

	for (std::vector<std::string>::const_iterator it = Makefile->GetIncludeDirectories().begin() 
		; it !=Makefile->GetIncludeDirectories().end();it++)
	{
		TiXmlElement* IncludePath = new TiXmlElement("IncludePath");
		IncludePath->SetAttribute("Value",*it );
		Compiler->LinkEndChild(IncludePath);
	}
	const char* cdefs = Makefile->GetProperty("COMPILE_DEFINITIONS");
	if(cdefs)
	{
		// Expand the list.
		std::vector<std::string> defs;
		cmSystemTools::ExpandListArgument(cdefs, defs);
		for(std::vector<std::string>::const_iterator di = defs.begin();
			di != defs.end(); ++di)
		{
			TiXmlElement* Preprocessor = new TiXmlElement("Preprocessor");
			Preprocessor->SetAttribute("Value", *di );
			Compiler->LinkEndChild(Preprocessor);
		}
	}
	return Compiler;
}
template<typename Iterator>
void WriteGroups(TiXmlElement* node , std::string const& groupName , Iterator begin ,Iterator end)
{
	for (Iterator it = begin ; it != end;it++)
	{
		TiXmlElement* group = new TiXmlElement(groupName);
		group->SetAttribute("Value", *it );
		node->LinkEndChild(group);
	}
}


TiXmlElement* cmCodeLiteTargetGenerator::WriteLinker(bool isEnable )
{
	const char* linkflags =  Target->GetProperty("LINK_FLAGS");
	std::string Options;
	if(linkflags)
		{
			// Expand the list.
			std::vector<std::string> defs;
			cmSystemTools::ExpandListArgument(linkflags, defs);
			for(std::vector<std::string>::const_iterator di = defs.begin();
				di != defs.end(); ++di)
			{
				Options+= *di +";";
			}
		}
	TiXmlElement* Linker = new TiXmlElement("Linker");
	Linker->SetAttribute("Options", Options );
	Linker->SetAttribute("Required", isEnable ? "yes" : "no" );


	//  LibraryPath 

	WriteGroups(Linker ,"LibraryPath", Target->GetLinkDirectories().begin() ,Target->GetLinkDirectories().end() );

	std::vector<std::string> libs;
	std::set<std::string> libpaths;
	GetLibraries(Target,libs ,libpaths);

	WriteGroups(Linker ,"LibraryPath", libpaths.begin(),libpaths.end() );

	WriteGroups(Linker ,"Library", libs.begin(),libs.end() );
	return Linker;
	
}

//void cmCodeLiteTargetGenerator::WriteGroups(TiXmlElement* node , std::string const& groupName ,  std::vector<std::string> const& groups)
//{
//	for (std::vector<std::string>::const_iterator it = groups.begin() ; it !=groups.end();it++)
//	{
//		TiXmlElement* group = new TiXmlElement(groupName);
//		group->SetAttribute("Value", *it );
//		node->LinkEndChild(group);
//	}
//}
void cmCodeLiteTargetGenerator::GetLibraries(cmTarget* target ,std::vector<std::string>& outlibs ,	std::set<std::string>& libpaths)
{
	// find link libraries
	const cmTarget::LinkLibraryVectorType& libs = target->GetLinkLibraries();

	std::string libOptions;
	std::string libDebugOptions;
	std::string libOptimizedOptions;
	for(cmTarget::LinkLibraryVectorType::const_iterator it = libs.begin(); it != libs.end(); ++it)
	{
		// NEVER LINK STATIC LIBRARIES TO OTHER STATIC LIBRARIES
		if ( target->GetType() != cmTarget::STATIC_LIBRARY 
			&& target->GetType() != cmTarget::MODULE_LIBRARY )
		{
			// Compute the proper name to use to link this library.
			std::string lib;
			std::string libDebug;
			std::string config = "Debug";
			cmGlobalGenerator* gg = Makefile->GetLocalGenerator()->GetGlobalGenerator() ;
			cmTarget* tgt = gg->FindTarget(0, it->first.c_str());
			if(tgt)
			{
				std::string dir = tgt->GetDirectory();
				const char* output = tgt->GetProperty("LIBRARY_OUTPUT_DIRECTORY");
				libpaths.insert(dir);
				lib = cmSystemTools::GetFilenameWithoutExtension
					(tgt->GetName());
				libDebug = cmSystemTools::GetFilenameWithoutExtension
					(tgt->GetName() );

			}
			else
			{
				lib = it->first.c_str();
				libDebug = it->first.c_str();

			}

			lib = Makefile->GetLocalGenerator()->ConvertToOptionallyRelativeOutputPath(lib.c_str());
			libDebug = 
				Makefile->GetLocalGenerator()->ConvertToOptionallyRelativeOutputPath(libDebug.c_str());

			if (it->second == cmTarget::GENERAL)
			{
				libOptions += " ";
				libOptions += lib;

			}
			if (it->second == cmTarget::DEBUG)
			{
				libDebugOptions += " ";
				libDebugOptions += lib;

			}
			if (it->second == cmTarget::OPTIMIZED)
			{
				libOptimizedOptions += " ";
				libOptimizedOptions += lib;

			}
			outlibs.push_back(lib);
		}
	}
}
void cmCodeLiteTargetGenerator::WriteCustomCommands(TiXmlElement* node)
{
	this->SourcesVisited.clear();
	std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
	for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
		source != sources.end(); ++source)
	{
		cmSourceFile* sf = *source;
		this->WriteCustomCommand(node ,sf);
	}
}

//----------------------------------------------------------------------------
void cmCodeLiteTargetGenerator::WriteCustomCommand(TiXmlElement* node ,cmSourceFile* sf)
{
	if(this->SourcesVisited.insert(sf).second)
	{
		if(std::vector<cmSourceFile*> const* depends =
			this->Target->GetSourceDepends(sf))
		{
			for(std::vector<cmSourceFile*>::const_iterator di = depends->begin();
				di != depends->end(); ++di)
			{
				this->WriteCustomCommand(node,*di);
			}
		}
		if(cmCustomCommand * command = sf->GetCustomCommand())
		{
			//this->WriteString("<ItemGroup>\n", 1);
			this->WriteCustomRule(sf, *command);
			//this->WriteString("</ItemGroup>\n", 1);
			std::string configName = "Debug";
			cmCustomCommandGenerator ccg(*command, configName.c_str(), this->Makefile);
			//ccg.GetNumberOfCommands()
			for(unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c)
			{
				// Build the command line in a single string.
				std::string cmd = ccg.GetCommand(c);
				if (cmd.size())
				{
					ccg.AppendArguments(c, cmd);

					TiXmlElement* Command = new TiXmlElement( "Command" );
					Command->SetAttribute("Enabled" ,"yes");
					TiXmlText *Content = new TiXmlText(cmd);
					Command->LinkEndChild(Content);
					node->LinkEndChild(Command);
				}
			}
		}
	}
}
void 
	cmCodeLiteTargetGenerator::WriteCustomRule(cmSourceFile* source,
	cmCustomCommand const & 
	command)
{
	std::string sourcePath = source->GetFullPath();
	// the rule file seems to need to exist for vs10
	if (source->GetExtension() == "rule")
	{
		if(!cmSystemTools::FileExists(sourcePath.c_str()))
		{
			// Make sure the path exists for the file
			std::string path = cmSystemTools::GetFilenamePath(sourcePath);
			cmSystemTools::MakeDirectory(path.c_str());
			std::ofstream fout(sourcePath.c_str());
			if(fout)
			{
				fout << "# generated from CMake\n";
				fout.flush();
				fout.close();
			}
			else
			{
				std::string error = "Could not create file: [";
				error +=  sourcePath;
				error += "]  ";
				cmSystemTools::Error
					(error.c_str(), cmSystemTools::GetLastSystemError().c_str());
			}
		}
	}
	
}
void cmCodeLiteTargetGenerator::AppendCustomCommand(std::vector<std::string>& commands,
	const cmCustomCommand& cc,
	std::string const& ConfigurationName)
{
	
	// if the command specified a working directory use it.
	const char* dir  = this->Makefile->GetStartOutputDirectory();
	const char* workingDir = cc.GetWorkingDirectory();
	if(workingDir)
	{
		dir = workingDir;
	}

	cmCustomCommandGenerator ccg(cc, ConfigurationName.c_str(),
		this->Makefile);

	// Add each command line to the set of commands.
	std::vector<std::string> commands1;
	for(unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c)
	{
		// Build the command line in a single string.
		std::string cmd = ccg.GetCommand(c);
		if (cmd.size())
		{
			// Use "call " before any invocations of .bat or .cmd files
			// invoked as custom commands in the WindowsShell.

			commands1.push_back(cmd);
		}
	}

	// Setup the proper working directory for the commands.
	//this->CreateCDCommand(commands1, dir, relative);

	// push back the custom commands
	commands.insert(commands.end(), commands1.begin(), commands1.end());
}
