#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <map>
#include "StructureParser.h"
#include "TreeReader.h"

char arr_en[36] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

StructureParser::StructureParser(std::vector<std::string> lines) {
  this->index = 0;
  this->lines = lines;
  this->parseImports();
  this->executeImports();
  this->findModule();
  this->parseModule();
}

void StructureParser::parseImports() {
  const std::regex re("import\\s(.*)$");
  for (; this->index < this->lines.size(); this->index++) {
    std::smatch match;
    if (regex_search(this->lines[this->index], match, re)) {
      this->imports.push_back(match[1]);
    } else {
      break;
    }
  }
}

void StructureParser::executeImports() {
  for (std::string name : this->imports) {
    std::ifstream mFile ("structure/components/" + name + ".strc");
    if (mFile.is_open()) {
      std::vector<std::string> lines;
      std::string line;
      const std::regex tab_rex("^[\\s\\t]+");
      while (getline(mFile, line)) {
        lines.push_back(regex_replace(line, tab_rex, ""));
      }
      mFile.close();
      StructureParser parser(lines);
      std::map<std::string, StructureParser> modules = parser.getModules();
      this->modules.insert(modules.begin(), modules.end());
      this->modules.insert(std::pair<std::string, StructureParser> (parser.name, parser));
    }
  }
}

void StructureParser::findModule() {
  const std::regex string_re("\\{");
  for (; this->index < this->lines.size(); this->index++) {
    std::smatch string_match;
    if (!this->lines[this->index].empty()) {
      if (regex_search(this->lines[this->index], string_match, string_re)) {
        this->name = this->parseModuleName(this->lines[this->index]);
        const std::regex args_re("\\((.*)\\)");
        std::smatch args_match;
        if (regex_search(this->lines[this->index], args_match, args_re)) {
          std::string res_string = args_match[1];
          int pos = 0;
          while (true) {
            if (res_string.find(",", pos) != -1) {
              this->args.push_back(
                res_string.substr(pos, res_string.find(",", pos)-pos)
              );
            } else {
              this->args.push_back(res_string.substr(pos));
              break;
            }
            pos = res_string.find(",", pos) + 2;
          }
        }
        break;
      }
    }
  }
}

std::string StructureParser::parseModuleName(std::string line) {
  const std::regex re("([\\w\\-]+)");
  std::smatch match;
  if (regex_search(line, match, re)) {
    return match[1];
  } else {
    return "";
  }
}

void StructureParser::parseModule() {
  const std::regex func_re("(:\\w+:\\s\\()");
  const std::regex state_re("(:\\w+:\\s\\{)");
  const std::regex class_re("^\\.([\\w\\d\\-]+)");
  const std::regex content_re("<\\w+>");
  const std::regex tab_re("^[\\s\\t]+");
  std::smatch match;
  for (; this->index < this->lines.size(); this->index++) {
    std::string result_line = this->lines[this->index];
    if (regex_search(result_line, match, func_re)) {
      std::cout << "Function founded" << '\n';
    } else if (regex_search(result_line, match, state_re)) {
      std::cout << "State changed" << '\n';
    } else if (regex_search(result_line, match, class_re)) {
      // Парсинг CSS по найденному классу начинающегося с: .
      this->index++;
      this->parseClass(match[1]);
    } else if (regex_search(result_line, match, content_re)) {
      this->index++;
      this->rootModule.name = this->name;
      int position = 0;
      this->parseContent(this->rootModule, position);
    }
  }
}

Module StructureParser::getRootModule() {
  return this->rootModule;
}

