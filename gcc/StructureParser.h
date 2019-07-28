#include <vector>
#include <map>
#include <string>
#include "Module.h"
#ifndef STRUCTUREPARSER_H
#define STRUCTUREPARSER_H

class StructureParser {
  private:
    int index;
    Module rootModule;
    std::vector<std::string> args;
    std::vector<std::string> lines;
    std::vector<std::string> imports;
    std::map<std::string, StructureParser> modules;
  public:
    std::map< std::string, std::map<std::string, std::string> > styles;
    std::map< std::string, std::map<std::string, std::vector<std::string> > > scripts;
    std::string name;
    StructureParser(std::vector<std::string> lines);
    Module getRootModule();
    void parseImports();
    void executeImports();
    void findModule();
    void parseModule();
    void parseClass(std::string class_name);
    void parseContent(Module &module, int &position);
    std::map<std::string, std::string> parseAttributes(std::string attributes_str);
    std::string parseModuleName(std::string line);
    std::string generateID();
    std::vector<std::string> getImports();
    std::map<std::string, StructureParser> getModules();
};

#endif
