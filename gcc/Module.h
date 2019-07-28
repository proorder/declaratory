#include <map>
#ifndef MODULE_H
#define MODULE_H

struct Module {
  std::string name;
  bool is_text = false;
  std::map<std::string, std::string> styles;
  std::map<std::string, std::vector<std::string> > scripts;
  std::map<std::string, std::string> attributes;
  std::vector<Module> children;
  std::string hash;
};

#endif
