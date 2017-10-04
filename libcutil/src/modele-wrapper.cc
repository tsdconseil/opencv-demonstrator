#include "modele.hpp"
#include <string>

using namespace std;

namespace utils
{
namespace model
{


string NodeCppWrapper::format_comment(int indent, const Localized &l)
{
  string s = "", desc = utils::str::utf8_to_latin(l.get_description());
  uint32_t i, j, k;

  for(i = 0; i < (uint32_t) indent; i++)
    s += " ";

  s += "/** @brief ";
  s += utils::str::utf8_to_latin(l.get_localized());

  /* Clean description. */
  string d2;
  bool only_spaces = true;
  uint32_t scnt = 0;
  for(i = 0; i < desc.size(); i++)
  {
    if((desc[i] == ' ') || (desc[i] == 0x0d) || (desc[i] == 0x0a) || (desc[i] == '\t'))
    {
      if(scnt == 0)
        d2 += " ";
      scnt++;
    }
    else
    {
      char c[2];
      c[0] = desc[i];
      c[1] = 0;
      d2 += (&c[0]);
      scnt = 0;
      only_spaces = false;
    }
  }
  if(only_spaces)
    d2 = "";

  if(only_spaces && (l.get_localized().size() == 0))
    return "";


  if(d2.size() > 0)
  {
    /* Text justification */
    uint32_t colcnt = 0;
    for(j = 0; j < d2.size(); j++)
    {
      if((j == 0) || (colcnt >= 70))
      {
        if((j > 0) && (d2[j-1] != ' '))
          s += "-";
        /* new line */
        s += "\n";
        for(k = 0; k < (uint32_t) indent; k++)
          s += " ";
        s += " *  ";
        if((j > 0) && (d2[j-1] != ' '))
          s += "-";
        colcnt = 0;
      }
      s += utils::str::utf8_to_latin(d2.substr(j, 1));
      colcnt++;
    }
  }
  return s;
}

NodeCppWrapper::NodeCppWrapper()
{
}


int NodeCppWrapper::gen_ccp_wrapper(NodeSchema *schema, const string &path_c, const string &path_h)
{
  if(files::check_and_build_directory(path_c))
  {
    erreur("unable to create output source folder.");
    return -1;
  }

  if(files::check_and_build_directory(path_h))
  {
    erreur("unable to create output include folder.");
    return -1;
  }

  string s;
  string fileid = str::str_to_file(schema->name.get_id());

  s += "/** @file " + fileid + ".hpp\n";
  s += " *  Specific interface to the node " + schema->name.get_id() + " (" + schema->name.get_localized() + ").\n\n";
  s += " *  File generated on @todo */\n\n\n";

  s += "#include <string>\n";
  s += "#include <deque>\n";
  s += "#include \"modele.hpp\"\n";
  s += "\n";
  s += "using namespace std;\n";
  s += "using namespace utils;\n";
  s += "using namespace utils::model;\n\n";


  string header_path = path_h + files::get_path_separator() + fileid + ".hpp";
  string source_path = path_c + files::get_path_separator() + fileid + ".cc";

  // (1) Make the include file

  // @todo dependancies
  s += gen_class(schema);

  files::save_txt_file(header_path, s);


  // (2) Make the source file
  s = "";

  s += "#include \"" + fileid + ".hpp\"\n";
  s += "#include <sstream>\n";
  s += "#include <stdexcept>\n";
  s += "#include <cstdlib>\n";
  s += "#include <iostream>\n\n";

  s += gen_class_impl(schema);

  files::save_txt_file(source_path, s);
  return 0;
}


string NodeCppWrapper::gen_class_impl(NodeSchema *schema)
{
  unsigned int i, n;
  string s;
  size_t indent = 0, indent_step = 2;

  /* export child classes */
  n = schema->children.size();
  for(i = 0; i < n; i++)
    s += gen_class_impl(schema->children[i].ptr);

  string cls = str::str_to_class(schema->name.get_id());


  ///// (1) Read from model implementation
  {
  s += "int " + cls + "::read_from_model(const Node model, bool partial_model)\n{\n";

  n = schema->attributes.size();
  for(i = 0; i < n; i++)
  {
    const AttributeSchema &as = *(schema->attributes[i]);

    // Manage partial model
    s += "  if (!partial_model || model.has_attribute(\"" + as.name.get_id() + "\"))\n";
    s += "    " + str::str_to_var(as.name.get_id()) + " = " + gen_get_attribute_as(as) + ";\n";
  }


  n = schema->children.size();
  for(i = 0; i < n; i++)
  {
    SubSchema &ss = schema->children[i];
    NodeSchema *child = ss.ptr;

    std::string mname = str::str_to_var(child->name.get_id());

    if(ss.max == 1)
    {
      // Manage partial model
      s += "  if (!partial_model || model.has_child(\"" + child->name.get_id() + "\"))\n";
      
      s += "    " + mname + ".read_from_model(model.get_child(\"" + child->name.get_id() +  "\"));\n";
    }
    else
    {
      s += "  // Adapt the size of the container\n";
      s += "  " + mname + "s.resize(model.get_children_count(\"" + child->name.get_id() + "\"));\n";

      s += "  for(unsigned int i = 0; i < " + mname + "s.size(); i++)\n";
      s += "    " + mname + "s[i].read_from_model(model.get_child_at(\"" + child->name.get_id() +  "\", i));\n";
    }
  }

  s += "\n  return 0;\n";

  s += "}\n\n";
  }

  ///// (2) Write to model implementation
  {
  s += "int " + cls + "::write_to_model(Node model)\n{\n";

  /* Initialize the fields values */
  TextMatrix tm(2);

  n = schema->attributes.size();
  for(i = 0; i < n; i++)
  {
    const AttributeSchema &as = *(schema->attributes[i]);

    tm.add("  model.set_attribute(\"" + as.name.get_id() + "\", ");
    tm.add("  " + str::str_to_var(as.name.get_id()) + ");");

    tm.next_line();
  }
  s += tm.get_result();


  n = schema->children.size();
  for(i = 0; i < n; i++)
  {
    SubSchema &ss = schema->children[i];
    NodeSchema *child = ss.ptr;

    std::string mname = str::str_to_var(child->name.get_id());

    if(ss.max == 1)
      s += "  " + mname + ".write_to_model(model.get_child(\"" + child->name.get_id() +  "\"));\n";
    else
    {
      s += "  {\n";
      s += "    // Adapt number of children in the model according to the deque size\n";
      s += "    unsigned int n1 = " + mname + "s.size();\n";
      s += "    unsigned int n2 = model.get_children_count(\"" + child->name.get_id() + "\");\n\n";

      s += "    for(unsigned int i = n1; i < n2; i++)\n";
      s += "      model.remove_child(model.get_child_at(\"" + child->name.get_id() + "\", n1));\n\n";

      s += "    for(unsigned int i = n2; i < n1; i++)\n";
      s += "      model.add_child(\"" + child->name.get_id() + "\");\n\n";
        

      //s += "  " + mname + "s.clear();";
      //s += "  " + mname + "s.resize(model.get_children_count(\"" + child->name.get_id() + "\");\n";

      s += "    for(unsigned int i = 0; i < " + mname + "s.size(); i++)\n";
      s += "      " + mname + "s[i].write_to_model(model.get_child_at(\"" 
        + child->name.get_id() +  "\", i));\n";
      s += "  }\n";
    }
  }

  s += "\n  return 0;\n";
  s += "}\n\n";
  }

  ///// (3) Update model implementation
  {
  s += "void " + cls + "::update_model(Node model) const\n{\n";
  indent += indent_step;
  
  // Process attributes
  n = schema->attributes.size();
  for(i = 0; i < n; i++)
  {
    const AttributeSchema &as = *(schema->attributes[i]);

    s += gen_indent(indent) + "if (model.has_attribute(\"" + as.name.get_id() + "\")) {\n";
    indent += indent_step;
    s += gen_indent(indent) + gen_attribute_type(as) + " _" + str::str_to_var(as.name.get_id()) + " = " + gen_get_attribute_as(as) + ";\n";
    s += gen_indent(indent) + "if (_" + str::str_to_var(as.name.get_id()) + " != " + str::str_to_var(as.name.get_id()) + ") {\n";
    indent += indent_step;
//    s += gen_indent(indent) + "std::cout << \"--- Attribute '" + as.name.get_id() + "' value has changed : \" << _" + str::str_to_var(as.name.get_id()) + " << \" -> \" << " + str::str_to_var(as.name.get_id()) + " << std::endl;\n";
    s += gen_indent(indent) + "model.set_attribute(\"" + as.name.get_id() + "\", " + str::str_to_var(as.name.get_id()) + ");\n";
    indent -= indent_step;
    s += gen_indent(indent) + "}\n";
    indent -= indent_step;
    s += gen_indent(indent) + "} else {\n";
    indent += indent_step;
    s += gen_indent(indent) + "throw std::invalid_argument(\"No attribute of name '" + as.name.get_id() + "' was found in model to update\");\n";
    indent -= indent_step;
    s += gen_indent(indent) + "}\n";
  }
  s += gen_indent(indent) + "\n";

  // Process children
  n = schema->children.size();
  for(i = 0; i < n; i++)
  {
    SubSchema &ss = schema->children[i];
    NodeSchema *child = ss.ptr;

    std::string mname = str::str_to_var(child->name.get_id());

    s += gen_indent(indent) + "if (model.has_child(\"" + child->name.get_id() + "\")) {\n";
    indent += indent_step;
    
    if (ss.max == 1) {
      s += gen_indent(indent) + mname + ".update_model(model.get_child(\"" + child->name.get_id() +  "\"));\n";
    } else {
      s += gen_indent(indent) + "if (model.get_children_count(\"" + child->name.get_id() + "\") == " + mname + "s.size()) {\n";
      indent += indent_step;

      s += gen_indent(indent) + "for (size_t i = 0 ; i < " + mname + "s.size() ; i++) {\n";
      indent += indent_step;
      s += gen_indent(indent) + mname + "s[i].update_model(model.get_child_at(\"" + child->name.get_id() +  "\", i));\n";
      indent -= indent_step;
      s += gen_indent(indent) + "}\n";

      indent -= indent_step;
      s += gen_indent(indent) + "} else {\n";
      indent += indent_step;
      s += gen_indent(indent) + "stringstream ss;\n";
      s += gen_indent(indent) + "ss << \"Number of children of name '" + child->name.get_id() + "' in model to update is not equal to expected size\";\n";
      s += gen_indent(indent) + "ss << \" (\" << model.get_children_count(\"" + child->name.get_id() + "\") << \" != \" << " + mname + "s.size() << \")\";\n";
      s += gen_indent(indent) + "throw std::invalid_argument(ss.str());\n";
      indent -= indent_step;
      s += gen_indent(indent) + "}\n";
    }

    indent -= indent_step;
    s += gen_indent(indent) + "} else {\n";
    indent += indent_step;
//     s += gen_indent(indent) + "std::cout << \"--- ERROR: No child of name '" + child->name.get_id() + "' was found in model to update\" << std::endl;\n";
    s += gen_indent(indent) + "throw std::invalid_argument(\"No child of name '" + child->name.get_id() + "' was found in model to update\");\n";
    indent -= indent_step;
    s += gen_indent(indent) + "}\n";
  }

  s += "}\n\n";
  }

  ///// (4) Read a single attribute from path and value
  {
  s += "void " + cls + "::read(const string &_path_, const string &_value_)\n{\n";

  s += "  // Look if attribute is in a child\n";
  s += "  size_t pos = _path_.find(\"/\");\n";
  s += "  if (pos == std::string::npos) {\n";
  s += "    // Search and modify concerned attribute\n";

  s += "    ";
  n = schema->attributes.size();
  for(i = 0; i < n; i++)
  {
    const AttributeSchema &as = *(schema->attributes[i]);

    s += "if (_path_ == \"" + as.name.get_id() + "\") {\n";
    s += "      " + str::str_to_var(as.name.get_id());

    switch(as.type)
    {
    case TYPE_STRING:
      s += " = _value_;\n";
      break;
    case TYPE_FLOAT:
//       s += " = std::stof(_value_);\n";
      s += " = strtof(_value_.c_str(), nullptr);\n";
      break;
    case TYPE_BOOLEAN:
//      s += " = (bool)std::stoi(_value_);\n";
      s += " = (bool)strtol(_value_.c_str(), nullptr, 10);\n";
      break;
    case TYPE_INT:
      if(as.enumerations.size() > 0) {
//        s += " = (" + str::str_to_var(as.name.get_id()) + "_t) std::stoi(_value_);\n";
        s += " = (" + str::str_to_var(as.name.get_id()) + "_t) strtol(_value_.c_str(), nullptr, 10);\n";
      } else {
//        s += " = std::stoi(_value_);\n";
        s += " = strtol(_value_.c_str(), nullptr, 10);\n";
      }
      break;
    default:
      erreur("unmanaged att type!");
      break;
    }

    s += "    } else ";
  }
  s += "{\n";
  s += "      throw std::invalid_argument(\"No attribute of name '\" + _path_ + \"' was found\");\n";
  s += "    }\n";

  s += "  } else {\n";
  s += "    // Search and enter in concerned child\n";

  s += "\n    // Extract child information\n";
  s += "    std::string child_name = _path_.substr(0, pos);\n";
  s += "    std::string next_path = _path_.substr(pos + 1);\n";

  bool use_index = false;
  n = schema->children.size();
  for(i = 0; i < n; i++) {
    SubSchema &ss = schema->children[i];
    if (ss.max != 1) {
      use_index = true;
      break;
    }
  }
  if (use_index) {
    s += "    std::size_t child_index = 0;\n";
    s += "    std::size_t pos_ind = child_name.find(\"[\");\n";
    s += "    if (pos_ind != std::string::npos) {\n";
    s += "      std::string str_ind = child_name.substr(pos_ind + 1, child_name.length() - pos_ind - 2);\n";
//     s += "      child_index = std::stoi(str_ind);\n";
    s += "      child_index = strtol(str_ind.c_str(), nullptr, 10);\n";
    s += "      child_name = child_name.substr(0, pos_ind);\n";
    s += "    }\n";
  }

  s += "\n    // Find concerned child\n";
  s += "    ";
  n = schema->children.size();
  for(i = 0; i < n; i++)
  {
    SubSchema &ss = schema->children[i];
    NodeSchema *child = ss.ptr;

    std::string mname = str::str_to_var(child->name.get_id());

    s += "if (child_name == \"" + child->name.get_id() + "\") {\n";
    if (ss.max == 1) {

      s += "      " + mname + ".read(next_path, _value_);\n";
    } else {
      s += "      // Verify size of the container\n";
      s += "      if (child_index < " + mname + "s.size()) {\n";
      s += "        " + mname + "s[child_index].read(next_path, _value_);\n";
      s += "      } else {\n";
      s += "        std::stringstream oss;\n";
      s += "        oss << \"Index \" << child_index << \" of child named '\" << child_name << \"' is out of range (size = \" << " + mname + "s.size() << \")\";\n";
      s += "        std::string ossstr = oss.str();\n";
      s += "        throw std::out_of_range(ossstr.c_str());\n";
      s += "      }\n";
    }
    s += "    } else ";
  }
  s += "{\n";
  s += "      throw std::invalid_argument(\"No child of name '\" + child_name + \"' was found\");\n";
  s += "    }\n";

  s += "  }\n";

  s += "}\n\n";
  }


  return s;
}

string NodeCppWrapper::gen_class(NodeSchema *schema, int indent)
{
  string s;
  unsigned int i, j, k, n;
  string cls_name = str::str_to_class(schema->name.get_id());

  /* export child classes */
  n = schema->children.size();
  for(i = 0; i < n; i++)
    s += gen_class(schema->children[i].ptr, indent);

  s += format_comment(indent, schema->name) + "*/\n";

  for(i = 0; i < (unsigned int) indent; i++)
    s += " ";

  s += "class ";
  s += cls_name;
  s += "\n{\n";
  s += "public:\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "/** @brief Initialize the content of this specific class from a generic tree model.\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += " * By default partial_model is false, the model is considered complete, so that expected but unavailable sub-nodes generate an error.\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += " * If partial_model is true, the model is partially completed and only available sub-nodes are read. */\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "int read_from_model(const Node model, bool partial_model = false);\n\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "/** @brief Write all the values (recursively) contained in this specific class to a generic tree model. */\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "int write_to_model(Node model);\n\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "/** @brief Update all the values (recursively) contained in this specific class to a generic tree model. */\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "void update_model(Node model) const;\n\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "/** @brief Modify an attribute value of this specific class given a path and a value.\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += " * If given path or given value is invalid, an std::invalid_argument or std::out_of_range is thrown. */\n";

  for(i = 0; i < (unsigned int) indent + 2; i++)
    s += " ";

  s += "void read(const string &path, const string &value);\n\n";

  n = schema->attributes.size();
  for(j = 0; j < n; j++)
  {
    AttributeSchema &as = *(schema->attributes[j]);

    s += gen_attribute_comment(as, indent + 2);

    for(i = 0; i < (unsigned int) (indent + 2); i++)
      s += " ";

    unsigned int nb_enums = as.enumerations.size();

    if((nb_enums > 0) && (as.type == TYPE_INT))
    {
      s += "enum "
          + str::str_to_var(as.name.get_id()) + "_t" +

          "\n  {\n";

      TextMatrix tm(3);


      for(k = 0; k < nb_enums; k++)
      {
        Enumeration &match = as.enumerations[k];

        if(match.name.has_description())
          tm.add_unformatted_line(format_comment(indent + 4, match.name) + "*/\n");

        tm.add("   ");
        tm.add(str::str_to_cst(as.name.get_id() + "_" + match.name.get_id()));

        string str = string(" = ") + match.value;


        if(k != nb_enums - 1)
          str += ",";

        tm.add(str);

        tm.next_line();
      }
      s += tm.get_result();

      s += "  }";
    }
    else
    {
      s += gen_attribute_type(as);
    }


    s += " ";
    s += str::str_to_var(as.name.get_id());
    s += ";\n\n";
  }

  for(k = 0; k < schema->children.size(); k++)
  {
    const SubSchema &ss = schema->children[k];
    NodeSchema *child = schema->children[k].ptr;

    for(i = 0; i < (unsigned int) indent + 2; i++)
      s += " ";

    s += "/** @brief " + child->name.get_localized() + " */\n";

    for(i = 0; i < (unsigned int) indent + 2; i++)
      s += " ";

    string cls_name = str::str_to_class(child->name.get_id());
    string var_name = str::str_to_var(child->name.get_id());

    /** Only one instance */
    if((ss.min == 1) && (ss.max == 1))
    {
      s += cls_name + " " + var_name + ";\n\n";
    }
    /** Multiple instances */
    else
    {
      s += "deque<" + cls_name + "> " + var_name + "s" + ";\n\n";
    }
  }
  s += "};\n\n\n\n";

  return s;
}




std::string NodeCppWrapper::gen_attribute_comment(const AttributeSchema &as, int indent)
{
  unsigned int i, j;
  string s = format_comment(indent, as.name);

  uint32_t nmatch = as.enumerations.size();

  if(nmatch > 0)
  {
    TextMatrix mmatrix(2);
    s += "\n";
    for(j = 0; j < nmatch; j++)
    {
      const Enumeration &match = as.enumerations[j];
      string idt = "";

      for(i = 0; i < (uint32_t) indent + 4; i++)
        idt += " ";

      mmatrix.add(idt + match.value + ": ");


      string dsc = utils::str::utf8_to_latin(match.name.get_localized());

      mmatrix.add(dsc);
      mmatrix.next_line();
    }
    s += mmatrix.get_result();
    s = s.substr(0, s.size() - 1);
  }

  s += " */\n";

  return s;
}

std::string NodeCppWrapper::gen_attribute_type(const AttributeSchema &as) {
  switch(as.type)
  {
  case utils::model::TYPE_STRING:
    return "string";
    break;
  case utils::model::TYPE_INT:
    return "int";
    break;
  case utils::model::TYPE_BOOLEAN:
    return "bool";
    break;
  case utils::model::TYPE_FLOAT:
    return "float";
    break;
  default:
    avertissement("type unknown.");
    return "int";
    break;
  }
}

std::string NodeCppWrapper::gen_indent(size_t indent) {
  string res;
  
  for(size_t i = 0; i < indent; i++)
    res += " ";

  return res;
}

std::string NodeCppWrapper::gen_get_attribute_as(const AttributeSchema &as) {
  std::string res;
  
  switch(as.type)
  {
  case TYPE_STRING:
    res = "model.get_attribute_as_string(\"" + as.name.get_id() + "\")";
    break;
    
  case TYPE_FLOAT:
    res = "model.get_attribute_as_float(\"" + as.name.get_id() + "\")";
    break;

  case TYPE_BOOLEAN:
    res = "model.get_attribute_as_boolean(\"" + as.name.get_id() + "\")";
    break;

  case TYPE_INT:
    if(as.enumerations.size() > 0) {
      res = "(" + str::str_to_var(as.name.get_id()) + "_t) model.get_attribute_as_int(\"" + as.name.get_id() + "\")";
    } else {
      res = "model.get_attribute_as_int(\"" + as.name.get_id() + "\")";
    }
    break;
    
  default:
    erreur("unmanaged att type!");
    break;
  }

  return res;
}


}
}
