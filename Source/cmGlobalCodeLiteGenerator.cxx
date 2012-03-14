/*============================================================================
CMake - Cross Platform Makefile Generator
Copyright 2012-2012 chenghan yao.
Copyright 2004-2009 Kitware, Inc.
Copyright 2004 Alexander Neundorf (neundorf@kde.org)

Distributed under the OSI-approved BSD License (the "License");
see accompanying file Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License for more information.
============================================================================*/
#include "cmGlobalCodeLiteGenerator.h"
#include "cmCodeLiteTargetGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmCustomCommandGenerator.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Directory.hxx>
#include "cmGlobalMinGWMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
using namespace std;


//----------------------------------------------------------------------------
void cmClobalCodeLiteGenerator::GetDocumentation(cmDocumentationEntry& entry, const char*) const
{
	entry.Name = this->GetName();
	entry.Brief = "Generates CodeLite workspace & project files.";
	entry.Full =
		"Project files for CodeLite will be created in the top directory ...";


}

cmClobalCodeLiteGenerator::cmClobalCodeLiteGenerator()
	
{
//#if defined(_WIN32)
//	this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
//	//this->SupportedGlobalGenerators.push_back("NMake Makefiles");
//	// disable until somebody actually tests it:
//	this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
//#elif
//	this->SupportedGlobalGenerators.push_back("Unix Makefiles");
//#endif
	//cmake* cm = this->GlobalGenerator->GetCMakeInstance();
	//cm->DefineProperty
	//	("CodeLite", cmProperty::VARIABLE,
	//	"True when using the CodeLite",
	//	"Set to true when the target platform is the CodeLite, "
	//	"as opposed to the command line compiler.",
	//	false,
	//	"Variables That Describe the System");

}
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmClobalCodeLiteGenerator::CreateLocalGenerator()
{
	cmLocalGenerator* lg = cmGlobalMinGWMakefileGenerator::CreateLocalGenerator();

	return lg;
}
void cmClobalCodeLiteGenerator::EnableLanguage(std::vector<std::string>const& languages,
	cmMakefile *mf, bool optional)
{
	cmGlobalMinGWMakefileGenerator::EnableLanguage(languages,mf,optional);
}
void cmClobalCodeLiteGenerator::Generate()
{
	this->cmGlobalMinGWMakefileGenerator::Generate();

	// Hold root tree information for creating the workspace 
	std::string workspaceProjectName;
	std::string workspaceOutputDir;
	std::string workspaceFileName;
	std::string workspaceSourcePath;

	// loop projects and locate the root project.
	// and extract the information for creating the workspace 

	TiXmlDocument doc("CodeLite_Workspace");
	TiXmlElement *wsp = new TiXmlElement("CodeLite_Workspace");
	doc.LinkEndChild(wsp);

	for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
		it = this->GetProjectMap().begin();
		it!= this->GetProjectMap().end();
	++it)
	{
		const cmMakefile* mf =it->second[0]->GetMakefile();
		// 
		if (strcmp(mf->GetStartOutputDirectory(), mf->GetHomeOutputDirectory()) == 0)
		{
			workspaceOutputDir = mf->GetStartOutputDirectory();
			workspaceProjectName = mf->GetProjectName();
			workspaceSourcePath = mf->GetHomeDirectory();
			workspaceFileName = workspaceOutputDir+"/";
			workspaceFileName+= workspaceProjectName + ".workspace";

			wsp->SetAttribute("Name" ,workspaceProjectName);
			wsp->SetAttribute("Database" ,string("./") + workspaceProjectName + ".tags");
		}
	}

	TiXmlElement* BuildMatrix = new TiXmlElement("BuildMatrix");
	wsp->LinkEndChild(BuildMatrix);
	
	

	// for each sub project in the project create a codeblocks project
	std::vector<TiXmlElement*> projects ;
	for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
		it = this->GetProjectMap().begin();
		it!= this->GetProjectMap().end();
	++it)
	{
		// create a project file
		CreateProjectFile(it->second ,projects);
	
	}
	for (std::vector<TiXmlElement*>::const_iterator it = projects.begin() 
		; it !=projects.end();it++)
	{

		wsp->LinkEndChild( (*it) );
	}
	const char* default_order[] = {"Release", "MinSizeRel",
		"RelWithDebugInfo", "Debug", 0};
	for(const char** c = default_order; *c; ++c)
	{
		TiXmlElement* WorkspaceConfiguration = new TiXmlElement("WorkspaceConfiguration");
		WorkspaceConfiguration->SetAttribute("Name" ,*c);
		WorkspaceConfiguration->SetAttribute("Selected" , "yes");

		BuildMatrix->LinkEndChild(WorkspaceConfiguration);

		for (std::vector<TiXmlElement*>::const_iterator it = projects.begin() 
			; it !=projects.end();it++)
		{
			TiXmlElement* Project = new TiXmlElement("Project");
			Project->SetAttribute("Name",(*it)->Attribute("Name") );

			Project->SetAttribute("ConfigName", *c ) ;
			WorkspaceConfiguration->LinkEndChild(Project);
		}

	}

	doc.SaveFile(workspaceFileName.c_str() );
}

std::string cmClobalCodeLiteGenerator::GetCompiler( const cmMakefile* mf)
{
	std::string compilerIdVar = "CMAKE_CXX_COMPILER_ID";
	if (this->GetLanguageEnabled("CXX") == false)
	{
		compilerIdVar = "CMAKE_C_COMPILER_ID";
	}

	std::string compilerId = mf->GetSafeDefinition(compilerIdVar.c_str());
	std::string compiler = "gnu gcc";  // default to gcc
	if (compilerId == "GNU")
	{
		compiler = "gnu g++";
	}
	return compiler;

}
/* create the project file */
void cmClobalCodeLiteGenerator::CreateProjectFile(const std::vector<cmLocalGenerator*>& lgs ,std::vector<TiXmlElement*> &projects )
{
	for (std::vector<cmLocalGenerator*>::const_iterator it = lgs.begin() ; it!=lgs.end() ;it++)
	{
		cmMakefile* mf= (*it)->GetMakefile();
		std::string outputDir=mf->GetStartOutputDirectory();
		std::string projectName=mf->GetProjectName();

		cmTargets& targets=mf->GetTargets();

		for (cmTargets::iterator ti = targets.begin();
			ti != targets.end(); ti++)
		{

			switch(ti->second.GetType())
			{
			case cmTarget::EXECUTABLE:
			case cmTarget::STATIC_LIBRARY:
			case cmTarget::SHARED_LIBRARY:
			case cmTarget::UTILITY:
				{
					std::string projectName = ti->second.GetName() ;
					std::string projectPath = outputDir+"/" + projectName ;
					projectPath+=".project";
					////////////////////////////////////

					TiXmlElement* Project = new TiXmlElement("Project");
					Project->SetAttribute("Name",projectName);
					Project->SetAttribute("Path",projectPath);
					Project->SetAttribute("Active","no");
					projects.push_back(Project);
					
				
					cmCodeLiteTargetGenerator gen(&ti->second , 0);
					gen.Generate();

				}
				break;
			}
		}
	}

}


