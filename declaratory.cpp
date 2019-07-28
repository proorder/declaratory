#include <iostream>
#include <regex>
#include <fstream>
#include <dirent.h>
#include "gcc/StructureParser.h"
#include "gcc/TreeReader.h"
using namespace std;

StructureParser readFile(std::string file_name) {
  ifstream mFile (file_name);
  if (mFile.is_open()) {
    vector<string> lines;
    string line;
    const std::regex tab_rex("^[\\s\\t]+");
    while (getline(mFile, line)) {
      lines.push_back(regex_replace(line, tab_rex, ""));
    }
    mFile.close();
    StructureParser parser(lines);
    return parser;
  } else {
    cout << "Unable to open";
  }
}

int main() {
  StructureParser root = readFile("structure/base.strc");
  map<string, StructureParser> modules = root.getModules();

  DIR *d = opendir("./structure/pages");
  struct dirent *dir;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_name != string (".") && dir->d_name != string ("..")) {
        StructureParser module = readFile(string ("structure/pages/") + dir->d_name);
        map<string, StructureParser> moduleImports = module.getModules();
        modules.insert(moduleImports.begin(), moduleImports.end());
        modules.insert(pair<string, StructureParser> (module.name, module));
      }
    }
    closedir(d);
  }

  TreeReader reader(root.getRootModule(), modules);

  return 0;
}