void StructureParser::parseClass(std::string class_name) {
  const std::regex js_property_re("&on:(\\w+)");
  const std::regex close_re("\\}");
  const std::regex prop_re("^([\\w\\-]+):\\s(.*);$");
  std::smatch match;
  std::map<std::string, std::string> styles;
  std::map<std::string, std::vector<std::string> > scripts;
  for (; this->index < this->lines.size(); this->index++) {
    std::string result_line = this->lines[this->index];

    if (regex_search(result_line, match, js_property_re)) {
      // match[1] хранит событие
      std::string event = match[1];
      std::vector<std::string> lines;
      this->index++;
      for (; this->index < this->lines.size(); this->index++) {
        // Совершать проверки на JS и CSS
        if (regex_search(this->lines[this->index], match, close_re)) {
          break;
        } else {
          lines.push_back(this->lines[this->index]);
        }
      }
      scripts.insert(
        std::pair <std::string, std::vector<std::string> > (
          event,
          lines
        )
      );
    } else if (regex_search(result_line, match, close_re)) {
      break;
    } else {
      // Искать свойство
      std::smatch prop_match;
      if (regex_search(this->lines[this->index], prop_match, prop_re)) {
        styles.insert(std::pair<std::string, std::string> (prop_match[1], prop_match[2]));
      }
    }
  }
  this->scripts.insert(std::pair< std::string, std::map<std::string, std::vector<std::string> > > (class_name, scripts));
  this->styles.insert(std::pair< std::string, std::map<std::string, std::string> > (class_name, styles));
}

void StructureParser::parseContent(Module &module, int &position) {
  const std::regex start_rex("^([^<]*?)(<|$)");
  const std::regex close_rex("^</(.+)>(.|$)");
  const std::regex module_rex("^<([\\w\\-]+)\\s?([^/]*?)>(.|$)");
  const std::regex cmodule_rex("^<([\\w\\-]+)\\s?(.*?)/>(.|$)");
  for (; this->index < this->lines.size(); this->index++) {
    while (position != this->lines[this->index].size()) {
      std::string line = this->lines[this->index].substr(position);
      std::smatch match;
      Module new_module;
      if (regex_search(line, match, start_rex) && !std::string (match[1]).empty()) {
        // Находит Текст до тега, выводит текст, затем позицию
        new_module.name = match[1];
        new_module.is_text = true;
        module.children.push_back(new_module);
        position = position + match.position(2);
      } else if (regex_search(line, match, close_rex)) {
        position = position + match.position(2);
        return;
      } else if (regex_search(line, match, module_rex)) {
        // Находит модуль с содержимым
        new_module.name = match[1];
        new_module.hash = generateID();
        if (this->styles.find(match[1]) != this->styles.end()) {
          new_module.styles = this->styles.find(match[1])->second;
        }
        if (this->scripts.find(match[1]) != this->scripts.end()) {
          new_module.scripts = this->scripts.find(match[1])->second;
        }
        if (!std::string (match[2]).empty()) {
          new_module.attributes = this->parseAttributes(match[2]);
        }
        position = position + match.position(3);
        this->parseContent(new_module, position);
        module.children.push_back(new_module);
      } else if (regex_search(line, match, cmodule_rex)) {
        // Находит модуль без содержимого
        new_module.name = match[1];
        new_module.hash = generateID();
        if (this->styles.find(match[1]) != this->styles.end()) {
          new_module.styles = this->styles.find(match[1])->second;
        }
        if (this->scripts.find(match[1]) != this->scripts.end()) {
          new_module.scripts = this->scripts.find(match[1])->second;
        }
        if (!std::string (match[2]).empty()) {
          new_module.attributes = this->parseAttributes(match[2]);
        }
        position = position + match.position(3);
        module.children.push_back(new_module);
      } else {
        break;
      }
    }
    position = 0;
  }
}

std::string StructureParser::generateID() {
  std::string id = "";
  for (int i = 0; i < 10; i++) {
    id += arr_en[std::rand() % 36];
  }
  return id;
}

std::map<std::string, std::string> StructureParser::parseAttributes(std::string attributes_str) {
  const std::regex space_rex("^[\\s\\t]+");
  const std::regex attribute_rex("^(\\w+)=?\"?([^\"]*)\"?(.|$)");
  std::string line = regex_replace(attributes_str, space_rex, "");
  std::map<std::string, std::string> attributes;
  while (!line.empty()) {
    std::smatch match;
    if (regex_search(line, match, attribute_rex)) {
      std::string attr_name = match[1];
      std::string attr_value = match[2];
      attributes.insert( std::pair<std::string, std::string> (attr_name, attr_value) );
      line = line.substr(match.position(3));
    }
  }
  return attributes;
}

std::vector<std::string> StructureParser::getImports() {
  return this->imports;
}

std::map<std::string, StructureParser> StructureParser::getModules() {
  return this->modules;
}

