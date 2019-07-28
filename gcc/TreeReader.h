#include <vector>
#include <map>
#include <string>
#include "Module.h"
#include "StructureParser.h"

class TreeReader {
  private:
    int tab_space;
    std::map<std::string, StructureParser> modules;
    std::string html(std::string tag);
    Module inner;
    std::string styles;
    std::string body;
  public:
    TreeReader(Module rootModule, std::map<std::string, StructureParser> modules);
    void withdraw(Module module);
    void drawtab();
};
