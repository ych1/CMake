/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalCodeLiteGenerator_h
#define cmGlobalCodeLiteGenerator_h

#include "cmGlobalGenerator.h"
#include "cmSourceGroup.h"
#include "cmGlobalUnixMakefileGenerator3.h"

#ifdef WIN32
#include "cmGlobalMinGWMakefileGenerator.h"
typedef cmGlobalMinGWMakefileGenerator cmClobalCodeLiteGeneratorBase;
#else
typedef cmGlobalUnixMakefileGenerator3 cmClobalCodeLiteGeneratorBase ;
#endif

#include "tinyxml/tinyxml.h"
class cmLocalGenerator;
class XMLDocument;
class XMLNode;

class cmClobalCodeLiteGenerator : public cmClobalCodeLiteGeneratorBase
{
public:
  cmClobalCodeLiteGenerator();

  static cmGlobalGenerator* New() {
	  return new cmClobalCodeLiteGenerator; }
  virtual const char* GetName() const {
	  return cmClobalCodeLiteGenerator::GetActualName();}
  static const char* GetActualName()   { return "CodeLite";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry, 
                                const char* fullName) const;

  virtual void Generate();

  virtual void EnableLanguage(std::vector<std::string>const& languages,
	  cmMakefile *, bool optional);
   virtual cmLocalGenerator *CreateLocalGenerator();
private:
	void CreateProjectFile(const std::vector<cmLocalGenerator*>& lgs ,std::vector<TiXmlElement*> &projects);

	std::string GetCompiler( const cmMakefile* mf);

	std::map<std::string , TiXmlElement*> WorkspaceConfigurations;

	TiXmlDocument* doc;
};

#endif
