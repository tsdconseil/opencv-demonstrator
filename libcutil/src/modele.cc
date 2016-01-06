#include "modele.hpp"
#include "mxml.hpp"
#include "cutil.hpp"
#include "templates/modele-private.hpp"

#include <iostream>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include <sstream>
#include <locale>
#include <assert.h>


#ifdef LINUX
#ifdef putc
#undef putc
#endif
#endif



namespace utils
{
namespace model
{

Logable NodeSchema::log("model");
Logable Node::log("model");
Logable NodePatron::log("model");
Logable AttributeSchema::log("model");
Logable Attribute::log("model");

bool NodeSchema::is_empty() const
{
  if(attributes.size() > 0)
    return false;
  if(children.size() > 0)
    return false;
  return true;
}

NodeSchema::~NodeSchema()
{
}

void NodeSchema::add_attribute(refptr<AttributeSchema> schema)
{
  attributes.push_back(schema);
  att_mapper[schema->name.get_id()] = attributes.size() - 1;
}

void NodeSchema::add_sub_node(const SubSchema &schema)
{
  children.push_back(schema);
  string subname = schema.child_str;
  if(schema.name.get_id().size() > 0)
    subname = schema.name.get_id();
  mapper[subname] = children.size() - 1;
}

bool NodeSchema::has_editable_props()
{
  for(unsigned int i = 0; i < attributes.size(); i++)
  {
    AttributeSchema &as = *attributes[i];
    if(!as.is_hidden)
      return true;
  }
  if(this->children.size() > 0)
  {
    for(unsigned int i = 0; i < children.size(); i++)
    {
      if(children[i].ptr->has_editable_props())
        return true;
    }
  }
  if(this->references.size() > 0)
    return true;
  return false;
}

void NodeSchema::update_size_info()
{
  attributes_fixed_size = true;
  for(unsigned int i = 0; i < this->attributes.size(); i++)
  {
    AttributeSchema &as = *attributes[i];
    attributes_fixed_size = attributes_fixed_size && as.fixed_size();
  }
  children_fixed_size = true;
  for(unsigned int i = 0; i < this->children.size(); i++)
  {
    SubSchema ss = children[i];
    //ss.ptr->update_size_info();
    if((ss.min == -1)
        || (ss.max == -1)
        || (ss.min != ss.max)
       || (!ss.ptr->fixed_size))
      children_fixed_size = false;
  }
  fixed_size = attributes_fixed_size && children_fixed_size;
}

bool fixed_size;
bool attributes_fixed_size;
bool children_fixed_size;

CommandSchema::CommandSchema(const MXml &mx)
{
  name = Localized(mx);

  if(mx.has_child("input"))
    input = refptr<NodeSchema>(new NodeSchema(mx.get_child("input")));

  if(mx.has_child("output"))
    output = refptr<NodeSchema>(new NodeSchema(mx.get_child("output")));
}

CommandSchema::CommandSchema(const Node &model)
{
  name = model.get_localized();

  if(model.get_children_count("input") > 0)
    input = refptr<NodeSchema>(new NodeSchema(model.get_child_at("input", 0)));

  if(model.get_children_count("output") > 0)
    output = refptr<NodeSchema>(new NodeSchema(model.get_child_at("output", 0)));
}

CommandSchema::CommandSchema(const CommandSchema &cs)
{
  *this = cs;
}

void CommandSchema::operator =(const CommandSchema &cs)
{
  name    = cs.name;
  input   = cs.input;
  output  = cs.output;
}

Localized Node::get_localized() const
{
  Localized res;

  if(data == nullptr)
  {
    res.set_value(Localized::LANG_ID, "");
    return res;
  }

  if(has_attribute("name"))
    res.set_value(Localized::LANG_ID, name());
  else if(has_attribute("type"))
    res.set_value(Localized::LANG_ID, get_attribute_as_string("type"));
  else
  {
    log.warning("Localization of an node without identifier.");
  }
  if(has_attribute("fr"))
  {
    std::string fr = get_attribute_as_string("fr");
    if(fr.size() > 0)
      res.set_value(Localized::LANG_FR, fr);
  }
  if(has_attribute("en"))
  {
    std::string en = get_attribute_as_string("en");
    if(en.size() > 0)
      res.set_value(Localized::LANG_EN, en);
  }
  if(has_attribute("de"))
  {
    std::string de = get_attribute_as_string("de");
    if(de.size() > 0)
      res.set_value(Localized::LANG_DE, de);
  }
  if(this->has_child("description"))
  {
    for(const Node desc: children("description"))
    {
      res.set_description(
          Localized::parse_language(desc.get_attribute_as_string("lang")),
          desc.get_attribute_as_string("content"));
    }
  }
  return res;
}

NodeSchema::NodeSchema(const Node &e, FileSchema *root, const string &name_)
{
  unsigned int i;

  inheritance = nullptr;

  if(e.is_nullptr())
  {
    log.warning("constructor: from nullptr node.");
    return;
  }

  string schema_name;

  if(name_ != "")
    name.set_value(Localized::LANG_ID, name_);
  if(e.schema()->name.get_id().compare("input") == 0)
  {
    name.set_value(Localized::LANG_ID, "input");
  }
  else if(e.schema()->name.get_id().compare("output") == 0)
  {
    name.set_value(Localized::LANG_ID, "output");
  }
  else if(e.schema()->name.get_id().compare("partial") == 0)
  {
    name.set_value(Localized::LANG_ID, "partial");
  }
  else
    name = e.get_localized();

  fixed_size = true;

  for(const Node &ch: e.children("attribute"))
  {
    string attname = ch.get_attribute_as_string("name");

    refptr<AttributeSchema> ref = refptr<AttributeSchema>(new AttributeSchema(ch));
    add_attribute(ref);
  }

  if(e.has_attribute("inherits"))
    inheritance_name = e.get_attribute_as_string("inherits");
  if(e.has_attribute("icon"))

  icon_path = e.get_attribute_as_string("icon");


  for(const Node cmde: e.children("command"))
  {
    CommandSchema cs(cmde);
    commands.push_back(cs);
  }

  for(const Node sb: e.children("sub"))
  {
    SubSchema ss;
    ss.min = sb.get_attribute_as_int("min");
    ss.max = sb.get_attribute_as_int("max");
    ss.child_str = sb.get_attribute_as_string("type");
    ss.name = sb.get_localized();
    ss.display_tab = false;
    ss.display_tree = false;
    ss.default_key = sb.get_attribute_as_string("key");

    if(sb.has_attribute("sub-readonly"))
      ss.readonly = sb.get_attribute_as_boolean("sub-readonly");
    if(sb.has_attribute("hidden"))
      ss.is_hidden = sb.get_attribute_as_boolean("hidden");

    if(sb.has_attribute("display-tree"))
    {
      ss.display_tree = sb.get_attribute_as_boolean("display-tree");
    }
    if(sb.has_attribute("default"))
      ss.default_count = sb.get_attribute_as_int("default");
    if(sb.has_attribute("display-tab"))
    {
      ss.display_tab = sb.get_attribute_as_boolean("display-tab");
      std::string list = sb.get_attribute_as_string("display-resume");
      std::vector<std::string> lst;
      utils::str::parse_string_list(list, lst, ',');
      ss.resume = lst;
    }
    ss.display_unfold = true;
    if(sb.has_attribute("display-unfold"))
      ss.display_unfold = sb.get_attribute_as_boolean("display-unfold");
    if(ss.name.get_id().size() == 0)
      ss.name.set_value(Localized::LANG_ID, ss.child_str);

    //ss.ptr = nullptr;
    if(root != nullptr)
      ss.ptr = root->get_schema(ss.child_str);

    add_sub_node(ss);
  }

  for(const Node sb: e.children("sub-node"))
  {
    SubSchema ss;
    ss.min = sb.get_attribute_as_int("min");
    ss.max = sb.get_attribute_as_int("max");
    ss.child_str = sb.get_attribute_as_string(/*"type"*/"name");
    ss.name = sb.get_localized();
    ss.display_tab = false;
    ss.display_tree = false;
    ss.default_key = sb.get_attribute_as_string("key");

    if(sb.has_attribute("sub-readonly"))
        ss.readonly = sb.get_attribute_as_boolean("sub-readonly");
    if(sb.has_attribute("hidden"))
      ss.is_hidden = sb.get_attribute_as_boolean("hidden");

    if(sb.has_attribute("display-tree"))
    {
      ss.display_tree = sb.get_attribute_as_boolean("display-tree");
    }
    if(sb.has_attribute("default"))
      ss.default_count = sb.get_attribute_as_int("default");
    if(sb.has_attribute("display-tab"))
    {
      ss.display_tab = sb.get_attribute_as_boolean("display-tab");
      std::string list = sb.get_attribute_as_string("display-resume");
      std::vector<std::string> lst;
      utils::str::parse_string_list(list, lst, ',');
      ss.resume = lst;
    }
    ss.display_unfold = true;
    if(sb.has_attribute("display-unfold"))
      ss.display_unfold = sb.get_attribute_as_boolean("display-unfold");
    if(ss.name.get_id().size() == 0)
      ss.name.set_value(Localized::LANG_ID, ss.child_str);

    //ss.ptr = nullptr;
    if(root != nullptr)
      ss.ptr = root->get_schema(ss.child_str);

    add_sub_node(ss);
  }




  for(i = 0; i < e.get_children_count("ref"); i++)
  {
    const Node &sb = e.get_child_at("ref", i);
    RefSchema ss;
    ss.child_str = sb.get_attribute_as_string("type");
    ss.name = sb.get_localized();
    ss.path = sb.get_attribute_as_string("path");
    references.push_back(ss);
  }
}

void NodeSchema::from_xml(const MXml &mx)
{
  std::vector<const MXml *> lst;
  unsigned int i;

  mx.get_children("attribute", lst);
  for(const MXml *att: lst)
  {
    refptr<AttributeSchema> ref = refptr<AttributeSchema>(new AttributeSchema(*att));
    add_attribute(ref);
  }

  lst.clear();
  mx.get_children("command", lst);
  for(i = 0; i < lst.size(); i++)
  {
    CommandSchema cs(*lst[i]);
    commands.push_back(cs);
  }

  lst.clear();
  mx.get_children("sub", lst);
  for(i = 0; i < lst.size(); i++)
  {
    SubSchema ss;
    ss.min = -1;
    ss.max = -1;

    if(lst[i]->has_attribute("key"))
      ss.default_key = lst[i]->get_attribute("key").to_string();

    if(lst[i]->has_attribute("sub-readonly"))
        ss.readonly = lst[i]->get_attribute("sub-readonly").to_bool();
    if(lst[i]->has_attribute("default"))
      ss.default_count = lst[i]->get_attribute("default").to_int();
    if(lst[i]->has_attribute("hidden"))
        ss.is_hidden = lst[i]->get_attribute("hidden").to_bool();
    if(lst[i]->has_attribute("min"))
      ss.min = lst[i]->get_attribute("min").to_int();
    if(lst[i]->has_attribute("max"))
      ss.max= lst[i]->get_attribute("max").to_int();
    ss.show_header = true;
    if(lst[i]->has_attribute("show-header"))
      ss.show_header= lst[i]->get_attribute("show-header").to_bool();
    ss.child_str = lst[i]->get_attribute("type").to_string();
    ss.name = Localized(*lst[i]);

    ss.display_tree = false;
    if(lst[i]->has_attribute("display-tree"))
    {
      ss.display_tree = lst[i]->get_attribute("display-tree").to_bool();
    }

    ss.display_tab = false;
    if(lst[i]->has_attribute("display-tab"))
    {
      ss.display_tab = lst[i]->get_attribute("display-tab").to_bool();
      std::string list;
      if(lst[i]->has_attribute("display-resume"))
        list = lst[i]->get_attribute("display-resume").to_string();
      std::vector<std::string> lst;
      utils::str::parse_string_list(list, lst, ',');
      ss.resume = lst;
    }

    ss.display_unfold = true;
    if(lst[i]->has_attribute("display-unfold"))
      ss.display_unfold = lst[i]->get_attribute("display-unfold").to_bool();

    //children.push_back(ss);
    //mapper[ss.child_str] = children.size() - 1;
    add_sub_node(ss);
  }

  lst.clear();
  mx.get_children("sub-node", lst);
  for(i = 0; i < lst.size(); i++)
  {
    SubSchema ss;
    ss.min = -1;
    ss.max = -1;
    if(lst[i]->has_attribute("sub-readonly"))
      ss.readonly = lst[i]->get_attribute("sub-readonly").to_bool();

    if(lst[i]->has_attribute("key"))
      ss.default_key = lst[i]->get_attribute("key").to_string();

    if(lst[i]->has_attribute("default"))
      ss.default_count = lst[i]->get_attribute("default").to_int();
    if(lst[i]->has_attribute("hidden"))
        ss.is_hidden = lst[i]->get_attribute("hidden").to_bool();
    if(lst[i]->has_attribute("min"))
      ss.min = lst[i]->get_attribute("min").to_int();
    if(lst[i]->has_attribute("max"))
      ss.max= lst[i]->get_attribute("max").to_int();
    ss.show_header = true;
    if(lst[i]->has_attribute("show-header"))
      ss.show_header= lst[i]->get_attribute("show-header").to_bool();
    ss.child_str = lst[i]->get_attribute("name").to_string();
    ss.name = Localized(*lst[i]);

    ss.display_tree = false;
    if(lst[i]->has_attribute("display-tree"))
    {
      ss.display_tree = lst[i]->get_attribute("display-tree").to_bool();
    }

    ss.display_tab = false;
    if(lst[i]->has_attribute("display-tab"))
    {
      ss.display_tab = lst[i]->get_attribute("display-tab").to_bool();
      std::string list;
      if(lst[i]->has_attribute("display-resume"))
        list = lst[i]->get_attribute("display-resume").to_string();
      std::vector<std::string> lst;
      utils::str::parse_string_list(list, lst, ',');
      ss.resume = lst;
    }

    ss.display_unfold = true;
    if(lst[i]->has_attribute("display-unfold"))
      ss.display_unfold = lst[i]->get_attribute("display-unfold").to_bool();

    add_sub_node(ss);
  }

  lst.clear();
  mx.get_children("ref", lst);
  for(unsigned int i = 0; i < lst.size(); i++)
  {
    RefSchema ss;
    ss.child_str = lst[i]->get_attribute("type").to_string();
    ss.name = Localized(*lst[i]);
    ss.path = "";
    if(lst[i]->has_attribute("path"))
      ss.path = lst[i]->get_attribute("path").to_string();
    references.push_back(ss);
  }
}

NodeSchema::NodeSchema(const MXml &mx)
{
  fixed_size = true;

  inheritance = nullptr;

  if(mx.name.compare("input") == 0)
  {
    if(!mx.has_attribute("name"))
    {
      name.set_value(Localized::LANG_ID, "input");
    }
    else
      name = Localized(mx);
  }
  else
  {
    if(!mx.has_attribute("name"))
    {
      log.anomaly("XML has no 'name' attribute.");
      return;
    }
    name = Localized(mx);
  }

  if(mx.has_attribute("inherits"))
    inheritance_name = mx.get_attribute("inherits").to_string();

  if(mx.has_attribute("icon"))
    icon_path = mx.get_attribute("icon").to_string();

  from_xml(mx);
}

NodeSchema::NodeSchema(const NodeSchema &c)
{
  inheritance = nullptr;
  *this = c;
}

bool NodeSchema::has_reference(std::string name) const
{
  for(unsigned int i = 0; i < references.size(); i++)
  {
    if(references[i].name.get_id().compare(name) == 0)
      return true;
  }
  return false;
}

SubSchema *NodeSchema::get_child(std::string name)
{
  for(unsigned int i = 0; i < children.size(); i++)
  {
    if(children[i].name.get_id().compare(name) == 0)
      return &(children[i]);
  }
  return nullptr;
}

bool NodeSchema::has_child(std::string name) const
{
  for(unsigned int i = 0; i < children.size(); i++)
  {
    if(children[i].name.get_id().compare(name) == 0)
      return true;
  }
  return false;
}

NodeSchema *NodeSchema::get_sub(std::string name)
{
  for(uint32_t i = 0; i < children.size(); i++)
  {
    if(children[i].name.get_id().compare(name) == 0)
    {
      return children[i].ptr;
    }
  }
  log.anomaly("Sub not found: %s.", name.c_str());
  return nullptr;
}

std::string NodeSchema::get_localized() const
{
  std::string res = name.get_localized();
  if(langue.has_item(res))
      res = langue.get_item(res);
  return res;
}

void Node::get_children_recursive(const string &type,
                                  deque<Node> &res)
{
  unsigned int n = schema()->children.size();

  for(unsigned int i = 0; i < n; i++)
  {
    string sub_type = schema()->children[i].ptr->name.get_id();

    for(Node child: children(sub_type))
    {
      if(type.compare(sub_type) == 0)
        res.push_back(child);
      child.get_children_recursive(type, res);
    }
  }
}

RefSchema *NodeSchema::get_reference(std::string name)
{
  for(unsigned int i = 0; i < references.size(); i++)
  {
    if(references[i].name.get_id().compare(name) == 0)
      return &(references[i]);
  }
  return nullptr;
}

bool NodeSchema::has_attribute(std::string name) const
{
  for(const refptr<AttributeSchema> &as: attributes)
  {
    if(as->name.get_id().compare(name) == 0)
      return true;
  }

  return false;
}

refptr<AttributeSchema> NodeSchema::get_attribute(std::string name)
{
  for(const refptr<AttributeSchema> &as: attributes)
  {
    if(as->name.get_id().compare(name) == 0)
      return as;
  }
  log.anomaly("att not found: %s.", name.c_str());
  return refptr<AttributeSchema>();
}

void NodeSchema::do_inherits()
{
  if(inheritance == nullptr)
    return;
  for(unsigned int i = 0; i < inheritance->children.size(); i++)
  {
    bool present = false;
    for(unsigned int j = 0; j < children.size(); j++)
    {
      if(children[j].ptr == inheritance->children[i].ptr)
      {
        present = true;
        break;
      }
    }
    if(!present)
    {
      //children.push_front(inheritance->children[i]);
      add_sub_node(inheritance->children[i]);
    }
  }
  for(unsigned int i = 0; i < inheritance->references.size(); i++)
    references.push_front(inheritance->references[i]);
  for(unsigned int i = 0; i < inheritance->attributes.size(); i++)
  {
    bool present = false;
    for(unsigned int j = 0; j < attributes.size(); j++)
    {
      if(attributes[j]->name.get_id().compare(inheritance->attributes[i]->name.get_id()) == 0)
      {
        present = true;
        break;
      }
    }
    if(!present)
    {
      auto &att = inheritance->attributes[i];
      attributes.push_back(att);
      att_mapper[att->name.get_id()] = attributes.size() - 1;
    }
  }
}

void NodeSchema::serialize(ByteArray &ba)
{
  ba.puts(name.get_id());
  ba.putw(attributes.size());

  for(const refptr<AttributeSchema> &as: attributes)
    as->serialize(ba);

  ba.putw(children.size());
  for(const SubSchema &ss: children)
  {
    ba.puts(ss.child_str);
    uint16_t flags = 0;
    if(ss.has_min())
      flags |= 1;
    if(ss.has_max())
      flags |= 2;
    ba.putw(flags);
    if(ss.has_min())
      ba.putl(ss.min);
    if(ss.has_max())
      ba.putl(ss.max);
  }


}

int  NodeSchema::unserialize(ByteArray &ba)
{
  uint32_t i, n;
  name.set_value(Localized::LANG_ID, ba.pops());
  n = ba.popw();

  for(i = 0; i < n; i++)
  {
    AttributeSchema *as = new AttributeSchema();
    as->unserialize(ba);
    attributes.push_back(refptr<AttributeSchema>(as));
  }

  n = ba.popw();

  for(i = 0; i < n; i++)
  {
    SubSchema ss;
    ss.child_str = ba.pops();
    uint16_t flags = ba.popw();
    ss.min = -1;
    ss.max = -1;
    if(flags & 1)
      ss.min = ba.popl();
    if(flags & 2)
      ss.max = ba.popl();
    //children.push_back(ss);
    add_sub_node(ss);
  }
  return 0;
}

void NodeSchema::operator =(const NodeSchema &c)
{
  name = c.name;
  icon_path = c.icon_path;
  inheritance = c.inheritance;
  inheritance_name = c.inheritance_name;
  fixed_size = c.fixed_size;
  mapper = c.mapper;
  att_mapper = c.att_mapper;

  children = c.children;
  commands = c.commands;
  references = c.references;
  attributes = c.attributes;
}

CommandSchema *NodeSchema::get_command(std::string name)
{
  for(unsigned int i = 0; i < commands.size(); i++)
  {
    if(commands[i].name.get_id().compare(name) == 0)
      return &commands[i];
  }
  log.anomaly("get_command(%s): not found.", name.c_str());
  return nullptr;
}

void Node::dispatch_event(const ChangeEvent &ce)
{
  if(data != nullptr)
  {
    data->dispatch(ce);
  }
}


int FileSchema::check_complete()
{
  build_references();
  return 0;
}

FileSchema::~FileSchema()
{
  //log.trace("Destruction file schema...");
}

void FileSchema::add_schema(const Node &e, const string &name)
{
  if((e.type().compare("node") == 0) || (name.size() > 0))
  {
    refptr<NodeSchema> es(new NodeSchema(e, nullptr, name));
    from_element2(e);
    schemas.push_back(es);
  }
  else
  {
    if(e.type().compare("schema") != 0)
    {
      log.anomaly("This file is not an application file (root tag is " + e.type() + ").");
      return;
    }
    for(unsigned int i = 0; i < e.get_children_count("node"); i++)
    {
      NodeSchema *es_ = new NodeSchema(e.get_child_at("node", i));
      refptr<NodeSchema> es(es_);
      schemas.push_back(es);
      //trace("Added schema: %s.", es.name.c_str());
    }

#   if 0
    for(unsigned int i = 0; i < e.get_children_count("command"); i++)
    {
      Node cmd = e.get_child_at("command", i);
      if(cmd.has_child("input"))
      {
        add_schema(cmd.get_child("input"));
      }
      NodeSchema *es_ = new NodeSchema(e.get_child_at("node", i));
      refptr<NodeSchema> es(es_);
      schemas.push_back(es);
      //trace("Added schema: %s.", es.name.c_str());
    }
#   endif
  }
  //build_references();
}

void FileSchema::from_element2(const Node &e)
{
  for(unsigned int i = 0; i < e.get_children_count("sub-node"); i++)
  {
    Node se = e.get_child_at("sub-node", i);
    NodeSchema *es_ = new NodeSchema(se);
    refptr<NodeSchema> es(es_);
    schemas.push_back(es);
    from_element2(se);
  }
}

void FileSchema::from_element(const Node &e)
{
  if(e.type().compare("schema") != 0)
  {
    if(e.type().compare("node") == 0)
    {
      NodeSchema *es_ = new NodeSchema(e);
      refptr<NodeSchema> es(es_);
      from_element2(e);
      schemas.push_back(es);
      build_references();
      std::string rootname = e.get_attribute_as_string("name");
      root = get_schema(rootname);
      root->update_size_info();
      return;
    }
    log.anomaly("This file is not an application file (root tag is " + e.type() + ").");
    return;
  }
  std::string rootname = e.get_attribute_as_string("root");

  for(Node child: e.children("node"))
  {
    NodeSchema *es_ = new NodeSchema(child);
    refptr<NodeSchema> es(es_);
    from_element2(child);

    schemas.push_back(es);
  }
  build_references();
  root = get_schema(rootname);
  root->update_size_info();
}

void FileSchema::from_xml2(const MXml &node)
{
  std::vector<const MXml *> lst;

  node.get_children("sub-node", lst);

  for(unsigned int i = 0; i < lst.size(); i++)
  {
    const MXml &se = *(lst[i]);

    refptr<NodeSchema> es(new NodeSchema(se));
    schemas.push_back(es);
    from_xml2(se);
  }

  lst.clear();
  node.get_children("node", lst);
  for(unsigned int i = 0; i < lst.size(); i++)
  {
    const MXml &se = *(lst[i]);
    refptr<NodeSchema> es(new NodeSchema(se));
    schemas.push_back(es);
    // trace("added schema %s.", es.name.get_id().c_str());
    from_xml2(se);
  }
}

int FileSchema::from_xml(const MXml &xm)
{
  unsigned int i, j;

  if(xm.name.compare("schema") != 0)
  {
    log.anomaly("This file is not an application file (root tag is " + xm.name + ").");
    return -1;
  }

  std::string rootname = xm.get_attribute("root").to_string();

  std::vector<const MXml *> lst;

  xm.get_children("node", lst);
  for(i = 0; i < lst.size(); i++)
  {
    from_xml2(*lst[i]);
    refptr<NodeSchema> es(new NodeSchema(*lst[i]));
    schemas.push_back(es);
  }

  lst.clear();
  xm.get_children("include-schema", lst);
  for(i = 0; i < lst.size(); i++)
  {
    if(!lst[i]->has_attribute("path"))
    {
      log.warning("Tag include-schema without path.");
      continue;
    }
    std::string path = lst[i]->get_attribute("path").to_string();

    MXml xm;
    if(xm.from_file(utils::get_fixed_data_path() + PATH_SEP + path))
    {
      log.anomaly("Failed to load include schema @%s.", path.c_str());
      continue;
    }
    else
    {
      log.trace("Loading include-schema %s..", path.c_str());
      //trace("VALLLL = \n%s\n", xm.dump().c_str());
      from_xml2(xm);
    }
  }

  lst.clear();
  xm.get_children("extension", lst);

  for(i = 0; i < lst.size(); i++)
  {
    std::string type = lst[i]->get_attribute("type").to_string();
    for(j = 0; j < schemas.size(); j++)
    {
      if(schemas[j]->name.get_id().compare(type) == 0)
      {
        schemas[j]->from_xml(*lst[i]);
        break;
      }
    }
    if(j == schemas.size())
    {
      log.warning("Extension: schema not found: %s.", type.c_str());
    }
    else
    {
      from_xml2(*lst[i]);
    }
  }

  build_references();
  root = get_schema(rootname);
  if(root != nullptr)
    root->update_size_info();

  return 0;
}

int FileSchema::from_string(const string &s)
{
  MXml xm;

  if(xm.from_file(s))
    return -1;

  return from_xml(xm);
}

int FileSchema::from_file(std::string filename)
{
  MXml xm;

  if(xm.from_file(filename))
    return -1;

  return from_xml(xm);
}

FileSchema::FileSchema(std::string filename): log("libcutil")
{
  log.trace("fileschema(%s)...", filename.c_str());
  from_file(filename);
}

void FileSchema::build_references()
{
  for(unsigned int i = 0; i < schemas.size(); i++)
  {
    restart:
    for(unsigned int j = 0; j < schemas[i]->children.size(); j++)
    {
      schemas[i]->children[j].ptr = get_schema(schemas[i]->children[j].child_str);

      if(schemas[i]->children[j].ptr == nullptr)
      {
        log.warning("build references: schema not found: %s.", schemas[i]->children[j].child_str.c_str());
        // Remove schema from the children
        std::deque<SubSchema>::iterator it = schemas[i]->children.begin() + j;
        schemas[i]->children.erase(it);
        goto restart;
      }
    }

    for(unsigned int j = 0; j < schemas[i]->attributes.size(); j++)
    {
      refptr<AttributeSchema> &as = schemas[i]->attributes[j];

      for(unsigned int k = 0; k < as->enumerations.size(); k++)
      {
        unsigned int l;
        if(as->enumerations[k].schema_str.size() > 0)
        {
          as->enumerations[k].schema = get_schema(as->enumerations[k].schema_str);
          for(l = 0; l < schemas[i]->children.size(); l++)
          {
            SubSchema &ss = schemas[i]->children[l];
            if(ss.ptr == as->enumerations[k].schema)
            {
              ss.is_exclusive = true;
              break;
            }
          }
        }
      }
    }

    for(unsigned int j = 0; j < schemas[i]->references.size(); j++)
    {
      schemas[i]->references[j].ptr = get_schema(schemas[i]->references[j].child_str);
      if(schemas[i]->references[j].ptr == nullptr)
      {
        log.anomaly("Schema not found for ref: %s.", schemas[i]->references[j].child_str.c_str());
        return;
      }
    }
    if(schemas[i]->inheritance_name.size() > 0)
    {
      schemas[i]->inheritance = get_schema(schemas[i]->inheritance_name);
    }
  }
  for(unsigned int i = 0; i < schemas.size(); i++)
  {
    if(schemas[i]->inheritance != nullptr)
      schemas[i]->do_inherits();
  }
}

NodeSchema *FileSchema::get_schema(std::string name)
{
  for(refptr<NodeSchema> &s: schemas)
  {
    if(s->name.get_id().compare(name) == 0)
      return s.get_reference();
  }
  log.warning("Schema not found: " + name);
  return nullptr;
}

std::string NodeSchema::to_string()
{
  std::string res = "";
  NodeSchema *s = this;
  res += "<node " + utils::str::xmlAtt("name", s->name.get_id());
  res += ">\n";

  for(unsigned int j = 0; j < s->attributes.size(); j++)
    res += s->attributes[j]->to_string();

  for(const SubSchema &ss: children)
  {
    res += "<sub " + utils::str::xmlAtt("type", ss.ptr->name.get_id());
    if(ss.ptr->name.get_id().compare(ss.name.get_id()) != 0)
      res += utils::str::xmlAtt("name", ss.name.get_id());
    if(ss.min != -1)
      res += utils::str::xmlAtt("min", ss.min);
    if(ss.max != -1)
      res += utils::str::xmlAtt("max", ss.max);
    res += "/>\n";
  }

  for(unsigned int j = 0; j < s->references.size(); j++)
  {
    res += "<ref " + utils::str::xmlAtt("type", s->references[j].ptr->name.get_id());
    if(s->references[j].ptr->name.get_id().compare(s->references[j].name.get_id()) != 0)
      res += utils::str::xmlAtt("name", s->references[j].name.get_id());
    res += utils::str::xmlAtt("path", s->references[j].path);
    res += "/>\n";
  }
  res += "</node>\n";
  return res;
}

std::string FileSchema::to_string()
{
  std::string res = "";
  res += "<schema " + utils::str::xmlAtt("root", root->name.get_id()) + ">\n";

  for(const refptr<NodeSchema> &s: schemas)
  {
    res += "<node " + utils::str::xmlAtt("name", s->name.get_id());
    if(s->inheritance != nullptr)
      res += utils::str::xmlAtt("inherits", s->inheritance->name.get_id());
    res += ">\n";

    for(unsigned int j = 0; j < s->attributes.size(); j++)
      res += s->attributes[j]->to_string();

    for(const SubSchema &ss: s->children)
    {
      res += "<sub " + utils::str::xmlAtt("type", ss.ptr->name.get_id());
      if(ss.ptr->name.get_id().compare(ss.name.get_id()) != 0)
        res += utils::str::xmlAtt("name", ss.name.get_id());
      if(ss.min != -1)
        res += utils::str::xmlAtt("min", ss.min);
      if(ss.max != -1)
        res += utils::str::xmlAtt("max", ss.max);
      res += "/>\n";
    }
    for(unsigned int j = 0; j < s->references.size(); j++)
    {
      res += "<ref " + utils::str::xmlAtt("type", s->references[j].ptr->name.get_id());
      if(s->references[j].ptr->name.get_id().compare(s->references[j].name.get_id()) != 0)
        res += utils::str::xmlAtt("name", s->references[j].name.get_id());
      res += utils::str::xmlAtt("path", s->references[j].path);
      res += "/>\n";
    }
    res += "</node>\n";
  }
  res += "</schema>\n";
  return res;
}

FileSchema::FileSchema(): log("libcutil")
{
}

FileSchema::FileSchema(const FileSchema &c): log("libcutil")
{
  *this = c;
}

void FileSchema::operator =(const FileSchema &c)
{
  /*schemas.clear();
  for(unsigned i = 0; i < c.schemas.size(); i++)
    schemas.push_back(c.schemas[i]);*/
  schemas = c.schemas;
  build_references();
  root = get_schema(c.root->name.get_id());
}


AttributeSchema::~AttributeSchema()
{
  //log.trace("delete.");
}

long int AttributeSchema::get_min()
{
  if(has_min)
    return min;
  if(!is_signed)
    return 0;
  if(size == 1)
    return -127;
  if(size == 2)
    return -32767;
  return -2147483647;
}

bool AttributeSchema::has_constraints() const
{
  return (constraints.size() > 0);
}

long int AttributeSchema::get_max()
{
  if(has_max)
  {
    if(max == -1)
    {
      log.anomaly("max == -1");
      return 4294967295UL;
    }
    return max;
  }
  if(is_signed)
  {
    if(size == 1)
      return 128;
    if(size == 2)
      return 32768;
    return 2147483647;
  }
  if(size == 1)
    return 255;
  if(size == 2)
    return 65535;
  return 2147483647;
}

AttributeSchema::AttributeSchema()
{
  log.setup("model");
  //fprintf(stderr, "att_schema: cons..\n"); fflush(stderr);
  /*setup("model", "attribute-schema");*/
  id= 0xffff;
  size = 1;
  //name = "";
  type = TYPE_STRING;
  is_signed = false;
  min = -1;
  is_ip = false;
  max = -1;
  unit = "";
  extension = "";
  //default_value;
  is_hexa = false;
  is_bytes = false;
  is_hidden = false;
  is_volatile = false;
  is_read_only = false;
  is_instrument = false;
  formatted_text = false;
}

void AttributeSchema::serialize(ByteArray &ba) const
{
  uint16_t flags = 0;

  if(is_signed)
    flags |= (1 << 0);
  if(is_bytes)
    flags |= (1 << 1);
  if(is_hexa)
    flags |= (1 << 2);
  if(is_hidden)
    flags |= (1 << 3);
  if(is_volatile)
    flags |= (1 << 4);
  if(is_read_only)
    flags |= (1 << 5);
  if(has_min)
    flags |= (1 << 6);
  if(has_max)
    flags |= (1 << 7);

  ba.putc((uint8_t) type);
  ba.putw(flags);
  ba.putc(size);
  ba.puts(name.get_id());
  ba.puts(unit);


# if 0

  std::string unit, extension, requirement;
  long int min;
  long int max;
  long int get_min();
  long int get_max();
  bool has_constraints() const;
  std::vector<std::string> constraints;
  std::vector<Enumeration> enumerations;
  std::string default_value;
  /** TODO: generalize */
  std::string description, description_fr;
  /** ? */
  bool is_unique;
  /** TODO: generalize */
  std::string fr, en;
# endif
}

int  AttributeSchema::unserialize(ByteArray &ba)
{
  type = (attribute_type_t) ba.popc();
  uint16_t flags = ba.popw();
  is_signed = ((flags & 1) != 0);
  is_bytes  = ((flags & (1 << 1)) != 0);
  is_hexa   = ((flags & (1 << 2)) != 0);
  is_hidden = ((flags & (1 << 3)) != 0);
  is_volatile = ((flags & (1 << 4)) != 1);
  is_read_only = ((flags & (1 << 5)) != 0);
  has_min = ((flags & (1 << 6)) != 0);
  has_max = ((flags & (1 << 7)) != 0);
  size = ba.popc();
  name.set_value(Localized::LANG_ID, ba.pops());
  return 0;
}

bool AttributeSchema::fixed_size() const
{
  if((type == TYPE_STRING) || (type == TYPE_FOLDER) || (type == TYPE_FILE)|| (type == TYPE_SERIAL))
    return false;
  return true;
}

std::string AttributeSchema::get_ihm_value(std::string val) const
{
  for(const Enumeration &e: enumerations)
  {
    if((e.value.compare(val) == 0) || (e.name.get_id().compare(val) == 0))
      return e.name.get_localized();
  }
  if(type == TYPE_BOOLEAN)
  {
    if(langue.current_language.compare("fr") == 0)
    {
      if((val.compare("0") == 0) || (val.compare("false") == 0))
        return "non";
      else
        return "oui";
    }
  }
  return val;
}

std::string AttributeSchema::get_default_value() const
{
  if(default_value.size() > 0)
    return get_string(default_value);

  switch(type)
  {
  case TYPE_STRING: return "";
  case TYPE_BOOLEAN: return "false";
  case TYPE_FLOAT: return "0.0";
  case TYPE_INT:
  {
    if(enumerations.size() > 0)
      return enumerations[0].value;//name.get_id();
    if(has_min)
      return utils::str::int2str(min);
    if(is_hexa)
      return "0x00000000";
    return "0";
  }
  case TYPE_COLOR: return "0.0.0";
  case TYPE_BLOB: return "";
  case TYPE_FOLDER: return ".";
  case TYPE_FILE: return "";
  case TYPE_SERIAL: return "";
  case TYPE_DATE: return "1.1.2000";
  }
  return "";
}

std::string AttributeSchema::to_string() const
{
  std::string res = "";
  res = "<attribute ";
  res += utils::str::xmlAtt("name", name.get_id());
  res += utils::str::xmlAtt("type", type2string());
  if((type == TYPE_INT) || (type == TYPE_FLOAT))
    res += utils::str::xmlAtt("size", utils::str::int2str(size));
  if(has_min)
    res += utils::str::xmlAtt("min", utils::str::int2str(min));
  if(has_max)
    res += utils::str::xmlAtt("max", utils::str::int2str(max));
  if(id != 0xffff)
    res += utils::str::xmlAtt("id", utils::str::int2str(id));
  if(this->is_signed)
    res += utils::str::xmlAtt("signed", "true");
  if(this->is_hexa)
    res += utils::str::xmlAtt("is_hexa", "true");
  if(unit.size() > 0)
    res += utils::str::xmlAtt("unit", unit);
  if(extension.size() > 0)
      res += utils::str::xmlAtt("extension", extension);
  if(default_value.size() > 0)
    res += utils::str::xmlAtt("default", get_string(default_value));
  if(constraints.size() > 0)
  {
    res += " constraints = \"";
    for(unsigned int i = 0; i < constraints.size(); i++)
    {
      res += constraints[i];
      if(i < constraints.size() - 1)
        res += std::string("|");
    }
    res += "\"";
  }
  if(enumerations.size() == 0)
    return res + "/>\n";
  res += ">\n";
  for(unsigned int i = 0; i < enumerations.size(); i++)
  {
    res += "<match "
        +  utils::str::xmlAtt("name", enumerations[i].name.get_id())
        +  utils::str::xmlAtt("value", enumerations[i].value)
        + "/>\n";
  }
  res += "</attribute>\n";
  return res;
}


SubSchema::SubSchema()
{
  display_unfold = true;
  is_hidden = false;
  is_exclusive = false;
  default_count = 0;
  readonly = false;
  display_tree = false;
  display_tab = false;
  min = -1;
  max = -1;
}

void SubSchema::operator =(const SubSchema &ss)
{
  show_header = ss.show_header;
  default_count = ss.default_count;
  is_hidden = ss.is_hidden;
  display_unfold = ss.display_unfold;
  min = ss.min;
  max = ss.max;
  ptr = ss.ptr;
  name = ss.name;
  child_str = ss.child_str;
  display_tab = ss.display_tab;
  display_tree = ss.display_tree;
  is_exclusive = ss.is_exclusive;
  readonly = ss.readonly;
  resume = ss.resume;
}

std::string SubSchema::to_string() const
{
  std::string res = "sub-schema: ";
  res += name.get_id() + ", cstr=";
  res += child_str + ", ";
  res += std::string("min = ") + utils::str::int2str(min) + ", ";
  res += std::string("max = ") + utils::str::int2str(max) + ", ";
  if(is_hidden)
    res += "hidden ";
  if(display_tab)
    res += "display-tab ";
  if(display_tree)
      res += "display-tree ";
  if(display_unfold)
    res += "display-unfold ";
  return res;
}

AttributeSchema::AttributeSchema(const Node &e)
{
  std::string typestr = e.get_attribute_as_string("type");
  name = e.get_localized();

  min = -1;
  is_ip = false;
  max = -1;
  size = 1;
  id = 0xffff;
  is_read_only = false;

  if(e.has_attribute("require"))
    requirement = e.get_attribute_as_string("require");

  if(e.has_attribute("readonly"))
    is_read_only = e.get_attribute_as_boolean("readonly");

  is_instrument = false;
  if(e.has_attribute("instrument"))
    is_instrument = e.get_attribute_as_boolean("instrument");

  is_volatile = e.has_attribute("volatile") ? e.get_attribute_as_boolean("volatile") : false;
  is_hidden = e.has_attribute("hidden") ? e.get_attribute_as_boolean("hidden") : false;
  is_hexa   = e.get_attribute_as_boolean("hexa");
  is_bytes  = e.has_attribute("bytes") ? e.get_attribute_as_boolean("bytes") : false;

  unit = e.get_attribute_as_string("unit");
  extension = e.has_attribute("extension") ? e.get_attribute_as_string("extension") : "";
  regular_exp = e.has_attribute("regular") ? e.get_attribute_as_string("regular") : "";

  if(e.has_attribute("id"))
    id = e.get_attribute_as_int("id");

  formatted_text = false;
  if(e.has_attribute("formatted-text"))
    formatted_text = e.get_attribute_as_boolean("formatted-text");



  for(const Node &sb: e.children("match"))
  {
    Enumeration en;
    en.name  = sb.get_localized();
    en.value = sb.get_attribute_as_string("value");
    if(en.value.size() == 0)
      en.value = en.name.get_id();

    if(sb.has_attribute("schema"))
      en.schema_str = sb.get_attribute_as_string("schema");
    enumerations.push_back(en);
  }
  if(typestr.compare("string") == 0)
  {
    type = TYPE_STRING;
  }
  else if(typestr.compare("ip") == 0)
  {
    type = TYPE_STRING;
    is_ip = true;
  }
  else if(typestr.compare("float") == 0)
  {
    type = TYPE_FLOAT;
    size = 4;
    is_signed = true;
  }
  else if(typestr.compare("double") == 0)
  {
    type = TYPE_FLOAT;
    size = 8;
    is_signed = true;
  }
  else if(typestr.compare("boolean") == 0)
  {
    type = TYPE_BOOLEAN;
    size = 1;
  }
  else if(typestr.compare("blob") == 0)
  {
    type = TYPE_BLOB;
    size = 3;
  }
  else
  {
    type = TYPE_INT;
    size = e.get_attribute_as_int("size");
    is_signed = e.get_attribute_as_boolean("signed");
  }

  has_max = (e.get_attribute_as_string("max").size() > 0) && (e.get_attribute_as_string("max")[0] != 'n') && (e.get_attribute_as_string("max") != "-1");
  if(has_max)
  {
    string s = e.get_attribute_as_string("max");
    ByteArray ba;
    if(serialize(ba, s) == 0)
    {
      max = get_int(ba);
    }
  }

  has_min = (e.get_attribute_as_string("min").size() > 0) && (e.get_attribute_as_string("min")[0] != 'n') && (e.get_attribute_as_int("min") != -1);
  if(has_min)
  {
    string s = e.get_attribute_as_string("min");
    ByteArray ba;
    if(serialize(ba, s) == 0)
    {
      min = get_int(ba);
    }
  }
  //min = e.get_attribute_as_int("min");

  string s = e.get_attribute_as_string("default");

  if(s.size() > 0)
  {
    default_value.clear();
    serialize(default_value, e.get_attribute_as_string("default"));
  }
  else
  {
    make_default_default_value(default_value);
  }


  assert(is_valid(default_value));

  std::string tot = e.get_attribute_as_string("constraints");
  if(tot.size() > 0)
  {

      // Parse list of match ('|' separed)
      const char *s = tot.c_str();
      char current[200];
      int current_index = 0;
      for(unsigned int i = 0; i < strlen(s); i++)
      {
          if(s[i] != '|')
          {
              current[current_index++] = s[i];
          }
          else
          {
              current[current_index] = 0;
              constraints.push_back(std::string(current));
              current_index = 0;
          }
      }
      if(current_index > 0)
      {
          current[current_index] = 0;
          constraints.push_back(std::string(current));
      }
  }
}

AttributeSchema::AttributeSchema(const MXml &mx)
{
  std::string typestr = "int";

  log.setup(string("att-schema/") + mx.get_name());

  if(mx.has_attribute("type"))
  {
    //anomaly("The XML attribute has no type: %s", mx.dump().c_str());
    //return;
    typestr = mx.get_attribute("type").to_string();
  }

  if(!mx.has_attribute("name"))
  {
    log.anomaly("The XML attribute has no name: %s", mx.dump().c_str());
    return;
  }

  is_ip = false;

  name = Localized(mx);
  //log.setup("model", std::string("attribute-schema/") + name.get_id());
  log.setup("model");
  min = -1;
  max = -1;
  has_min = false;
  has_max = false;
  is_hexa = false;
  is_bytes = false;
  unit = "";
  extension = "";
  size = 1;
  //default_value = "";
  is_hidden = false;
  is_volatile = false;
  is_signed = false;
  id = 0xffff;
  is_read_only = false;

  formatted_text = false;
  if(mx.has_attribute("formatted-text"))
    formatted_text = mx.get_attribute("formatted-text").to_bool();

  is_instrument = false;
  if(mx.has_attribute("instrument"))
    is_instrument = mx.get_attribute("instrument").to_bool();

  //count = 1;

  if(mx.has_attribute("require"))
    requirement = mx.get_attribute("require").to_string();

  regular_exp = "";
  if(mx.has_attribute("regular"))
    regular_exp = mx.get_attribute("regular").to_string();

  //if(mx.has_attribute("count"))
   // count = mx.get_attribute("count").to_int();

  if(mx.has_attribute("readonly"))
    is_read_only = mx.get_attribute("readonly").to_bool();

  //trace("%s: readonly = %s.", name.c_str(), is_read_only ? "true" : "false");

  /*if(mx.has_attribute("en"))
    en = mx.get_attribute("en").to_string();

  if(mx.has_attribute("fr"))
    fr = mx.get_attribute("fr").to_string();*/

  if(mx.has_attribute("id"))
    id = mx.get_attribute("id").to_int();

  if(mx.has_attribute("size"))
    size = mx.get_attribute("size").to_int();

  if(mx.has_attribute("signed"))
    is_signed = mx.get_attribute("signed").to_bool();

  if(mx.has_attribute("hidden"))
    is_hidden = mx.get_attribute("hidden").to_bool();

  if(mx.has_attribute("volatile"))
    is_volatile = mx.get_attribute("volatile").to_bool();
  if(mx.has_attribute("hexa"))
    is_hexa = mx.get_attribute("hexa").to_bool();
  if(mx.has_attribute("bytes"))
    is_bytes = mx.get_attribute("bytes").to_bool();

  if(mx.has_attribute("unit"))
    unit = mx.get_attribute("unit").to_string();
  if(mx.has_attribute("extension"))
    extension = mx.get_attribute("extension").to_string();
  if(mx.has_attribute("max"))
  {
    has_max = true;
    max     = mx.get_attribute("max").to_int();
  }
  if(mx.has_attribute("min"))
  {
    has_min = true;
    min     = mx.get_attribute("min").to_int();
  }
  std::vector<MXml> lst = mx.get_children("match");
  for(unsigned int i = 0; i < lst.size(); i++)
  {
    Enumeration e;
    e.name  = Localized(lst[i]);//.get_attribute("name").to_string();
    if(lst[i].has_attribute("value"))
      e.value = lst[i].get_attribute("value").to_string();
    else
      e.value = e.name.get_id();
    /*e.en = e.name;
    e.fr = e.name;
    if(lst[i].has_attribute("en"))
      e.en = lst[i].get_attribute("en").to_string();
    if(lst[i].has_attribute("fr"))
      e.fr = lst[i].get_attribute("fr").to_string();*/

    if(lst[i].has_attribute("schema"))
        e.schema_str = lst[i].get_attribute("schema").to_string();

    /*if(lst[i].hasChild("description"))
    {
      e.description = lst[i].get_child("description").dumpContent();
    }*/

    enumerations.push_back(e);
  }
  if(typestr.compare("string") == 0)
  {
    type = TYPE_STRING;
  }
  else if(typestr.compare("ip") == 0)
  {
    type = TYPE_STRING;
    is_ip = true;
  }
  else if(typestr.compare("float") == 0)
  {
    type = TYPE_FLOAT;
    size = 4;
    is_signed = true;
  }
  else if(typestr.compare("double") == 0)
  {
    type = TYPE_FLOAT;
    size = 8;
    is_signed = true;
  }
  else if(typestr.compare("boolean") == 0)
  {
    type = TYPE_BOOLEAN;
    size = 1;
  }
  else if(typestr.compare("color") == 0)
  {
    type = TYPE_COLOR;
    size = 3;
  }
  else if(typestr.compare("date") == 0)
  {
    type = TYPE_DATE;
    size = 3;
  }
  else if(typestr.compare("folder") == 0)
  {
    type = TYPE_FOLDER;
    size = 3;
  }
  else if(typestr.compare("file") == 0)
  {
    type = TYPE_FILE;
    size = 3;
  }
  else if(typestr.compare("serial") == 0)
  {
    type = TYPE_SERIAL;
    size = 3;
  }
  else if(typestr.compare("blob") == 0)
  {
    type = TYPE_BLOB;
    size = 3;
  }
  else
  {
    type = TYPE_INT;
  }

  default_value.clear();
  if(mx.has_attribute("default"))
  {

    serialize(default_value, mx.get_attribute("default").to_string());
              //default_value = mx.get_attribute("default").to_string();
  }
  else
  {
    make_default_default_value(default_value);
  }

  assert(is_valid(default_value));

  if(mx.has_attribute("constraints"))
  {
      std::string tot = mx.get_attribute("constraints").to_string();
      // Parse list of match ('|' separed)
      const char *s = tot.c_str();
      char current[200];
      int current_index = 0;
      for(unsigned int i = 0; i < strlen(s); i++)
      {
          if(s[i] != '|')
          {
              current[current_index++] = s[i];
          }
          else
          {
              current[current_index] = 0;
              constraints.push_back(std::string(current));
              current_index = 0;
          }
      }
      if(current_index > 0)
      {
          current[current_index] = 0;
          constraints.push_back(std::string(current));
      }
  }
}

void AttributeSchema::make_default_default_value(ByteArray &res) const
{
  res.clear();

  if(type == TYPE_INT)
  {
    for(int i = 0; i < size; i++)
      res.putc(0);
  }
  else if(type == TYPE_FLOAT)
  {
    res.putf(0.0);
  }
  else if(type == TYPE_BOOLEAN)
  {
    res.putc(0);
  }
  else if(type == TYPE_BLOB)
  {
    /* (empty) */
  }
  else
  {
    res.puts("");
  }
}

AttributeSchema::AttributeSchema(const AttributeSchema &c)
{
  *this = c;
  log.setup("model");
  //log.setup("model", std::string("attribute-schema/") + c.name.get_id());
}

void AttributeSchema::operator =(const AttributeSchema &c)
{
  enumerations.clear();
  constraints.clear();
  regular_exp = c.regular_exp;
  is_ip = c.is_ip;
  id = c.id;
  name = c.name;
  type = c.type;
  size = c.size;
  is_signed = c.is_signed;
  is_hexa = c.is_hexa;
  is_bytes = c.is_bytes;
  is_volatile = c.is_volatile;
  unit = c.unit;
  extension = c.extension;
  min = c.min;
  max = c.max;
  has_min = c.has_min;
  has_max = c.has_max;
  enumerations = c.enumerations;
  is_unique = c.is_unique;
  is_hidden = c.is_hidden;
  constraints = c.constraints;
  is_read_only = c.is_read_only;
  requirement = c.requirement;
  is_instrument = c.is_instrument;
  formatted_text = c.formatted_text;
  default_value.clear();
  default_value = c.default_value;
  assert(c.is_valid(c.default_value));
  assert(is_valid(default_value));
  //log.setup("model", string("att-schema<") + name.get_id() + ">");
}

std::string AttributeSchema::type2string() const
{
  switch(type)
  {
  case TYPE_STRING:  return "string";
  case TYPE_BOOLEAN: return "boolean";
  case TYPE_FLOAT:   if(size == 4) return "float"; else return "double";
  case TYPE_INT:
    return "int";
  case TYPE_COLOR: return "color";
  case TYPE_BLOB: return "blob";
  case TYPE_FOLDER: return "folder";
  case TYPE_FILE: return "file";
  case TYPE_SERIAL: return "serial";
  case TYPE_DATE: return "date";
  }
  log.anomaly("type2string");
  return "error";
}


Attribute::Attribute()
{
  inhibit_event_dispatch = false;
  node = nullptr;
  parent = nullptr;
}

Attribute::Attribute(refptr<AttributeSchema> schema)
{
  node = nullptr;
  parent = nullptr;
  this->schema = schema;
  inhibit_event_dispatch = true;
  set_value(schema->default_value);
  inhibit_event_dispatch = false;
}




int    AttributeSchema::get_int    (const ByteArray &ba) const
{
  if(type == TYPE_FLOAT)
  {
    return (int) get_float(ba);
  }
  else if(type == TYPE_INT)
  {
    ByteArray tmp(ba);

    if((unsigned int) size != ba.size())
    {
      log.anomaly("get_int(): size in schema = %d, in data = %d.", size, ba.size());
      return 0;
    }

    if(size == 8)
    {
      uint32_t rs = tmp.popL();
      if(is_signed)
        return (int32_t) rs;
      else
        return rs;
    }
    else if(size == 4)
    {
      uint32_t rs = tmp.popl();
      if(is_signed)
        return (int32_t) rs;
      else
        return rs;
    }
    else if(size == 2)
    {
      uint16_t c = tmp.popw();
      if(is_signed)
      {
        return (int) ((int16_t) c);
      }
      return c;
    }
    else if(size == 1)
    {
      uint8_t c = tmp.popc();
      if(is_signed)
      {
        return (int) ((int8_t) c);
      }
      return c;
    }

    log.anomaly("get int: unmanaged size = %d.", size);

    return 0;
  }
  else
  {
    if((type == TYPE_STRING) && (enumerations.size() > 0))
    {
      string s = get_string(ba);
      for(unsigned int i = 0; i < enumerations.size(); i++)
      {
        if(s.compare(enumerations[i].name.get_id()) == 0)
          return atoi(enumerations[i].value.c_str());
      }
      return 0;
    }
  }
  return 0;
}

bool   AttributeSchema::get_boolean(const ByteArray &ba) const
{
  ByteArray tmp(ba);
  return tmp.popc() != 0;
}

float  AttributeSchema::get_float  (const ByteArray &ba) const
{
  if(type == TYPE_FLOAT)
  {
    ByteArray tmp(ba);
    return tmp.popf();
  }
  else if(type == TYPE_INT)
  {
    return get_int(ba);
  }
  else
  {
    log.anomaly("AttributeSchema::get_float(): invalid type.");
    return 0.0;
  }
}

string AttributeSchema::get_string (const ByteArray &ba) const
{
  string s;

  switch(type)
  {
    case TYPE_INT:
    {
      int i = get_int(ba);
      if(is_hexa)
        s = string("0x") + utils::str::int2strhexa(i);
      else
        s = utils::str::int2str(i);
      return s;
    }
    case TYPE_BOOLEAN:
    {
      return get_boolean(ba) ? "true" : "false";
    }
    case TYPE_FLOAT:
    {
      float f = get_float(ba);
      char buf[500];
      sprintf(buf, "%f", f);
      return string(buf);
    }
    default:
    case TYPE_FOLDER:
    case TYPE_STRING:
    {
      ByteArray tmp(ba);
      return tmp.pops();
    }
  }
  string id = name.get_id();
  log.anomaly("%s: type unspecified, att name = %s.", __func__, id.c_str());
  return "?";
}

bool  AttributeSchema::is_valid(const ByteArray &ba) const
{
  if(type == TYPE_INT)
  {
    if((unsigned int) size != ba.size())
    {
      return false;
    }
  }
  else if(type == TYPE_FLOAT)
  {
    if(ba.size() != 4)
      return false;
  }
  else if(type == TYPE_BOOLEAN)
  {
    if(ba.size() != 1)
      return false;
  }


  return true;
}

int    AttributeSchema::serialize(ByteArray &ba, int value)    const
{
  if(type == TYPE_FLOAT)
  {
    ba.putf((float) value);
    return 0;
  }


  if(type != TYPE_INT)
  {
    log.warning("serialize(int): not an int!");
  }

  if(size == 8)
    ba.putL(value);
  else if(size == 4)
    ba.putl(value);
  else if(size == 2)
    ba.putw(value);
  else if(size == 1)
    ba.putc(value);
  return 0;
}

int    AttributeSchema::serialize(ByteArray &ba, bool value)   const
{
  ba.putc(value ? 0x01 : 0x00);
  return 0;
}

int    AttributeSchema::serialize(ByteArray &ba, float value)  const
{
  if(type == TYPE_INT)
    return serialize(ba, (int) value);
  ba.putf(value);
  return 0;
}

int    AttributeSchema::serialize(ByteArray &ba, string value) const
{
  const AttributeSchema &as = *this;
  switch(as.type)
  {
    /*case TYPE_BLOB:
    {
      ba.putl(blob.size());
      ba.put(blob);
      break;
      }*/
    case TYPE_INT:
    {

      int val = 0;


      if(is_hexa)
      {
        std::string s = value;
        const char *c = s.c_str();
        if(((c[0] == '0') && (c[1] == 'x'))
           || ((c[0] == '0') && (c[1] == 'X')))
        {
          sscanf(c+2, "%x", &val);
        }
        else
          val = atoi(s.c_str());
      }
      else
      {
        val = atoi(value.c_str());
      }

      if(as.size == 1)
      {
        uint8_t v = (uint8_t) val;
        ba.putc(v);
      }
      else if(as.size == 2)
      {
        uint16_t v = (uint16_t) val;
        ba.putw(v);
      }
      else if(as.size == 4)
      {
        uint32_t v = (uint32_t) val;
        ba.putl(v);
      }
      else if(as.size == 8)
      {
        uint64_t v = (uint64_t) val;
        ba.putL(v);
      }
      break;
    }

    case TYPE_BOOLEAN:
    {
      if(value.compare("true") == 0)
        ba.putc(1);
      else
        ba.putc(0);
      break;
    }
    case TYPE_FLOAT:
    {
      std::string s = value;
      float result;

      for(unsigned int i = 0; i < s.size(); i++)
      {
        if(s[i] == ',')
          s[i] = '.';
      }

      std::istringstream istr(s);
      istr.imbue(std::locale("C"));
      istr >> result;

      ba.putf(result);

        //result = atof(s.c_str());
        //printf("get_float: %s -> %f\n", s.c_str(), result);

        //printf("get_float: %s -> %f\n", s.c_str(), result);

        //return result;

      break;
    }
    default:
    case TYPE_STRING:
    {
      ba.puts(value);
      break;
    }
  }
  return 0;
}



void Attribute::serialize(ByteArray &ba) const
{
  ba.put(value);
}

void Attribute::unserialize(ByteArray &ba)
{
  ByteArray new_value;
  //value.clear();

  //trace("unserialize()...");

  switch(schema->type)
  {
    case TYPE_INT:
    {
      ba.pop(new_value, schema->size);
      //trace("int unserialization: size = %d, new size = %d.", schema->size, new_value.size());
      break;
    }
    case TYPE_FLOAT:
    {
      ba.pop(new_value, 4);
      break;
    }
    case TYPE_BLOB:
    {
      uint32_t len = ba.popl();
      ba.pop(new_value, len);
      break;
    }
    case TYPE_BOOLEAN:
    {
      ba.pop(new_value, 1);
      break;
    }
    case TYPE_STRING:
    default:
    {
      string s = ba.pops();
      new_value.puts(s);
      break;
    }
  }

  if(value != new_value)
  {
    value = new_value;
    //trace("unserialization: size = %d.", value.size());

    if(!inhibit_event_dispatch)
    {
      value_changed();
      ChangeEvent ce = ChangeEvent::create_att_changed(this);
      //ce.source = this;
      ce.path = XPath(XPathItem(schema->name.get_id()));
      //ce.source_node = nullptr;//new Node(parent);
      dispatch(ce);
      //delete ce.source_node;
    }
  }


}

float Attribute::get_float() const
{
  return schema->get_float(value);
}

bool Attribute::get_boolean() const
{
  return schema->get_boolean(value);
}

int Attribute::get_int() const
{
  return schema->get_int(value);
}

void             Node::add_listener(CListener<ChangeEvent> *lst)
{
  if(data != nullptr)
    data->CProvider<ChangeEvent>::add_listener(lst);
}

void             Node::remove_listener(CListener<ChangeEvent> *lst)
{
  if(data != nullptr)
    data->CProvider<ChangeEvent>::remove_listener(lst);
}



int NodeSchema::get_sub_index(const string &name) const
{
  map<string, int>::const_iterator it = mapper.find(name);
  if(it == mapper.end())
    return -1;
  return it->second;
}

void RamNode::remove_child(Node child)
{
  int index = schema->get_sub_index(child.schema()->name.get_id());

  if(index == -1)
  {
    log.anomaly("%s: no such child.", __func__);
    return;
  }

  std::deque<Node>::iterator it;
  for (it = children[index].nodes.begin(); it != children[index].nodes.end(); it++)
  {
    if(child == *it)
    {
      children[index].nodes.erase(it);
      return;
    }
  }

  log.anomaly("Can't erase children.");
}

Node RamNode::add_child(const string &sub_name)
{
  int index = schema->get_sub_index(sub_name);

  if(index == -1)
  {
    //Node n(this);
    //trace(n.to_xml(0,true));
    /*trace(schema->to_string());

    trace("Mapper:");
    std::map<string,int>::iterator it;
    for(it = schema->mapper.begin(); it != schema->mapper.end(); it++)
    {
      trace("map(%s) = %d.", (*it).first.c_str(), (*it).second);
    }*/

    log.anomaly("%s: no such child: %s.", __func__, sub_name.c_str());
    return Node();
  }

  SubSchema &ss = schema->children[index];

  Node nv(new RamNode(ss.ptr));

  RamNode *nvram = (RamNode *) nv.data;
  children[index].nodes.push_back(nv);
  nvram->parent   = this;
  nvram->instance = children[index].nodes.size() - 1;
  nvram->sub_type = sub_name;
  nvram->type     = sub_name;
  return nv;
}



void RamNode::on_event(const ChangeEvent &ce)
{
  event_detected = true;

  //trace(ce.to_string());

  if((ce.type == ChangeEvent::GROUP_CHANGE) && (inhibit_event_raise))
    return;

  // must not dispatch a group change to its parent
  // but to external yes !

  // Dispatch all normal events to parent
  if(ce.type != ChangeEvent::GROUP_CHANGE)
  {
    ChangeEvent nce = ce;
    XPathItem xpi(schema->name.get_id(), instance);
    nce.path = ce.path.add_parent(xpi);
    //trace("ram_node::on_event -> dispatch... %s", nce.to_string().c_str());
    CProvider<ChangeEvent>::dispatch(nce);
  }

  // For each normal event, create a group change
  if((ce.type != ChangeEvent::GROUP_CHANGE) && (!inhibit_event_raise))
  {
    ChangeEvent nce;
    nce.type = ChangeEvent::GROUP_CHANGE;
    XPathItem xpi(schema->name.get_id(), instance);
    nce.path = XPath(xpi);
    CProvider<ChangeEvent>::dispatch(nce);
  }
}



bool Node::get_attribute_as_boolean(const string &name) const
{
  const Attribute *att = get_attribute(name);
  if(att == nullptr)
    return false;
  return att->get_boolean();
}

std::string Node::get_attribute_as_string(const string &name) const
{
  const Attribute *att = get_attribute(name);
  if(att != nullptr)
    return att->get_string();
  return "";
}

ByteArray Node::get_attribute_as_raw(const string &name) const
{
  const Attribute *att = get_attribute(name);
  if(att != nullptr)
    return att->value;
  return ByteArray();
}

float Node::get_attribute_as_float(const string &name) const
{
  const Attribute *att = get_attribute(name);
  if(att != nullptr)
    return att->get_float();
  return 0.0;
}

int Node::get_attribute_as_int(const string &name) const
{
  const Attribute *att = get_attribute(name);
  if(att != nullptr)
    return att->get_int();
  return 0;
}

XPath::XPath(const char *s)
{
  from_string(string(s));
}

XPath::XPath(const XPathItem &xpi)
{
  items.push_back(xpi);
  valid = true;
}

void Node::serialize(ByteArray &res) const
{
  uint32_t i, n, m;

  /* Serialize contents */
  n = schema()->attributes.size();
  for(i = 0; i < n; i++)
  {
    Attribute *att = data->get_attribute_at(i);
    att->serialize(res);
  }

  n = schema()->children.size();
  for(i = 0; i < n; i++)
  {
    std::string cname = schema()->children[i].name.get_id();
    m = get_children_count(cname);
    res.putl(m);
    for(const Node &child: children(cname))
    {
      child.serialize(res);
    }
  }
}

void Node::unserialize(ByteArray &source)
{
  uint32_t i, j, k, n, m;

  if(data == nullptr)
  {
    log.anomaly("%s: data == nullptr.", __func__);
    return;
  }

  data->inhibit_event_raise = true;
  data->event_detected      = false;

  /* Serialize contents */
  n = schema()->attributes.size();
  for(i = 0; i < n; i++)
  {
    data->get_attribute_at(i)->unserialize(source);
  }
    //get_attribute(schema()->attributes[i]->name.get_id())->unserialize(source);

  n = schema()->children.size();
  for(i = 0; i < n; i++)
  {
    std::string cname = schema()->children[i].name.get_id();

    // m is the new number of children

    if(source.size() < 4)
    {
      log.anomaly("unserialization: incomplete source.");
      return;
    }

    m = source.popl();

    // k is the old number of children
    k = get_children_count(cname);

    //trace("unserialize type %s: local count = %d, new count = %d.",
    //      cname.c_str(), k, m);

    // Update existing nodes
    for(j = 0; (j < k) && (j < m); j++)
      get_child_at(cname, j).unserialize(source);

    // Add new nodes
    if(m > k)
    {
      for(j = 0; j + k < m; j++)
      {
        Node e = add_child(schema()->children[i].ptr);
        e.unserialize(source);
      }
    }

    // Remove old nodes
    if(m < k)
    {
      for(j = 0; j + m < k; j++)
        remove_child(get_child_at(cname, k - 1 - j));
    }
  }

  data->inhibit_event_raise = false;

  if(data->event_detected)
  {
    ChangeEvent ce;
    XPathItem xpi(schema()->name.get_id(), data->instance);
    ce.path = XPath(xpi);
    ce.type = ChangeEvent::GROUP_CHANGE;
    data->dispatch(ce);
  }
}

int Node::get_path_to(const Node &child, XPath &res)
{
  unsigned int j;

  if(schema() == nullptr)
  {
    log.anomaly("get_path_to: no schema.");
    return -1;
  }

  if(child == *this)
  {
    res = XPath();
    return 0;
  }

  for(SubSchema &ss: schema()->children)
  {
    std::string sub_name = ss.name.get_id();//ss.ptr->name.get_id();
    //log.trace("check sub %s: %d elems.", sub_name.c_str(), get_children_count(sub_name));
    j = 0;
    for(Node cld: children(sub_name))
    {
      XPath intpath;
      if(cld.get_path_to(child, intpath) == 0)
      {
        XPathItem xpi;

        if((ss.default_key.size() > 0) && (cld.has_attribute(ss.default_key)))
        {
          xpi.att_name  = ss.default_key;
          xpi.att_value = cld.get_attribute_as_string(ss.default_key);
        }
        else
        {
          xpi.instance  = j;
        }

        xpi.name      = sub_name;
        res           = intpath.add_parent(xpi);
        return 0;
      }
      j++;
    }
  }
  return -1;
}

XPathItem::XPathItem()
{
  name = "";
  instance = 0;
}

XPathItem::XPathItem(const string &name, int instance)
{
  this->name = name;
  this->instance = instance;
}

XPathItem::XPathItem(const string &name, const string &att_name, const string &att_value)
{
  instance = -1;
  this->att_name = att_name;
  this->att_value = att_value;
}

XPathItem::~XPathItem()
{
}

XPath::XPath(const XPath &root, const string &leaf, int instance)
{
  *this = root;
  XPathItem xpi(leaf, instance);
  items.push_back(xpi);
}



Node Node::get_child(const XPath &path)
{
  Node res;

  if(!path.is_valid())
  {
    log.anomaly("get_child_from_path(): invalid path.");
    return res;
  }

  if(path.length() == 0)
    return *this;

  XPathItem root = path.root();

  //if(root.name.compare("") == 0)
  //return root_model.get_child(path.child());

  if(root.name.compare("..") == 0)
  {
    if(parent().is_nullptr())
    {
      log.anomaly("get_child_from_path(%s): no parent.",
              path.c_str());
      return res;
    }

    return parent().get_child(path.child());
  }

  if(root.name.compare(".") == 0)
  {
    return get_child(path.child());
  }


  if(!this->has_child(root.name))
  {
    log.warning("model = %s\n", to_xml(0,true).c_str());
    log.anomaly("get_child_from_path(%s): no such child: %s, len = %d.",
            path.c_str(),
            root.name.c_str(),
            path.length());
    return res;
  }

  int instance = root.instance;
  if(instance < 0)
    instance = 0;

  if(root.att_name.size() > 0)
  {
    bool found = false;
    Node child;

    for(Node ch: children(root.name))
    {
      if(ch.get_attribute_as_string(root.att_name).compare(root.att_value) == 0)
      {
        found = true;
        child = ch;
      }
    }

    if(!found)
    {
      log.anomaly("get_child_from_path(%s: %s = %s): no such child.",
              path.c_str(), root.att_name.c_str(), root.att_value.c_str());
      return res;
    }
    return child.get_child(path.child());
  }

  if(instance >= (int) get_children_count(root.name))
  {
    log.anomaly("get_child_from_path(%s): no such child instance: %s[%d].",
            path.c_str(), root.name.c_str(), instance);
    log.trace("model = %s\n", to_xml(0,true).c_str());
    return res;
  }

  return get_child_at(root.name, instance).get_child(path.child());
}

const Node Node::get_child(const XPath &path) const
{
  Node res;

  if(!path.is_valid())
  {
    log.anomaly("get_child_from_path(): invalid path.");
    return res;
  }

  if(path.length() == 0)
    return *this;

  XPathItem root = path.root();

  //if(root.name.compare("") == 0)
  //return root_model.get_child(path.child());

  if(root.name.compare("..") == 0)
  {
    if(parent().is_nullptr())
    {
      log.anomaly("get_child_from_path(%s): no parent.",
              path.c_str());
      return res;
    }

    return parent().get_child(path.child());
  }

  if(root.name.compare(".") == 0)
  {
    return get_child(path.child());
  }


  if(!this->has_child(root.name))
  {
    log.warning("model = %s\n", to_xml(0,true).c_str());
    log.anomaly("get_child_from_path(%s): no such child: %s, len = %d.",
            path.c_str(),
            root.name.c_str(),
            path.length());
    return res;
  }

  int instance = root.instance;
  if(instance < 0)
    instance = 0;

  if(root.att_name.size() > 0)
  {
    bool found = false;
    Node child;

    for(Node ch: children(root.name))
    {
      if(ch.get_attribute_as_string(root.att_name).compare(root.att_value) == 0)
      {
        found = true;
        child = ch;
      }
    }

    if(!found)
    {
      log.anomaly("get_child_from_path(%s: %s=%s): no such child.",
              path.c_str(), root.att_name.c_str(), root.att_value.c_str());
      return res;
    }
    return child.get_child(path.child());
  }

  if(instance >= (int) get_children_count(root.name))
  {
    log.anomaly("get_child_from_path(%s): no such child instance: %s[%d].",
            path.c_str(), root.name.c_str(), instance);
    log.trace("model = %s\n", to_xml(0,true).c_str());
    return res;
  }

  return get_child_at(root.name, instance).get_child(path.child());
}



std::string Node::description() const
{
  if(has_child("description"))
  {
    std::string res = get_child_at("description", 0).get_attribute_as_string("content");
    for(unsigned int i = 0; i < get_children_count("description"); i++)
    {
      const Node &desc = get_child_at("description", i);
      if(desc.get_attribute_as_string("lang").compare(langue.current_language) == 0)
      {
        return desc.get_attribute_as_string("content");
      }
    }
    return res;
  }
  else
  {
    return "";
  }
}

bool Node::has_attribute(const XPath &path) const
{
  if(data == nullptr)
    return false;

  if(path.length() == 1)
    return (schema()->att_mapper.count(path[0].name) > 0);

  Node owner = get_child(path.remove_last());
  if(owner.is_nullptr())
    return false;

  return owner.has_attribute(path.get_last());
}


string Node::get_fullpath() const
{
  string res = schema()->name.get_id();

  if(!parent().is_nullptr())
    res = parent().get_fullpath() + "/" + res;

  return res;
}

Attribute *Node::get_attribute(const XPath &path)
{
  if(path.length() == 1)
  {
    if(schema()->att_mapper.count(path[0].name) == 0)
    {
      string fpath = get_fullpath() + "/" + path[0].name;
      log.anomaly("Attribute not found: %s. Current path = %s.", path.c_str(), fpath.c_str());
      return nullptr;
    }
    int index = schema()->att_mapper[path[0].name];
    return data->get_attribute_at(index);
  }

  Node owner = get_child(path.remove_last());

  if(owner.is_nullptr())
  {
    string fpath = get_fullpath();
    log.anomaly("Attribute not found: %s. Current path = %s.", path.c_str(), fpath.c_str());
    return nullptr;
  }

  return owner.get_attribute(path.get_last());
}

const Attribute *Node::get_attribute(const XPath &path) const
{
  if(path.length() == 1)
  {
    if(schema()->att_mapper.count(path[0].name) == 0)
    {
      log.anomaly("Attribute not found: %s. Aborting...", path.c_str());
      return nullptr;
    }
    int index = schema()->att_mapper[path[0].name];
    return data->get_attribute_at(index);
  }

  const Node owner = get_child(path.remove_last());
  if(owner.is_nullptr())
  {
    log.anomaly("Attribute not found: %s. Aborting...", path.c_str());
    return nullptr;
  }

  return owner.get_attribute(path.get_last());
}

int Node::set_attribute(const XPath &path, const ByteArray &value)
{
  Attribute *att = get_attribute(path);

  if(att == nullptr)
    return -1;

  att->set_value(value);
  return 0;
}



int Node::set_attribute(const XPath &path, const string &value)
{
  Attribute *att = get_attribute(path);

  if(att == nullptr)
    return -1;

  att->set_value(value);

  return 0;
}

int Node::set_attribute(const XPath &path, bool value)
{
  if(value)
    return set_attribute(path, std::string("true"));

  return set_attribute(path, std::string("false"));
}

int Node::set_attribute(const XPath &path, int value)
{
  Attribute *att = get_attribute(path);

  if(att == nullptr)
    return -1;

  att->set_value(value);

  return 0;
}

int Node::set_attribute(const XPath &path, float value)
{
  Attribute *att = get_attribute(path);

  if(att == nullptr)
    return -1;

  att->set_value(value);

  return 0;
}

int Node::set_attribute(const XPath &name, const char *value)
{
  return set_attribute(name, string(value));
}

unsigned long RamNode::get_attribute_count() const
{
  return attributes.size();
}

const Attribute *RamNode::get_attribute_at(unsigned int i) const
{
  if(i >= attributes.size())
  {
    log.anomaly("%s: invalid att index:%d/%d", i, attributes.size());
    return nullptr;
  }
  return &attributes[i];
}

Attribute *RamNode::get_attribute_at(unsigned int i)
{
  if(i >= attributes.size())
  {
    log.anomaly("%s: invalid att index:%d/%d", __func__, i, attributes.size());
    return nullptr;
  }
  return &attributes[i];
}

std::string Node::get_identifier(bool disp_type, bool bold) const
{
  // TODOOOOOOOOOOOOOOOOOOOOOOOOOO

  std::string str_type = schema()->name.get_localized();

  if(str_type.compare("?") == 0)
  {
    log.anomaly("%s: schema name is not defined.", __func__);
  }

  if(langue.has_item(str_type))
    str_type = langue.get_item(str_type);

  /** If has a parent, preferably take the name of sub */
  Node prt = parent();
  if(!prt.is_nullptr())
  {
    NodeSchema *es = prt.schema();
    for(unsigned int i = 0; i < es->children.size(); i++)
    {
      SubSchema &ss = es->children[i];
      if((ss.child_str.compare(schema()->name.get_id()) == 0)
          && (ss.name.get_id().compare(ss.child_str)))
      {
        str_type = ss.name.get_localized();
      }
    }
  }

  if(!this->has_attribute("name"))
    return str_type;

  if(!disp_type)
  {
    auto lgs = Localized::language_list();

    for(auto lg: lgs)
    {
      auto id = Localized::language_id(lg);
      //printf("Check: %s.\n", id.c_str());
      if((langue.current_language.compare(id) == 0)
	 && has_attribute(id) && (get_attribute_as_string(id).size() > 0))
	return get_attribute_as_string(id);
    }

    // Default to english if available and no other translation available
    if(has_attribute("en") && (get_attribute_as_string("en").size() > 0))
      return get_attribute_as_string("en");
    return get_attribute_as_string("name");
  }

  if((langue.current_language.compare("fr") == 0))
  {
    std::string nm;
    if(has_attribute("fr"))
      nm = get_attribute_as_string("fr");
    if(nm.size() == 0)
      nm = name();
    if(bold)
      return str_type + " <b>" + nm + "</b>";
    else
      return str_type + " " + nm;
  }
  else
  {
    if(bold)
      return std::string("<b>") + name() + "</b> " + str_type ;
    else
      return name() + " " + str_type;
  }
}

unsigned long RamNode::get_children_count(const string &type) const
{
  int index = schema->get_sub_index(type);

  if(index == -1)
    return 0;

  if(index >= (int) children.size())
  {
    log.anomaly("get_children_count(%s): unitialized container.", type.c_str());
    return 0;
  }

  return children[index].nodes.size();
}

unsigned long RamNode::get_children_count(int type) const
{
  if(type >= (int) children.size())
  {
    log.anomaly("get_children_count(%d): unitialized container.", type);
    return 0;
  }

  return children[type].nodes.size();
}

std::string       Node::type() const
{
  if(data == nullptr)
    return "";
  return data->type;
}

NodeSchema *Node::schema() const
{
  if(data == nullptr)
  {
    printf("No schema.\n");
    fflush(0);
    return nullptr;
  }
  return data->schema;
}

const Node RamNode::get_child_at(const string &type, unsigned int instance) const
{
  int index = schema->get_sub_index(type);

  if(index < 0)
  {
    log.anomaly("get_children_at(%s,%d): invalid type.", type.c_str(), instance);
    return Node();
  }

  if(index >= (int) children[index].nodes.size())
  {
    log.anomaly("get_children_at(%s,%d): invalid index.", type.c_str(), instance);
    return Node();
  }

  return children[index].nodes[instance];
}

const Node RamNode::get_child_at(int type, unsigned int instance) const
{
  if(type >= (int) children.size())
  {
    log.anomaly("%s: invalid type.", __func__);
    return Node();
  }

  if(instance >= children[type].nodes.size())
  {
    log.anomaly("get_children_at(%d,%d): invalid index.", type, instance);
    return Node();
  }

  return children[type].nodes[instance];
}

Node RamNode::get_child_at(int type, unsigned int instance)
{
  if(type >= (int) children.size())
  {
    log.anomaly("%s: invalid type.", __func__);
    return Node();
  }

  if(instance >= children[type].nodes.size())
  {
    log.anomaly("get_children_at(%d,%d): invalid index.", type, instance);
    return Node();
  }

  return children[type].nodes[instance];
}

Node RamNode::get_child_at(const string &type, unsigned int instance)
{
  int index = schema->get_sub_index(type);

  if(index < 0)
  {
    log.anomaly("get_child_at(%s,%d): invalid type.", type.c_str(), instance);
    return Node();
  }

  if(index >= (int) children.size())
  {
    log.anomaly("get_child_at(%s,%d): Child container not ready.",
            type.c_str(), instance);
    return Node();
  }

  if(instance >= children[index].nodes.size())
  {
    log.anomaly("get_child_at(%s,%d): invalid index (max = %d).",
            type.c_str(), instance, ((int) children[index].nodes.size()) - 1);
    return Node();
  }

  return children[index].nodes[instance];
}


bool Node::has_child(const XPath &path) const
{
  if(path.length() == 0)
  {
    log.anomaly("has_child("")!");
    return true;
  }
  else if(path.length() == 1)
  {

    /* Must check an attribute value */
    if(path[0].att_name.size() > 0)
    {
      for(const Node child: children(path[0].name))
      {
        if(child.get_attribute_as_string(path[0].att_name).compare(path[0].att_value) == 0)
          return true;
      }
      return false;
    }
    else
      return (get_children_count(path.to_string()) > 0);
  }
  else
  {
    XPath first = path.get_first();
    return get_child(first).has_child(path.child());
  }
}


void Node::copy_from(const Node e)
{
  unsigned int i, n;

  // Must group different change events
  // into a single one.

  if(data == nullptr)
    return;

  data->inhibit_event_raise = true;
  data->event_detected      = false;

  n = e.get_reference_count();

  for(i = 0; i < n; i++)
  {
    std::string name;
    Node ref = e.get_reference_at(i, name);
    //trace("copy ref.");
    set_reference(name, ref);
  }

  for(unsigned int i = 0; i < e.data->get_attribute_count(); i++)
  {
    Attribute *att = e.data->get_attribute_at(i);
    set_attribute(att->schema->name.get_id(), att->value);
  }

  for(SubSchema &ss: schema()->children)
  {
    uint32_t dim0 = get_children_count(ss.name.get_id());
    uint32_t dim1 = e.get_children_count(ss.name.get_id());

    if(dim0 == dim1)
    {
      for(unsigned int j = 0; j < dim0; j++)
      {
        get_child_at(ss.name.get_id(), j).copy_from(e.get_child_at(ss.name.get_id(), j));
      }
    }
    else if(dim0 < dim1)
    {
      //trace_major("%d<%d (ADD) on %s.", dim0, dim1, ss.name.c_str());

      for(unsigned int j = 0; j < dim0; j++)
        get_child_at(ss.name.get_id(), j).copy_from(e.get_child_at(ss.name.get_id(), j));

      for(unsigned int j = 0; j < (dim1 - dim0); j++)
        add_child(e.get_child_at(ss.name.get_id(), dim0 + j));
    }
    else if(dim0 > dim1)
    {
      //trace_major("%d>%d (DEL) on %s.", dim0, dim1, ss.name.c_str());
      for(unsigned int j = 0; j < dim1; j++)
        get_child_at(ss.name.get_id(), j).copy_from(e.get_child_at(ss.name.get_id(), j));
      for(unsigned int j = 0; j < (dim0 - dim1); j++)
        remove_child(get_child_at(ss.name.get_id(), dim1));
    }
  }

  data->inhibit_event_raise = false;

  if(data->event_detected)
  {
    ChangeEvent ce;
    XPathItem xpi(schema()->name.get_id(), data->instance);
    ce.path = XPath(xpi);
    ce.type = ChangeEvent::GROUP_CHANGE;
    data->dispatch(ce);
  }
}

std::string Node::get_localized_name() const
{
  if(this->data == nullptr)
    return "null";
  std::string res = this->type();
  if(langue.current_language.compare("fr") == 0)
  {
    if(has_attribute("fr") && (get_attribute_as_string("fr").size() > 0))
      return get_attribute_as_string("fr");
  }
  if(has_attribute("en") && (get_attribute_as_string("en").size() > 0))
    return get_attribute_as_string("en");
  if(has_attribute("name") && (name().size() > 0))
    return name();
  return res;
}

int Node::load(const string &schema_file, const string &data_file)
{
  FileSchema fs;
  if(fs.from_file(schema_file))
    return -1;
  Node n = Node::create_ram_node(fs.root, data_file);
  *this = n;
  return 0;
}

/// ??????
void Node::load(const string &filename)
{
  Node n = Node::create_ram_node(schema(), filename);
  copy_from(n);
}

int Node::save(const string &filename,
               bool store_default_values)
{
  log.trace("Saving to %s.", filename.c_str());

  string path, file;
  files::split_path_and_filename(filename, path, file);

  return files::save_txt_file(filename,
                                 std::string("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n")
                                 + to_xml(0, store_default_values, true, true, path));
}

Node::Node(const Node &e)
{
  data = e.data;
  if(data != nullptr)
    data->reference();
}

Node::Node(Node &e)
{
  data = e.data;
  if(data != nullptr)
    data->reference();
}

RamNode::~RamNode()
{
  uint32_t i, j;

  if(nb_references > 0)
  {
    log.anomaly("delete, but nref = %d.", nb_references);
  }

  for(i = 0; i < attributes.size(); i++)
    attributes[i].CProvider<ChangeEvent>::remove_listener(this);

  for(i = 0; i < children.size(); i++)
  {
    for(j = 0; j < children[i].nodes.size(); j++)
      children[i].nodes[j].data->remove_listener(this);
  }
}

void Node::setup_default_subs()
{
  if(schema() == nullptr)
  {
    log.anomaly("setup_default_subs: no schema.");
    return;
  }

  for(SubSchema &ss: schema()->children)
  {
    NodeSchema *es = ss.ptr;
    string subtype = es->name.get_id();
    while(get_children_count(es->name.get_id()) < (unsigned int) ss.default_count)
      add_child(es);

    for(Node child: children(subtype))
      child.setup_default_subs();
  }
}

void Node::check_min()
{
  if((data == nullptr) || (schema() == nullptr))
    return;

  for(SubSchema &ss: schema()->children)
  {
    NodeSchema *es = ss.ptr;

    if(es == nullptr)
    {
      log.anomaly("Schema ptr is nullptr: %s.", ss.child_str.c_str());
    }
    else if(ss.has_min())
    {
      while(get_children_count(es->name.get_id()) < (unsigned int) ss.min)
        add_child(es);
    }
  }
}

void Node::setup_refs()
{
  uint32_t i, j;

  // Search for references
  for(i = 0; i < schema()->references.size(); i++)
  {
    RefSchema rs = schema()->references[i];
    //trace("Search reference for %s...", rs.name.get_id().c_str());
    XPath path = get_reference_path(rs.name.get_id());
    //trace("Path = %s", path.to_string().c_str());
    //trace("Root path = %s", rs.path.c_str());

    XPath full_path(rs.path + "/" + path.to_string());
    //trace("Full path = %s", full_path.to_string().c_str());

    Node target = get_child(full_path);
    if(target.is_nullptr())
    {
      log.anomaly("Referring node not found.");
    }
    else
    {
      set_reference(rs.name.get_id(), target);
    }
  }
  for(i = 0; i < schema()->children.size(); i++)
  {
    string type = schema()->children[i].name.get_id();
    unsigned int n = get_children_count(type);
    for(j = 0; j < n; j++)
      get_child_at(type, j).setup_refs();
  }
}


XPath Node::get_reference_path(const string &name)
{
  if(data == nullptr)
  {
    log.anomaly("get_reference_path on nullptr node.");
    return XPath();
  }
  return data->get_reference_path(name);
}

XPath RamNode::get_reference_path(const string &name)
{
  for(unsigned int i = 0; i < references.size(); i++)
  {
    if(references[i].name.compare(name) == 0)
    {
      XPath res = references[i].path;

      if(res.to_string().size() == 0)
      {
	if(!references[i].ptr.is_nullptr())
	{
	  //warning("get_reference_path(): must build path from ptr.");
	}


        continue;
        /*log.anomaly("get_ref_path: nullptr path returned.");
        for(unsigned int j = 0; j < references.size(); j++)
        {
          trace("ref[%d] = %s", j, references[i].name.c_str());
          }*/
      }

      return res;
    }
  }
  log.warning("get_reference_path(%s): not found.", name.c_str());
  return XPath();
}



std::string Node::class_name() const
{
  if(data == nullptr)
    return "nullptr-node";
  std::string res = "node";
  //printf("class_name: type()...\n"); fflush(0);
  if(type().size() > 0)
    res += std::string("/") + type();

  return res;
}



void Node::setup_schema()
{
  unsigned int i;

  if(schema() == nullptr)
    return;

  for(i = 0; i < data->schema->references.size(); i++)
  {
    if(!this->has_reference(data->schema->references[i].name.get_id()))
    {
      this->set_reference(data->schema->references[i].name.get_id(), Node());
    }
  }
}

RamNode::RamNode(NodeSchema *schema)
{
  inhibit_event_raise = false;
  instance = 0;
  parent = nullptr;
  if(schema == nullptr)
  {
    log.anomaly("constructeur sans schema.");
  }
  /* To manage dereferencing of elt. */
  {
    nb_references = 1;
    this->schema = schema;

    for(const SubSchema &ss: schema->children)
    {
      NodeCol nc;
      nc.type = ss.name.get_id();
      children.push_back(nc);
    }

    for(const refptr<AttributeSchema> &as: schema->attributes)
    {
      Attribute att(as);
      attributes.push_back(att);
      attributes[attributes.size() - 1].parent = this;
      attributes[attributes.size() - 1].CProvider<ChangeEvent>::add_listener(this);
    }

    {
      Node elt(this);
      type = schema->name.get_id();
      elt.setup_schema();
      elt.check_min();
      elt.setup_default_subs();
    }
  }
  nb_references = 0;
}

RamNode::RamNode()
{
  inhibit_event_raise = false;
  instance            = 0;
  parent              = nullptr;
  {
    nb_references = 1;
    Node elt(this);
  }
  nb_references = 0;
}

void Node::fromXml(const MXml &e, string root_path)
{
  data->type = e.name;

  /* Special management of description which can
   * contain embedded HTML/XML content not to be analyzed here. */
  if(e.name.compare("description") == 0)
  {
    this->set_attribute("content", e.dump_content());
  }

  unsigned int n = e.attributes.size();
  for(unsigned int i = 0; i < n; i++)
  {
    const XmlAttribute &xa = e.attributes[i];

    // check references
    if(schema()->has_reference(xa.name))
    {
      set_reference(xa.name, XPath(xa.to_string()));
    }
    else
    {
      if(!schema()->has_attribute(xa.name))
      {
        log.warning("No such attribute: %s.", xa.name.c_str());
        continue;
      }


      std::string s = xa.string_value;
      char *tmp = (char *) malloc(s.size() + 1);

      unsigned int ko = 0;
      for(unsigned int k = 0; k < s.size(); k++)
      {
        if(s[k] != '\\')
        {
          tmp[ko++] = s[k];
        }
        else if((k + 1 < s.size()) && (s[k+1] == 'G'))
        {
          k++;
          tmp[ko++] = '"';
        }
        else if((k + 1 < s.size()) && (s[k+1] == '\\'))
        {
          k++;
          tmp[ko++] = '\\';
        }
        else
        {
          tmp[ko++] = s[k];
        }
      }
      tmp[ko] = 0;

      string value(tmp);
      free(tmp);

      refptr<AttributeSchema> as = schema()->get_attribute(xa.name);

      if((as->type == TYPE_FILE) && (root_path.size() > 0))
      {
        // Convert relative path to absolute path.
        string abs;
        if(files::rel2abs(root_path, value, abs))
        {
          log.warning("Relative path to absolute path conversion failed.");
        }
        else
        {
          /*trace("Converted rel path to abs: ref = [%s], rel = [%s], abs = [%s].",
                root_path.c_str(), value.c_str(), abs.c_str());*/
          value = abs;
        }
      }

      XPathItem xpi(xa.name);
      XPath xp(xpi);
      set_attribute(xp, value);
    }
  }

  NodeSchema *schem = schema();
  for(const SubSchema &ss: schem->children)
  {
    std::string sname =  ss.name.get_id();
    unsigned int n0 = get_children_count(sname);
    std::vector<const MXml *> lst;
    e.get_children(sname, lst);
    unsigned int n1 = lst.size();

    for(unsigned int k = 0; (k < n0) && (k < n1); k++)
      this->get_child_at(sname, k).fromXml(*lst[k], root_path);

    if(n1 > n0)
    {
      for(unsigned int k = 0; k < n1 - n0; k++)
      {
        Node nv = add_child(sname);
        nv.fromXml(*lst[k + n0], root_path);
      }
    }
    else if(n0 > n1)
    {

    }
  }
}

const Node Node::get_reference(const string &name) const
{
  unsigned int n = get_reference_count();
  for(unsigned int i = 0; i < n; i++)
  {
    std::string ref_name;
    Node elt;
    elt = get_reference_at(i, ref_name);
    if(ref_name.compare(name) == 0)
      return elt;
  }
  log.anomaly(std::string("reference not found: ") + name);
  log.trace("%d ref available:", n);
  for(unsigned int i = 0; i < n; i++)
  {
    std::string ref_name;
    Node elt;
    elt = get_reference_at(i, ref_name);
    log.trace("ref #%d = %s.", i, ref_name.c_str());
  }
  return Node();
}

unsigned int RamNode::get_reference_count() const
{
  return references.size();
}

bool Node::has_reference(const string &name) const
{
  unsigned int i, n = get_reference_count();
  for(i = 0; i < n; i++)
  {
    std::string ref_name;
    Node elt;
    elt = get_reference_at(i, ref_name);
    if(ref_name.compare(name) == 0)
      return true;
  }
  return false;
}

void RamNode::set_reference(const string &name, const XPath &xp)
{
  RefSchema *rs = schema->get_reference(name);
  if(rs == nullptr)
  {
    Node n(this);
    n.log.anomaly("set_ref: '%s' not found.", name.c_str());
    return;
  }

  unsigned int i, n = references.size();
  for(i = 0; i < n; i++)
  {
    if(references[i].name.compare(name) == 0)
    {
      references[i].path = xp;
      return;
    }
  }


  Reference rf;
  rf.name = name;
  rf.path = xp;
  references.push_back(rf);

  {
    Node tmp(this);
    tmp.log.trace("set_ref(%s = %s).", name.c_str(), xp.to_string().c_str());
  }
}

void RamNode::set_reference(const string &name, Node e)
{
  for(unsigned int i = 0; i < references.size(); i++)
  {
    if(references[i].name.compare(name) == 0)
    {
      references[i].ptr = e;
      return;
    }
  }
  if((!e.is_nullptr()) && (e.schema() != nullptr))
  {
    Reference rf;
    rf.name = e.schema()->name.get_id();
    rf.ptr  = e;
    references.push_back(rf);
    Node elt(this);
    elt.log.trace("Set ref ok (%s -> ...)", rf.name.c_str());
  }
  else
  {
    Reference rf;
    rf.name = name;
    rf.ptr  = e;
    references.push_back(rf);
  }
}

const Node RamNode::get_reference_at(unsigned int i, std::string &name) const
{
  name = references[i].name;
  return references[i].ptr;
}

std::string Node::to_xml_atts(unsigned int indent,
                              bool display_default_values,
                              bool charset_latin,
                              string root_path) const
{
  std::string res = "";
  std::vector<std::string > attnames, attvalues;
  unsigned int n = data->get_attribute_count();

  for(unsigned int i = 0; i < n; i++)
  {
    const Attribute *a = data->get_attribute_at(i);
    //if(a.name.compare("description") == 0)
    if((a->schema->name.get_id().compare("content") == 0) && (schema()->name.get_id().compare("description") == 0))
      continue;
    if(display_default_values
        || (a->value != a->schema->default_value)
        || (a->schema->type == TYPE_BLOB))
    {
      if(!a->schema->is_volatile)
      {
        attnames.push_back(a->schema->name.get_id());
        string val = a->schema->get_string(a->value);

        // If necessary, convert absolute path to relative path.
        if((a->schema->type == TYPE_FILE) && (root_path.size() > 0))
        {
          // Must get the relative path to the file from the root path.
          string rel;
          if(files::abs2rel(root_path, val, rel))
            log.warning("Abs to rel path conversion failed.");

          log.trace("Abs to relative: root = [%s], abs = [%s], rel = [%s].",
                root_path.c_str(), val.c_str(), rel.c_str());
          val = rel;
        }

        // TODO: must protect against following characters: <, >, "
        std::string s2;
        for(auto i = 0u; i < val.size(); i++)
        {
          if(val[i] == '"')
            s2 += "\\\"";
          else if(val[i] == '<')
            s2 += "\\<";
          if(val[i] == '>')
            s2 += "\\>";
          else
          {
            char tmp[2];
            tmp[0] = val[i];
            tmp[1] = 0;
            s2 += std::string(tmp);
          }
        }
        val = s2;

        if(charset_latin)
          val = utils::str::utf8_to_latin(val);

        attvalues.push_back(val);
      }
    }
  }
  n = get_reference_count();
  for(unsigned int i = 0; i < n; i++)
  {
    std::string ref_name;
    Node rs = get_reference_at(i, ref_name);
    if(rs.is_nullptr())
      log.warning("Cannot save nullptr reference.");
    else
    {
      XPath rel_path;
      std::string ref_path;
      RefSchema *rschema = schema()->get_reference(ref_name);
      std::string root_path = rschema->path;

      Node root = get_child(root_path);
      if(root.is_nullptr())
      {
	log.anomaly("unable to retrieve root path for ref. %s.", ref_name.c_str());
	continue;
      }

      if(root.get_path_to(rs, rel_path))
      {
	log.anomaly("to_xml(): unable to retrieve relative path for %s.", ref_name.c_str());
	continue;
      }

      ref_path = rel_path.to_string();

      log.trace("Save ref %s = %s.", ref_name.c_str(), ref_path.c_str());
      attnames.push_back(ref_name);
      attvalues.push_back(ref_path);//rs.name());
    }
  }
  unsigned int max_att_len = 0;
  for(unsigned int i = 0; i < attnames.size(); i++)
  {
    if(attnames[i].size() > max_att_len)
      max_att_len = attnames[i].size();
  }

  for(unsigned int i = 0; i < attnames.size(); i++)
  {
    res += attnames[i];
    for(unsigned int j = 0; j < max_att_len - attnames[i].size(); j++)
        res += " ";

    std::string s = attvalues[i];
    char *tmp = (char *) malloc(s.size() * 3 + 10);


    unsigned int ko = 0;
    for(unsigned int k = 0; k < s.size(); k++)
    {
      if(s[k] == '"')
      {
        tmp[ko++] = '\\';
        tmp[ko++] = 'G';
      }
      else if(s[k] == '\\')
      {
        tmp[ko++] = '\\';
        tmp[ko++] = '\\';
      }
      else
      {
        tmp[ko++] = s[k];
      }
    }
    tmp[ko] = 0;

    res += std::string(" = \"") + /*attvalues[i]*/std::string(tmp) + "\"";

    free(tmp);

    if(i < attnames.size() - 1)
    {
      res += "\n";
      for(unsigned int j = 0; j < indent + 2 + type().size(); j++)
        res += " ";
    }
  }

  return res;
}

std::string Node::text_resume(int indent) const
{
  if(schema() == nullptr)
  {
    log.anomaly("text_resume(): schema is nullptr.");
    return "";
  }
  std::string s = "";

  NodeSchema *sch = schema();

  for(uint32_t k = 0; k < (uint32_t) indent; k++)
    s += std::string(" ");
  char buf[500];
  sprintf(buf, "[%s]: ", sch->name.get_id().c_str());
  s += std::string(buf);
  for(uint32_t i = 0; i < sch->attributes.size(); i++)
  {
    sprintf(buf, "%s = %s%s", sch->attributes[i]->name.get_id().c_str(),
	    this->get_attribute_as_string(sch->attributes[i]->name.get_id().c_str()).c_str(),
                              (i + 1 < sch->attributes.size()) ? ", ": "");
    s += std::string(buf);
  }
  s += std::string("\n");

  for(uint32_t i = 0; i < sch->children.size(); i++)
  {
    NodeSchema *csch = sch->children[i].ptr;

    unsigned int n = this->get_children_count(csch->name.get_id());

    for(uint32_t k = 0; k < (uint32_t) indent; k++)
      s += std::string(" ");

    char buf[500];
    sprintf(buf, "%s: %d childs.\n", csch->name.get_id().c_str(), n);
    s += std::string(buf);

    if(n < 10)
    {
      for(uint32_t j = 0; j < n; j++)
      {
        s += this->get_child_at(csch->name.get_id(), j).text_resume(indent + 2);
      }
    }

  }

  return s;
}

std::string Node::to_xml(unsigned int indent,
                         bool display_default_values,
                         bool display_spaces,
                         bool charset_latin,
                         string root_path) const
{
  bool is_description = false;

  if(is_nullptr())
    return "";

  if((schema()->name.get_id().compare("description") == 0))
    is_description = true;

  if(this->is_nullptr())
  {
    return "(nullptr node)";
  }

  if(schema() == nullptr)
  {
    if(type().size() == 0)
    {
      log.anomaly("Cannot save node without schema nor type.");
      return "";
    }
  }
  std::string res = "";

  if(display_spaces && !is_description)
  {
    for(unsigned int i = 0; i < indent; i++)
      res += " ";
  }
  res += std::string("<") + type() + " ";
  res += to_xml_atts(indent, display_default_values, charset_latin, root_path);

  bool has_child = (get_children_count() > 0);

  if((schema()->name.get_id().compare("description") == 0)
      && (get_attribute_as_string("content").size() > 0))
    has_child = true;


  if(!has_child)
  {
    return res + "/>\n";
  }

  res += ">";
  if(!is_description)
    res += "\n";

  if(schema()->name.get_id().compare("description") == 0)
  {
    std::string content = get_attribute_as_string("content");
    if(content.size() > 0)
    {
      if(charset_latin)
        content = utils::str::utf8_to_latin(content);
      res += content;
    }
  }


  unsigned int n = schema()->children.size();
  for(unsigned int i = 0; i < n; i++)
  {
    const SubSchema &ss = schema()->children[i];
    string sname = ss.name.get_id();

    unsigned int m = get_children_count(sname);//child_str);

    for(unsigned int j = 0; j < m; j++)
      res += get_child_at(sname, j).to_xml(indent+2, display_default_values, display_spaces, charset_latin, root_path);
  }
  if(display_spaces && !is_description)
  {
    for(unsigned int i = 0; i < indent; i++)
      res += " ";
  }

  res = res + "</" + type() + ">";

  //if(!is_description)
    res += "\n";

  return res;
}

void Reference::set_reference(Node elt)
{
  ptr = elt;
}

Node Reference::get_reference()
{
  return ptr;
}

std::string Reference::get_name()
{
  return name;
}

static void accept_interval(std::vector<std::string> &cars, char c1, char c2)
{
  for(char c = c1; c <= c2; c++)
  {
    char b[2];
    b[0] = c;
    b[1] = 0;
    cars.push_back(std::string(b));
  }
}

static void accept_utf8(std::vector<std::string> &cars, char c1, char c2)
{
  char b[3];
  b[0] = c1;
  b[1] = c2;
  b[2] = 0;
  cars.push_back(std::string(b));
}

static void accept_all(std::vector<std::string> &cars)
{
  unsigned int i;

  accept_interval(cars, 'a', 'z');
  accept_interval(cars, 'A', 'Z');
  accept_interval(cars, '0', '9');
  accept_interval(cars, '.', '.');
  accept_interval(cars, '&', '&');
  accept_interval(cars, '-', '-');
  accept_interval(cars, '_', '_');
  accept_interval(cars, '(', '(');
  accept_interval(cars, ')', ')');

  for(i = 0x80; i <= 0xbf; i++)
    accept_utf8(cars, 0xc2, i);

  for(i = 0xc0; i <= 0xfe; i++)
    accept_utf8(cars, 0xc3, i);

  accept_interval(cars, '"', '"');
  accept_interval(cars, '\'', '\'');
  accept_interval(cars, '=', '=');
  accept_interval(cars, ' ', ' ');
}

void AttributeSchema::get_valid_chars(std::vector<std::string> &cars)
{
  unsigned int i;

  log.trace("get_valid_chars()..");

  cars.clear();
  if(constraints.size() > 0)
  {
    // combo or choice
  }
  else if((enumerations.size() > 0) && (has_max) && (max < 100))
  {
    // combo
  }
  else if((type == TYPE_STRING) && (is_ip))
  {
    cars.push_back(".");
    for(i = 0; i < 10; i++)
      cars.push_back(utils::str::int2str(i));
  }
# if 0
  else if((type == TYPE_STRING) && (regular_exp.size() > 0))
  {

    RegExp re;
    if(re.from_string(regular_exp))
    {
      log.anomaly("Failed to parse regexp: %s.", regular_exp.c_str());
      return;
    }
    re.get_valid_chars(cars);
  }
# endif
  else if(type == TYPE_STRING)
  {
    /* Accept all characters */
    accept_all(cars);
  }
  else if((type == TYPE_INT) && (is_hexa))
  {
    /* Accept int and 'x' symbol */
    accept_interval(cars, '0', '9');
    accept_interval(cars, 'x', 'x');
  }
  else if(type == TYPE_INT)
  {
    /* Accept int symbols */
    accept_interval(cars, '0', '9');
  }
  else
  {
    log.warning("get_vchars: unmanaged type %d.", type);
  }
  log.trace("done.");
}

bool AttributeSchema::is_valid(std::string s)
{
  const char *ss = s.c_str();
  uint32_t i, n = s.size();


  if((type == TYPE_STRING) && (is_ip))
  {
    //trace("check ip(%s)", ss);
    if((s.compare("test") == 0) || (s.compare("localhost") == 0))
    {

    }
    else
    {
      std::vector<int> ilist;
      if(utils::str::parse_int_list(s, ilist))
      {
        log.warning("set_value(\"%s\"): invalid ip.", ss);
        return false;
      }
      else
      {
        if(ilist.size() != 4)
        {
          log.warning("set_value(\"%s\"): invalid ip.", ss);
          return false;
        }
        else
        {
          for(uint32_t i = 0; i < 4; i++)
          {
            if((ilist[i] < 0) || (ilist[i] > 255))
            {
              log.warning("set_value(\"%s\"): invalid ip.", ss);
              return false;
            }
          }
        }
      }
    }
  }
  else if(type == TYPE_BLOB)
  {
    return true;
  }
  else if(type == TYPE_COLOR)
  {
    ByteArray ba(s);
    if(ba.size() != 3)
    {
      log.warning("Invalid color spec: %s.", s.c_str());
      return false;
    }
    return true;
  }
  else if(type == TYPE_INT)
  {
    if(s.size() == 0)
    {
      log.warning("set_value(\"\"): invalid value for int.");
      return false;
    }

    for(i = 0; i < enumerations.size(); i++)
    {
      if(s.compare(enumerations[i].name.get_id()) == 0)
        return true;
    }


    if(!is_bytes)
    {
      if(is_hexa)
      {
        if(s.compare("0") == 0)
          return true;
        if(ss[0] != '0')
          return false;
        if(ss[1] != 'x')
          return false;
        // Check int
        for(i = 2; i < n; i++)
        {
          if(!utils::str::is_hexa(ss[i]))
          {
            log.warning("set_value(\"%s\"): invalid character for hexa int.", ss);
            return false;
          }
        }
      }
      else
      {
        // Check int
        for(i = 0; i < n; i++)
        {
          if(!utils::str::is_deci(ss[i]) && (ss[i] != '-'))
          {
            log.warning("set_value(\"%s\"): invalid value for int.", ss);
            return false;
          }
        }

        if(has_min && (atoi(ss) < min))
        {
          log.warning("set_value(\"%s\"): < min = %ld.", ss, min);
          return false;
        }

        if(has_max && (atoi(ss) > max))
        {
          log.warning("set_value(\"%s\"): > max = %ld.", ss, max);
          return false;
        }
      }
    }
  }
  return true;
}

void Attribute::forward_change_event()
{
  /*ChangeEvent ce = ChangeEvent::create_att_changed(this);
  ce.source = this;
  ce.path = XPath(schema->name.get_id());
  dispatch(ce);*/
}

int  Attribute::set_value(const ByteArray &ba)
{
  if(ba != value)
  {
    if(!schema->is_valid(ba))
    {
      string s = ba.to_string();
      log.anomaly("%s(%s): Invalid value.", __func__, s.c_str());
      return -1;
    }

    value = ba;

    if(!inhibit_event_dispatch)
    {
      ChangeEvent ce = ChangeEvent::create_att_changed(this);
      ce.path = XPath(XPathItem(schema->name.get_id()));
      dispatch(ce);
    }

    return 0;
  }

  return 0;
}

int Attribute::set_value(const string &s)
{
  if(!schema->is_valid(s))
  {
    log.warning("set_value(%s): invalid value.", s.c_str());
    if(!inhibit_event_dispatch)
    {
      ChangeEvent ce = ChangeEvent::create_att_changed(this);
      ce.path = XPath(XPathItem(schema->name.get_id()));
      dispatch(ce);
    }
    return -1;
  }

  ByteArray ba;
  if(schema->serialize(ba, s))
    return -1;
  return set_value(ba);
}

string Attribute::get_string() const
{
  return schema->get_string(value);
}

void Attribute::set_value(int i)
{
  ByteArray ba;
  schema->serialize(ba, i);
  set_value(ba);
}

void Attribute::set_value(float f)
{
  ByteArray ba;
  schema->serialize(ba, f);
  set_value(ba);
}

void Attribute::set_value(bool b)
{
  ByteArray ba;
  schema->serialize(ba, b);
  set_value(ba);
}



// GET CHILDREN, TYPE NON PRECISE

unsigned long     Node::get_children_count() const
{
  uint32_t i, res = 0;

  if(data == nullptr)
    return 0;

  for(i = 0; i < schema()->children.size(); i++)
  {
    res += data->get_children_count(i);
  }
  return res;
}

// GET CHILDREN, TYPE PRECISE
unsigned long     Node::get_children_count(const string &type) const
{
  if(data == nullptr)
    return 0;
  return data->get_children_count(type);
}

Node           Node::get_child_at(const string &type, unsigned int i)
{
  //trace("get child %s %d", type.c_str(), i);
  if(data == nullptr)
    return Node();
  return data->get_child_at(type, i);
}
const Node     Node::get_child_at(const string &type, unsigned int i) const
{
  if(data == nullptr)
      return Node();
  //trace("get child %s %d", type.c_str(), i);
  return data->get_child_at(type, i);
}



Node           Node::add_child(Node nv)
{
  if(data == nullptr)
    return Node();

  Node res = data->add_child(nv.schema()->name.get_id());
  res.copy_from(nv);

  if(res.data != nullptr)
  {
    res.data->CProvider<ChangeEvent>::add_listener(data);
    ChangeEvent ce = ChangeEvent::create_child_added(nv.schema()->name.get_id(),
                                                     get_children_count(nv.schema()->name.get_id()) - 1);
    RamNode *rnode = (RamNode *) data;
    ce.path = ce.path.add_parent(XPathItem(schema()->name.get_id(), rnode->instance));
    //verbose("dispatch...");
    data->CProvider<ChangeEvent>::dispatch(ce);
    //verbose("dispatch done.");
  }

  return res;
}

Node           Node::add_child(NodeSchema *schema)
{
  return add_child(schema->name.get_id());
}

Node           Node::add_child(const string &sub_name)
{
  if(data == nullptr)
    return Node();

  //verbose("add_child(%s)...", sub_name.c_str());

  Node res = data->add_child(sub_name);

  if(res.data != nullptr)
  {
    res.data->CProvider<ChangeEvent>::add_listener(data);
    ChangeEvent ce = ChangeEvent::create_child_added(sub_name, get_children_count(sub_name) - 1);
    RamNode *rnode = (RamNode *) data;
    ce.path = ce.path.add_parent(XPathItem(this->schema()->name.get_id(), rnode->instance));
    data->CProvider<ChangeEvent>::dispatch(ce);
  }

  //verbose("done.");

  return res;
}

void              Node::remove_child(Node child)
{
  if(data == nullptr)
    return;

  if(child.data == nullptr)
    return;

  std::string sub_name = child.schema()->name.get_id();
  unsigned int instance = 0, n = get_children_count(sub_name);

  for(unsigned i = 0; i < n; i++)
  {
    if(child == get_child_at(sub_name, i))
    {
      instance = i;
      break;
    }
  }

  child.data->CProvider<ChangeEvent>::remove_listener(data);
  data->remove_child(child);

  ChangeEvent ce = ChangeEvent::create_child_removed(sub_name, instance);
  RamNode *rnode = (RamNode *) data;
  ce.path = ce.path.add_parent(XPathItem(this->schema()->name.get_id(), rnode->instance));
  data->CProvider<ChangeEvent>::dispatch(ce);
}

bool Node::is_attribute_valid(const string &name)
{
  if(!this->has_attribute(name))
  {
    log.warning("is_att_valid: att %s not found.", name.c_str());
    return false;
  }
  Attribute *att = this->get_attribute(name);
  std::string rq = att->schema->requirement;

  /* No requirement? */
  if(rq.size() == 0)
    return true;

  /* parse requirement:
   * must be in the form
   * "att-name=value1|value2|..." */

  size_t pos = rq.find("=");

  if(pos == std::string::npos)
  {
    log.warning("Invalid requirement: '%s'.", rq.c_str());
    return false;
  }

  std::string attname = rq.substr(0, pos);
  std::string tot     = rq.substr(pos + 1, rq.size() - pos - 1);

  std::vector<std::string> constraints;

  if(tot.size() > 0)
  {
    // Parse list of match ('|' separed)
    const char *s = tot.c_str();
    char current[200];
    int current_index = 0;
    for(unsigned int i = 0; i < strlen(s); i++)
    {
        if(s[i] != '|')
        {
            current[current_index++] = s[i];
        }
        else
        {
            current[current_index] = 0;
            constraints.push_back(std::string(current));
            current_index = 0;
        }
    }
    if(current_index > 0)
    {
        current[current_index] = 0;
        constraints.push_back(std::string(current));
    }
  }

  std::string s = std::string("Requirement: ") + attname + " = ";
  for(uint32_t i = 0; i < constraints.size(); i++)
  {
    s += constraints[i];
    if(i + 1 < constraints.size())
      s += " | ";
  }
  //trace(s);

  if(!has_attribute(attname))
  {
    log.warning("%s attribute not found (in requirement).", attname.c_str());
    return false;
  }

  std::string value = get_attribute_as_string(attname);

  for(uint32_t i = 0; i < constraints.size(); i++)
  {
    if(value.compare(constraints[i]) == 0)
    {
      log.trace("att %s is available.", name.c_str());
      return true;
    }
  }
  log.trace("att %s is not available.", name.c_str());
  return false;
}


// REFERENCES
unsigned int      Node::get_reference_count() const
{
  if(data != nullptr)
    return data->get_reference_count();
  else
    return 0;
}
void              Node::set_reference(const string &name, Node e)
{
  if(data != nullptr)
    data->set_reference(name, e);
}
void              Node::set_reference(const string &name, const XPath &xp)
{
  if(data != nullptr)
    data->set_reference(name, xp);
}

const Node     Node::get_reference_at(unsigned int i, std::string &name) const
{
  if(data != nullptr)
    return data->get_reference_at(i, name);
  else
    return 0;
}




Node::Node(NodePatron *data)
{
  this->data = data;
  if(data != nullptr)
    data->reference();
}



void NodePatron::reference()
{
  nb_references++;
}

void NodePatron::dereference()
{
  nb_references--;
}

std::string       NodePatron::class_name() const
{
  return "node-patron";
}

NodePatron::NodePatron()
{
  //this->ignore = false;
}

void NodePatron::lock()
{
  locked = true;
}


void NodePatron::unlock()
{
  locked = false;
}

Node::Node()
{
  data = nullptr;
}

bool Node::contains(const Node &elt)
{
  if(*this == elt)
    return true;

  unsigned int n = schema()->children.size();

  for(unsigned int i = 0; i < n; i++)
  {
    SubSchema &ss = schema()->children[i];
    unsigned int m = get_children_count(ss.child_str);

    for(unsigned int j = 0; j < m; j++)
      if(get_child_at(ss.child_str, j).contains(elt))
        return true;
  }

  return false;
}

void Node::lock()
{
  if(data != nullptr)
    data->lock();

  //for(uint32_t i = 0; i < get_children_count(); i++)
  //get_child_at(i).lock();
}

void Node::unlock()
{
  if(data != nullptr)
    data->unlock();
  //for(uint32_t i = 0; i < get_children_count(); i++)
  //get_child_at(i).unlock();
}

Node Node::create_ram_node(NodeSchema *schema)
{
  if(schema == nullptr)
    return Node();
  Node res(new RamNode(schema));
  return res;
}

Node Node::create_ram_node()
{
  Node res(new RamNode());
  return res;
}

Node Node::create_ram_node_from_string(NodeSchema *schema, std::string content)
{
  if(schema == nullptr)
    return Node();
  Node res(new RamNode(schema));

  MXml mx;
  if(mx.from_string(content))
  {
    res.log.anomaly("Parse error while parsing:\n%s", content.c_str());
    return res;
  }
  res.fromXml(mx);
  return res;
}

Node Node::create_ram_node(NodeSchema *schema, std::string filename)
{
  if(schema == nullptr)
    return Node();

  //schema->verbose("create empty rnode..");
  RamNode *rn = new RamNode(schema);

  Node res(rn);
  if(files::file_exists(filename))
  {
    MXml mx;

    schema->log.trace("Loading data tree from XML file [%s]...", filename.c_str());

    //schema->verbose("xml parsing..");
    if(mx.from_file(filename))
    {
      res.log.anomaly("Parse error while parsing %s.", filename.c_str());
      return res;
    }
    //schema->verbose("xml import..");

    string path, file;
    files::split_path_and_filename(filename, path, file);

    res.fromXml(mx, path);
    //schema->verbose("setup refs....");
    res.setup_refs();
    //schema->verbose("refs ok....");
  }
  else
    res.log.anomaly("create_ram_node: file not found (%s).", filename.c_str());
  return res;
}

bool Node::operator ==(const Node &e) const
{
  return (e.data == data);
}

bool Node::operator !=(const Node &e) const
{
  return (e.data != data);
}

void Node::operator =(const Node &e)
{
  NodePatron *old_data = data;
  data = e.data;
  if(data != nullptr)
    data->reference();

  if(old_data != nullptr)
  {
    old_data->dereference();
    if(old_data->nb_references <= 0)
    {
      old_data->discard();
      //log.trace("VRAI DELETE (op=).");
      delete old_data; // ?
    }
  }
}

Node::~Node()
{
  if(data == nullptr)
    return;
  //log.trace("destruction.");
  data->dereference();
  if(data->nb_references <= 0)
  {
    data->discard();
    data->CProvider<ChangeEvent>::remove_all_listeners();

    //log.trace("VRAI DELETE (des).");
    delete data;
  }
  data = nullptr;
}

bool Node::is_nullptr() const
{
  return (data == nullptr);
}

Node Node::parent() const
{
  if(data == nullptr)
    return Node();
  return data->get_parent();
}

Node RamNode::get_parent()
{
  Node elt(parent);
  return elt;
}

Node           Node::down(Node child)
{
  Node res;
  std::string cname = child.schema()->name.get_id();
  unsigned int i, n = get_children_count(cname);
  unsigned int index = n;

  std::vector<Node> children;
  for(i = 0; i < n; i++)
  {
    Node c = get_child_at(cname, i);
    if(child == c)
      index = i;
    children.push_back(c);
  }

  if(index == n)
  {
    log.anomaly("down(): child not found.");
    return child;
  }
  else if(index == n - 1)
  {
    log.anomaly("down(): index = n - 1.");
    return child;
  }

  for(i = 0; i < n; i++)
    remove_child(children[i]);

  for(i = 0; i < n; i++)
  {
    if(i == index)
    {
      add_child(children[index + 1]);
    }
    else if(i == index + 1)
    {
      res = add_child(children[index]);
    }
    else
      add_child(children[i]);
  }

  return res;
}

Node           Node::up(Node child)
{
  Node res;
  std::string cname = child.schema()->name.get_id();
  unsigned int i, n = get_children_count(cname);
  unsigned int index = n;

  std::vector<Node> children;
  for(i = 0; i < n; i++)
  {
    Node c = get_child_at(cname, i);
    if(child == c)
      index = i;
    children.push_back(c);
  }

  if(index == n)
  {
    log.anomaly("up(): child not found.");
    return child;
  }
  else if(index == 0)
  {
    log.anomaly("up(): index = 0.");
    return child;
  }

  for(i = 0; i < n; i++)
    remove_child(children[i]);

  for(i = 0; i < n; i++)
  {
    if((i + 1) == index)
    {
      res = add_child(children[index]);
    }
    else if(i == index)
    {
      add_child(children[i - 1]);
    }
    else
      add_child(children[i]);
  }

  return res;
}



void NodePatron::get_vector(std::string name, uint32_t index, void *data, uint32_t n)
{
  log.anomaly("TODO: generic get_vector.");
}

void NodePatron::add_vector(std::string name, void *data, uint32_t n)
{
  log.anomaly("TODO: add_vector.");
# if 0
  Node elt(this);
  /* Default implementation */
  for(uint32_t i = 0; i < n; i++)
  {
    Node nv = elt.add_child(name);
    nv.schema()
    nv.set_attribute()
  }
# endif
}

XPath::XPath()
{
  valid = true;
  //setup("model", "xpath");
}




XPath::XPath(const string &s)
{
  //setup("model", "xpath");
  this->valid = false;
  from_string(s);
}

int XPath::from_string(const string &s_)
{
  string s = s_;
  size_t pos;
  valid = false;
  items.clear();

  while(s.size() > 0)
  {
    /* get one item */
    uint32_t i = 0;
    while((s[i] != '/') && (s[i] != '[') && (i < s.size()))
    {
      i++;
    }
    XPathItem item;
    item.name     = s.substr(0, i);
    item.instance = -1;

    if(i == s.size())
    {
      items.push_back(item);
      break;
    }

    s = s.substr(i, s.size() - i);

    if(s[0] == '[')
    {
      s = s.substr(1, s.size() - 1);
      pos = s.find(']', 0);
      if(pos == std::string::npos)
      {
        //log.anomaly("xpath(\"%s\"): parse error.", s.c_str());
        return -1;
      }

      std::string spec = s.substr(0, pos);
      size_t pos_equal = spec.find('=', 0);

      if(pos_equal == std::string::npos)
      {
        item.instance = atoi(spec.c_str());
      }
      else
      {
        item.instance  = -1;
        item.att_name  = spec.substr(0, pos_equal);
        item.att_value = spec.substr(pos_equal + 1, spec.size() - (pos_equal + 1));
      }
      s = s.substr(pos + 1, s.size() - (pos + 1));
    }

    if(s.size() == 0)
    {
      items.push_back(item);
      break;
    }

    if(s[0] != '/')
    {
      //log.anomaly("xpath(\"%s\"): parse error.", s.c_str());
      return -1;
    }

    items.push_back(item);
    s = s.substr(1, s.size() - 1);
  }
  valid = true;
  return 0;
}

XPath::XPath(const XPath &xp)
{
  (*this) = xp;
}

void XPath::operator =(const XPath &xp)
{
  valid = xp.valid;
  items = xp.items;
}

const char *XPath::c_str() const
{
  XPath *th = (XPath *) this;
  th->full_string = to_string();
  return th->full_string.c_str();
}

std::string XPath::to_string() const
{
  std::string s = "";

  if(!is_valid())
    return "(invalid path)";

  if(items.size() == 0)
    return "";

  for(uint32_t i = 0; i < items.size(); i++)
  {
    s += items[i].name;
    if(items[i].att_name.size() > 0)
      s += std::string("[") + items[i].att_name + "=" + items[i].att_value  + "]";
    else if(items[i].instance > 0)
      s += std::string("[") + utils::str::int2str(items[i].instance) + "]";
    if(i + 1 < items.size())
      s += "/";
  }

  return s;
}

bool XPath::is_valid() const
{
  return valid;
}

XPath::~XPath()
{

}

XPathItem &XPath::operator[](const unsigned int i)
{
  if(i >= (unsigned int) length())
  {
    //anomaly("operator[%d]: overflow (%d nodes).", i, length());
    XPathItem *bidon = new XPathItem();
    return *bidon;
  }
  return items[i];
}

const XPathItem &XPath::operator[](const unsigned int i) const
{
  if(i >= (unsigned int) length())
  {
    //anomaly("operator[%d]: overflow (%d nodes).", i, length());
    XPathItem *bidon = new XPathItem();
    return *bidon;
  }
  return items[i];
}

XPathItem XPath::root() const
{
  if(!valid)
  {
    //anomaly("root() on invalid xpath.");
    XPathItem res;
    return res;
  }
  if(items.size() == 0)
  {
    //anomaly("root() on empty xpath.");
    XPathItem res;
    return res;
  }
  return items[0];
}

bool  XPath::has_child() const
{
  if(!valid)
    return false;
  return items.size() > 1;
}

XPath XPath::child() const
{
  XPath res;
  if(!valid)
    return res;
  res.valid = true;
  for(uint32_t i = 1; i < items.size(); i++)
    res.items.push_back(items[i]);
  return res;
}

XPath XPath::add_parent(XPathItem item) const
{
  XPath res;
  if(!valid)
    return res;
  res.valid = true;

  res.items.push_back(item);
  for(uint32_t i = 0; i < items.size(); i++)
      res.items.push_back(items[i]);

  return res;
}

bool XPath::operator ==(const XPath &xp) const
{
  if(items.size() != xp.items.size())
    return false;
  for(unsigned int i = 0; i < items.size(); i++)
  {
    if(items[i].name.compare(xp.items[i].name) != 0)
      return false;

    int i1 = items[i].instance, i2 = xp.items[i].instance;

    if(i1 == -1)
      i1 = 0;

    if(i2 == -1)
      i2 = 0;

    if(i1 != i2)
      return false;
    if(items[i].att_name.compare(xp.items[i].att_name))
      return false;
    if(items[i].att_value.compare(xp.items[i].att_value))
      return false;
  }
  return true;
}

int   XPath::length() const
{
  return items.size();
}

void XPath::clear()
{
  items.clear();
}

XPath XPath::remove_last() const
{
  XPath res;
  for(uint32_t i = 0; i + 1 < items.size(); i++)
  {
    res.items.push_back(items[i]);
  }
  return res;
}

XPath XPath::operator+(const XPath &xp) const
{
  unsigned int i = 0;
  XPath res(*this);
  for(i = 0; i < (unsigned int) xp.length(); i++)
  {
    res.add(xp[i]);
  }
  return res;
}

void XPath::add(const XPathItem &xpi)
{
  if((xpi.name.compare("") == 0)
      || (xpi.name.compare(".") == 0))
    return;
  if(xpi.name.compare("..") == 0)
    this->remove_last();
  else
  {
    items.push_back(xpi);
  }
}


XPath XPath::get_first() const
{
  XPath res;
  if(items.size() > 0)
    res.items.push_back(items[0]);
  return res;
}

std::string XPath::get_last() const
{
  if(items.size() == 0)
    return "";
  return items[items.size() - 1].name;
}

std::string ChangeEvent::to_string() const
{
  std::string s = "";
  switch(type)
  {
  case ATTRIBUTE_CHANGED: s += "ATTRIBUTE_CHANGED"; break;
  case CHILD_ADDED:       s += "CHILD_ADDED"; break;
  case CHILD_REMOVED:     s += "CHILD_REMOVED"; break;
  case COMMAND_EXECUTED:  s += "COMMAND_EXECUTED"; break;
  case GROUP_CHANGE:      s += "NODE_CHANGED"; break;
  }

  s += ", path = " + path.to_string();

  return s;
}

ChangeEvent::ChangeEvent()
{
  //source      = nullptr;
  //source_node = nullptr;
}

ChangeEvent ChangeEvent::create_att_changed(Attribute *source)
{
  ChangeEvent res;
  res.type        = ATTRIBUTE_CHANGED;
  res.path        = XPath(source->schema->name.get_id());
  return res;
}

ChangeEvent ChangeEvent::create_child_removed(std::string type, uint32_t instance)
{
  ChangeEvent res;

  res.type        = CHILD_REMOVED;
  res.path        = XPath(type + "[" + utils::str::int2str(instance) + "]");
  return res;
}

ChangeEvent ChangeEvent::create_child_added(std::string type, uint32_t instance)
{
  ChangeEvent res;

  res.type        = CHILD_ADDED;

  res.path        = XPath(type + "[" + utils::str::int2str(instance) + "]");
  return res;
}

ChangeEvent ChangeEvent::create_command_exec(Node *source, std::string name)
{
  ChangeEvent res;
  res.type = ChangeEvent::COMMAND_EXECUTED;

  int instance = ((RamNode *) source->data)->instance;

  res.path = XPath(source->schema()->name.get_id() + "[" + utils::str::int2str(instance) + "]/" + name);
  return res;
}

Enumeration::Enumeration()
{
  schema_str = "";
  schema = nullptr;
}

Enumeration::Enumeration(const Enumeration &e)
{
  *this = e;
}

void Enumeration::operator =(const Enumeration &e)
{
  name        = e.name;
  value       = e.value;
  schema      = e.schema;
  schema_str  = e.schema_str;
}

Enumeration::~Enumeration()
{
  schema_str = "";
}

DotTools::DotTools()
{
  log.setup("model");//, "dot-tools");
}

static std::string fcols[6] = {"FF8080", "80FF80", "FFFF30", "D0D0D0", "D0D0D0", "D0D0D0"};





string DotTools::get_name(const Node &e)
{
  std::string res = "";
  if(e.has_attribute("fr") && (e.get_attribute_as_string("fr").size() > 0))
    res = e.get_attribute_as_string("fr");
  else if(e.has_attribute("en") && (e.get_attribute_as_string("en").size() > 0))
    res = e.get_attribute_as_string("en");
  else
    res = e.name();

  if((res[0] >= 'a') && (res[0] <= 'z'))
    res[0] += 'A' - 'a';

  return res;
}

int DotTools::export_html_att_table(string &res, const Node &schema)
{
  std::string s = "";
  if(schema.get_children_count("attribute") > 0)
  {
    s   += std::string("<table border=\"1\" width=700 style='table-layout:fixed'>")
        + "<col width=150>"
          "<col width=100>"
          "<col width=125>"
          "<col width=95>"
          "<col width=300>"
        +  "<tr>"
        +  "<th><font size=\"-1\">" + utils::str::latin_to_utf8("Paramètre") + "</font></th>"
        +  "<th><font size=\"-1\">Identifiant</font></th>"
        +  "<th><font size=\"-1\">Type</font></th>"
        +  "<th><font size=\"-1\">Dimension</font></th>"
        +  "<th><font size=\"-1\">Description</font></th>"
        +  "</tr>";

    for(uint32_t i = 0; i < schema.get_children_count("attribute"); i++)
    {
      const Node &att = schema.get_child_at("attribute", i);
      s   += std::string("<tr><td>")
          + "<font size=\"-1\">"
          + get_name(att)
          + "</font>"
          + "</td>";

      s += "<th>";
      s += "<font size=\"-1\">";
      s += /*path + "/" +*/ att.name();
      s += "</font>";
      s += "</th>";

      s   += "<td><font size=\"-1\">" + get_attribute_type_description(att) + "</font></td>";

      s += "<td><font size=\"-1\">";

      if(att.get_attribute_as_string("type").compare("float") == 0)
      {
        s += "32 bits";
      }
      else
      {
        if(att.get_attribute_as_int("size") == 1)
          s += "1 octet";
        else
          s +=  att.get_attribute_as_string("size") + " octets";
      }
      s += "</font></td>";

      s   += "<td><font size=\"-1\">" + get_attribute_long_description(att) + "</font></td>"
          + "</tr>";
    }
    s += std::string("</table>");
  }
  res = s;
  return 0;
}

string LatexWrapper::get_attribute_long_description(const Node &e)
{
  string res = "";

  //res += std::string("<p>") + e.description() + "</p>";

  if(e.has_child("match"))
  {
    res += "Valeurs possibles :";
    res += "\\begin{description}\n";
    for(uint32_t i = 0; i < e.get_children_count("match"); i++)
    {
      const Node &ch = e.get_child_at("match", i);
      res += "\\item[" + ch.get_attribute_as_string("value") + "] " + get_name(ch);
      if(ch.description().size() > 0)
        res += "\n" + ch.description();
      res += "\n";
    }

    res += "\\end{description}\n";
  }

  if(e.has_attribute("unit") && (e.get_attribute_as_string("unit").size() > 0))
  {
    res += "S'exprime en {\\bf " + e.get_attribute_as_string("unit") + "}.\n\n";
  }

  std::string def = e.get_attribute_as_string("default");

  AttributeSchema as(e);

  std::string def2 = as.get_ihm_value(as.get_default_value());

  if(as.type == TYPE_STRING)
    def2 = "\"" + def2 + "\"";

  //Valeur par dÃ©faut
  res += langue.get_item("default-val") +  " : " + def2;

  if(e.has_attribute("unit") && (e.get_attribute_as_string("unit").size() > 0))
    res += " " + e.get_attribute_as_string("unit");

  res += "\n\n";


  if(!e.has_child("match"))
  {
    if(e.get_attribute_as_string("min").size() > 0)
    {
      res += "Valeur minimale : "
          + e.get_attribute_as_string("min")
          + " " + e.get_attribute_as_string("unit") + "\n\n";
    }
    if(e.get_attribute_as_string("max").size() > 0)
    {
      res += "Valeur maximale : "
          + e.get_attribute_as_string("max")
          + " " + e.get_attribute_as_string("unit")
          + "\n\n";
    }
  }

  res += e.description();

  return res;
}

string DotTools::get_attribute_type_description(const Node &e)
{
  std::string res = "";

  std::string tp = e.get_attribute_as_string("type");

  if(tp.compare("int") == 0)
  {
    res += "Entier";
    if(e.get_attribute_as_boolean("signed"))
      res += " " + langue.get_item("signed");
    else
      res += " non " + langue.get_item("signed");
  }
  else if(tp.compare("boolean") == 0)
    res += langue.get_item("boolean");
  else if(tp.compare("string") == 0)
    res += langue.get_item("string");
  else if(tp.compare("float") == 0)
    res += "Flottant";
  else
  {
    res += "Type inconnu: " + tp;
  }

  /*if(e.get_attribute_as_int("count") > 1)
  {
    res += " (tableau de " + e.get_attribute_as_string("count") + " ï¿½lï¿½ments)";
  }*/

  if(e.get_attribute_as_boolean("readonly"))
    res += "<br/><b>lecture seule</b>";

  /*if(e.get_attribute_as_string("type").compare("int") == 0)
  {
    res += std::string(" (") + e.get_attribute_as_string("size") + ")";
  }*/

  return res;
}

void DotTools::build_graphic(Node &schema, const std::string &output_filename)
{
    std::string fn =
      utils::get_current_user_path() + PATH_SEP +
      std::string("tmp-") + schema.name()
      + ".dot";

    log.trace("Building graphics from model =\n%s\n", schema.to_xml(0).c_str());

    FILE *tmp = fopen(fn.c_str(), "wt");
    if(tmp == nullptr)
    {
      log.anomaly("Failed to create [%s].", fn.c_str());
      return;
    }

    fprintf(tmp, "digraph G {\nratio=compress;\noverlap=false;\n");
    fprintf(tmp, "root=%s;\n", str::str_to_var(schema.name()).c_str());
    fprintf(tmp, "%s", complete_dot_graph(schema, 0).c_str());
    fprintf(tmp, "fontsize=5;\n");
    fprintf(tmp, "}");
    fclose(tmp);

    std::string img_fmt = "png";

    log.trace("calling twopi..");

    std::string dotpath = "dot";
    #ifdef WIN
    dotpath = "\"C:\\Program Files\\graphviz 2.28\\bin\\dot.exe\"";
    log.trace_major("Win32 dot: %s.", dotpath.c_str());
    #endif



    utils::proceed_syscmde("%s -T" + img_fmt
                              + " \"" + fn + "\" -o \"%s\"",
                          dotpath.c_str(),
                          output_filename.c_str());
}

std::string DotTools::complete_dot_graph(Node section, int level)
{
  std::string res = "";

  std::string fillcolor = fcols[level];


  //section.log.trace("complete_dot_graph(level %d)..", level);
  // margin=0 : marge horizontale
  //res += "node [shape=box, fontsize=9, fillcolor=\"#" + fillcolor + "\", style=filled];\n";
  //res += "node [shape=none, fontsize=9];\n";


  /*res += utils::str::str_to_var(section.get_attribute_as_string("name"))
	  +  " [label=\""
	  + section.get_attribute_as_string("name")
	  + "\"];\n";*/

  /** Construction de la classe */

  std::string name = section.name();

  res += str::str_to_var(name)
	  +  " [fontsize=9, shape=none, margin=0, label=<";

  res += "<TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"3\"";
  res += " BGCOLOR=\"#" + fillcolor + "\"";
  res += ">\n";

  res += "<TR><TD BORDER=\"1\">";

  res += section.get_localized_name();

  res += "</TD></TR>";

  for(unsigned int i = 0; i < section.get_children_count("attribute"); i++)
  {
	Node att = section.get_child_at("attribute", i);

	res += "<TR><TD>";

	res += att.get_localized_name();

	res += "</TD></TR>";
  }

  res += "</TABLE>\n";



  res += ">";

  std::string url = "";
  if(level == 1)
  {
	url = "#" + section.name();
  }

  if(url.size() > 0)
	res += ", URL=\"" + url + "\"";

  res += "];\n";


  fillcolor = fcols[level + 1];





  res += "node [shape=box, fontsize=9, fillcolor=\"#" + fillcolor + "\", "
		 "style=filled";

  //if(level == 0)
	//res += ", root=true"

  res += "];\n";


  for(unsigned int i = 0; i < section.get_children_count("sub-node"); i++)
  {
	Node sub = section.get_child_at("sub-node", i);

	// EDGE attributes

	int weight = 10 * (level + 1);

	//res += "edge [dir=\"back\", len=0.2, weight="
	res += "edge [dir=\"back\", weight="
		+   utils::str::int2str(weight) + ", arrowtail=\"diamond\", fontsize=7";
	int min = sub.get_attribute_as_int("min");
	int max = sub.get_attribute_as_int("max");
	if(min == -1)
	  min = 0;

	if((min != 1) || (max != 1))
	{
	  std::string smin = utils::str::int2str(min);
	  std::string smax = utils::str::int2str(max);
	  if(max == -1)
		smax = "n";
	  if(max == min)
		res += ", label=\"" + smin + "\"";
	  else
		res += ", label=\"" + smin + ".." + smax + "\"";
	}


	res += "];\n";

	res += str::str_to_var(section.name())
		+  " -> "
		+  str::str_to_var(sub.name())
		+ ";\n";
  }

  for(unsigned int i = 0; i < section.get_children_count("sub-node"); i++)
  {
	Node sub = section.get_child_at("sub-node", i);
	res += complete_dot_graph(sub, level + 1);
  }

  //section.log.trace("complete_dot_graph(level %d): done.", level);

  return res;
}

RefSchema::RefSchema()
{
  is_hidden = false;
}

const Node NodeIterator::operator*() const
{
  return parent->get_child_at(type, index);
  //Node n(parent);
  //return n.get_child_at(type, index);
}

Node NodeIterator::operator++()
{
  index++;
  return parent->get_child_at(type, index - 1);
  //Node n(parent);
  //return n.get_child_at(type, index - 1);
}

const Node ConstNodeIterator::operator*() const
{
  //const Node n((NodePatron *) parent);
  //printf("op*(%d)\n", index);
  //return n.get_child_at(type, index);
  return parent->get_child_at(type, index);
}

const Node ConstNodeIterator::operator++()
{
  //printf("op++(%d->%d)\n", index, index+1);
  //index++;
  //Node n((NodePatron *) parent);
  //return n.get_child_at(type, index - 1);
  index++;
  return parent->get_child_at(type, index - 1);
}

NodeIterator NodeList::end() const
{
  return NodeIterator(parent, type, parent->get_children_count(type));
}

ConstNodeIterator ConstNodeList::end() const
{
  return ConstNodeIterator(parent, type, parent->get_children_count(type));
}

std::string Node::format_c_comment(int indent)
{
  Node e = (*this);
  std::string s = "", desc = utils::str::utf8_to_latin(e.description());
  uint32_t i, j, k;

  for(i = 0; i < (uint32_t) indent; i++)
    s += " ";

  s += "/** @brief ";
  //s += prefix;
  s += utils::str::utf8_to_latin(e.get_localized_name());

  /* Clean description. */
  std::string d2;
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

  if(only_spaces && (e.get_localized_name().size() == 0))
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

  uint32_t nmatch = e.get_children_count("match");

  if(nmatch > 0)
  {
    TextMatrix mmatrix(2);
    s += "\n";
    for(j = 0; j < nmatch; j++)
    {
      Node match = e.get_child_at("match", j);
      std::string idt = "";

      for(i = 0; i < (uint32_t) indent + 4 + 7; i++)
        idt += " ";

      mmatrix.add(idt + match.get_attribute_as_string("value") + ": ");


      std::string dsc = utils::str::utf8_to_latin(match.get_localized_name());

      mmatrix.add(dsc);
      mmatrix.next_line();
    }
    s += mmatrix.get_result();
    s = s.substr(0, s.size() - 1);
  }

  s += " */\n";

  return s;
}

NodeList Node::children(const string &type)
{
  return NodeList(data, schema()->mapper[type]);
}

ConstNodeList Node::children(const string &type) const
{
  return ConstNodeList(data, schema()->mapper[type]);
}

string Node::to_html(unsigned int level) const
{
  unsigned int i, j, n;
  ostringstream res;
  NodeSchema *scheme = this->schema();

  //res << "<h" << level << ">" << scheme->name.get_localized() << "</h" << level << ">\n";

  res << "<div align=\"left\"><table>";
  for(i = 0; i < scheme->attributes.size(); i++)
  {
    refptr<AttributeSchema> as = scheme->attributes[i];
    Localized attname = scheme->attributes[i]->name;
    string val = as->get_ihm_value(get_attribute_as_string(attname.get_id()));
    string loc = attname.get_localized();
    res << "<tr><td>" << loc << "</td>";

    res << "<td>" << val;

    if(as->has_unit())
      res << " " << as->unit;

    res << "</td></tr>";
  }
  res << "</table></div>";

  for(i = 0; i < scheme->children.size(); i++)
  {
    string name = scheme->children[i].name.get_id();
    n = this->get_children_count(scheme->children[i].name.get_id());
    for(j = 0; j < n; j++)
    {

      res << "<h" << level << ">" << scheme->children[i].ptr->name.get_localized() << "</h" << level << ">\n";
      res << get_child_at(name, j).to_html(level + 1);
    }
  }
  return res.str();
}


Logable LatexWrapper::log("libcutil");

int LatexWrapper::export_att_table(string &res, const Node &schema)
{
  std::string s = "";
  if(schema.get_children_count("attribute") > 0)
  {
    s   += std::string("\\begin{longtable}{|c|c|c|c|p{7cm}|}\n")
        +  "\\hline\n"
        +  "{\\bf " + utils::str::latin_to_utf8("Paramètre") + "} & "
        +  "{\\bf Identifiant} &"
        +  "{\\bf Type} &"
        +  "{\\bf Dimension} &"
        +  "{\\bf Description}\\\\\n\\hline\n";

    for(uint32_t i = 0; i < schema.get_children_count("attribute"); i++)
    {
      const Node &att = schema.get_child_at("attribute", i);

      s += get_name(att) + " & ";

      s += att.name() + " & ";

      s += get_attribute_type_description(att) + " & ";

      if(att.get_attribute_as_string("type").compare("float") == 0)
      {
        s += "32 bits";
      }
      else
      {
        if(att.get_attribute_as_int("size") == 1)
          s += "1 octet";
        else
          s +=  att.get_attribute_as_string("size") + " octets";
      }
      s += " & ";


      s += "\\begin{minipage}[c]{7cm}\n";
      s += get_attribute_long_description(att);
      s += "\\end{minipage}\\\\\n";
      s += "\\hline\n";
    }
    s += std::string("\\end{longtable}\n");
  }
  res = s;
  return 0;
}


string LatexWrapper::get_name(const Node &e)
{
  std::string res = "";
  if(e.has_attribute("fr") && (e.get_attribute_as_string("fr").size() > 0))
    res = e.get_attribute_as_string("fr");
  else if(e.has_attribute("en") && (e.get_attribute_as_string("en").size() > 0))
    res = e.get_attribute_as_string("en");
  else
    res = e.name();

  if((res[0] >= 'a') && (res[0] <= 'z'))
    res[0] += 'A' - 'a';
  return res;
}

string LatexWrapper::get_attribute_type_description(const Node &e)
{
  std::string res = "";

  std::string tp = e.get_attribute_as_string("type");

  if(tp.compare("int") == 0)
  {
    res += "Entier";
    if(e.get_attribute_as_boolean("signed"))
      res += " " + langue.get_item("signed");
    else
      res += " non " + langue.get_item("signed");
  }
  else if(tp.compare("boolean") == 0)
    res += langue.get_item("boolean");
  else if(tp.compare("string") == 0)
    res += langue.get_item("string");
  else if(tp.compare("float") == 0)
    res += "Flottant";
  else
  {
    res += "Type inconnu: " + tp;
  }

  /*if(e.get_attribute_as_int("count") > 1)
  {
    res += " (tableau de " + e.get_attribute_as_string("count") + " ï¿½lï¿½ments)";
  }*/

  if(e.get_attribute_as_boolean("readonly"))
    res += " {\\bf lecture seule}";

  /*if(e.get_attribute_as_string("type").compare("int") == 0)
  {
    res += std::string(" (") + e.get_attribute_as_string("size") + ")";
  }*/

  return res;
}

string DotTools::get_attribute_long_description(const Node &e)
{
  string res = "";

  //res += std::string("<p>") + e.description() + "</p>";

  if(e.has_child("match"))
  {
    res += "<p>Valeurs possibles :<ul>";
    for(uint32_t i = 0; i < e.get_children_count("match"); i++)
    {
      const Node &ch = e.get_child_at("match", i);
      res += "<li><b>" + ch.get_attribute_as_string("value") + " : " + get_name(ch) + "</b>";
      if(ch.description().size() > 0)
        res += "<br/>" + ch.description();
      res += "</li>";
    }

    res += "</ul></p>";
  }

  if(e.has_attribute("unit") && (e.get_attribute_as_string("unit").size() > 0))
  {
    res += "S'exprime en <b>" + e.get_attribute_as_string("unit") + "</b>.";
  }

  std::string def = e.get_attribute_as_string("default");

  AttributeSchema as(e);

  std::string def2 = as.get_ihm_value(as.get_default_value());

  if(as.type == TYPE_STRING)
    def2 = "\"" + def2 + "\"";

  //Valeur par dÃ©faut
  res += "<p> " + langue.get_item("default-val") +  " : " + def2;

  if(e.has_attribute("unit") && (e.get_attribute_as_string("unit").size() > 0))
    res += " " + e.get_attribute_as_string("unit");

  res +=  "</p>";

  if(!e.has_child("match"))
  {
    if(e.get_attribute_as_string("min").size() > 0)
    {
      res += "<p>Valeur minimale : "
          + e.get_attribute_as_string("min")
          + " " + e.get_attribute_as_string("unit")
          + "</p>";
    }
    if(e.get_attribute_as_string("max").size() > 0)
    {
      res += "<p>Valeur maximale : "
          + e.get_attribute_as_string("max")
          + " " + e.get_attribute_as_string("unit")
          + "</p>";
    }
  }

  res += "<p>" + e.description() + "</p>";


  return res;
}


}
}
