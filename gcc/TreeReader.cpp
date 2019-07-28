#include <iostream>
#include "TreeReader.h"

const std::string tags[130] = {
  "//", "a", "abbr", "acronym", "address", "applet", "area", "article", "aside", "audio", "b", "base", "basefont", "bdi", "bdo", "bgsound", "blockquote", "big", "body", "blink", "br", "button", "canvas", "caption", "center", "cite", "code", "col", "colgroup", "command", "comment", "datalist", "dd", "del", "details", "dfn", "dir", "div", "dl", "dt", "em", "embed", "fieldset", "figcaption", "figure", "font", "form", "footer", "frame", "frameset", "h1", "h2", "h3", "h4", "h5", "h6", "head", "header", "hgroup", "hr", "html", "i", "iframe", "img", "input", "ins", "isindex", "kbd", "keygen", "label", "legend", "li", "link", "main", "map", "marquee", "mark", "menu", "meta", "meter", "nav", "nobr", "noembed", "noframes", "noscript", "object", "ol", "optgroup", "option", "output", "p", "param", "plaintext", "pre", "progress", "q", "rp", "rt", "ruby", "s", "samp", "script", "section", "select", "small", "span", "source", "strike", "strong", "style", "sub", "summary", "sup", "table", "tbody", "td", "textarea", "tfoot", "th", "thead", "time", "title", "tr", "tt", "u", "ul", "var", "video", "wbr", "xmp"
};

const std::string without_end[6] = {
  "br", "hr", "img", "meta", "option", "input"
};

TreeReader::TreeReader(Module rootModule, std::map<std::string, StructureParser> modules) {
  this->modules = modules;
  this->tab_space = 0;
  rootModule.name = "body";
  this->withdraw(rootModule);
  std::cout << this->styles;
  std::cout << this->body;
}

void TreeReader::withdraw(Module module) {
  if (this->modules.find(module.name) != this->modules.end()) {
    this->inner = module;
    Module object = this->modules.find(module.name)->second.getRootModule();
    if (object.children.size() > 1) {
      // Хэш на элемент не работает
      for (int i = 0; i < object.children.size(); i++) {
        this->withdraw(object.children[i]);
      }
    } else {
      // Хэш элемента
      this->withdraw(object.children[0]);
    }
  } else  {
    if (!module.children.empty()) {
      this->drawtab();
      //----------------------------------
      // Вывод модуля с детьми
      //----------------------------------
      this->body += "<" + this->html(module.name);
      if (!module.styles.empty()) {
        this->styles += "  ." + module.hash + " {\n";
        for (const auto& [key, value]: module.styles) {
          this->styles += "    " + key + ": " + value + ";\n";
        }
        this->styles += "  }\n";
        this->body += " class=\"" + module.hash + "\"";
      }
      this->body += ">\n";
      this->tab_space++;
      for (int i = 0; i < module.children.size(); i++) {
        this->withdraw(module.children[i]);
      }
      this->tab_space--;
      this->drawtab();
      this->body += "</" + this->html(module.name) + ">\n";
    } else {
      if (!module.is_text) {
        if (std::find(std::begin(without_end), std::end(without_end), module.name) != std::end(without_end)) {
          //----------------------------------
          // Вывод html тэга без детей
          //----------------------------------
          this->drawtab();
          this->body += "<" + this->html(module.name);
          if (!module.styles.empty()) {
            this->body += " class=\"" + module.hash + "\"";
          }
          this->body += "/>\n";
        } else {
          if (module.name != "inner") {
            this->drawtab();
            this->body += "<" + this->html(module.name);
            if (!module.styles.empty()) {
              this->styles += "  ." + module.hash + " {\n";
              for (const auto& [key, value]: module.styles) {
                this->styles += "    " + key + ": " + value + ";\n";
              }
              this->styles += "  }\n";
              this->body += " class=\"" + module.hash + "\"";
            }
            this->body += "></" + this->html(module.name) + ">\n";
          } else {
            for (int i = 0; i < this->inner.children.size(); i++) {
              this->withdraw(this->inner.children[i]);
            }
          }
        }
      } else {
        this->drawtab();
        this->body += module.name + "\n";
      }
    }
  }
}

std::string TreeReader::html(std::string tag) {
  if (std::find(std::begin(tags), std::end(tags), tag) != std::end(tags)) {
    return tag;
  } else {
    return "div";
  }
}

void TreeReader::drawtab() {
  for (int i = 0; i < this->tab_space; i++) {
    this->body += "  ";
  }
}
