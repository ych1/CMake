/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCodeLiteTargetGenerator_h
#define cmCodeLiteTargetGenerator_h
#include "cmStandardIncludes.h"
#include "tinyxml/tinyxml.h"

class cmTarget;
class cmMakefile;
class cmGeneratedFileStream;
class cmSourceFile;
class cmCustomCommand;
class cmComputeLinkInformation;
class cmGlobalCodeLiteGenerator;
#include "cmSourceGroup.h"

class cmCodeLiteTargetGenerator
{
public:
	cmCodeLiteTargetGenerator(cmTarget* target, 
		cmGlobalCodeLiteGenerator* gg );
	~cmCodeLiteTargetGenerator();

	void Generate();

private:
	//void WriteGroups(TiXmlElement* node , std::string const& groupName ,  std::vector<std::string> const& groups);

	void WriteGroupSources(TiXmlElement* node);

	TiXmlElement* WriteSettings();
	TiXmlElement* WriteConfiguration(std::string const& name , std::string const& type);
	TiXmlElement* WriteCompiler(bool isEnable = true);
	TiXmlElement* WriteLinker(bool isEnable = true);
	TiXmlElement* WriteCustomBuild();
	TiXmlElement* WriteBuildCommands(std::string const & buildType, std::vector<cmCustomCommand> & commands);
	void WriteCustomCommands(TiXmlElement* node);
	void WriteCustomCommand(TiXmlElement* node ,cmSourceFile* sf);
	void WriteCustomRule(cmSourceFile* source,
		cmCustomCommand const & 
		command);
	void GetLibraries(cmTarget* target ,std::vector<std::string>& outlibs ,std::set<std::string>& libpaths);
	void AppendCustomCommand(std::vector<std::string>& commands,
		const cmCustomCommand& cc,
		std::string const& ConfigurationName);
private:
	cmTarget* Target;
	cmMakefile* Makefile;
	std::string Name;
	cmGlobalCodeLiteGenerator* GlobalGenerator;
	std::set<cmSourceFile*> SourcesVisited;
	TiXmlDocument* doc;
};

#endif