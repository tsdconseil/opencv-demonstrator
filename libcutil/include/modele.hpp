/** @file   modele.hpp
 *  @brief  Gestion d'un mod�le de donn�es arborescent
 *          avec repr�sentation RAM, XML, ou ROM compress�.
 *
 *  This file is part of LIBCUTIL.
 *
 *  LIBCUTIL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBCUTIL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBCUTIL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J.A. **/


#ifndef MODELE_HPP
#define MODELE_HPP

#include "mxml.hpp"
#include "slots.hpp"
#include "trace.hpp"
#include "bytearray.hpp"
#include "cutil.hpp"

#include <vector>
#include <string>
#include <map>
#include <stdint.h>


namespace utils
{

/** @brief Generic data representation */
namespace model
{

using namespace std;
using namespace utils;

class FileSchema;
class NodeSchema;
class Attribute;
class Node;
class NodePatron;
class RamNode;



/** @brief Item of a path */
class XPathItem
{
public:
  XPathItem();
  XPathItem(const string &name, int instance = -1);
  XPathItem(const string &name, 
            const string &att_name, 
            const string &att_value);
  virtual ~XPathItem();

  /** Node type */
  string name;

  /** Optionnal instance number (-1 if unspecified) */
  int         instance;

  /** Optionnal attribute value ("" if unspecified) */
  string att_name, att_value;
};


/** @brief An XPath defines a path from a root Node to a sub node or attribute
 *  XPath can be built from a string with the following format:
 *  - xpath to an attribute: "subname/subname/attname"
 *  - xpath to a sub-node: "subname"
 *
 *  Supported tokens:
 *    ".." -> parent node
 *    "."  -> current node
 *    ""   -> root node
 *
 *  */
class XPath
{
public:
  XPath();
  XPath(const XPathItem &xpi);
  XPath(const string &s);
  XPath(const char *s);
  XPath(const XPath &xp);
  XPath(const XPath &root, const string &leaf, int instance = 0);
  virtual ~XPath();
  void clear();
  int from_string(const string &s);
  void operator =(const XPath &xp);
  bool operator ==(const XPath &xp) const;
  XPath operator+(const XPath &xp) const;
  string to_string() const;
  const char *c_str() const;
  bool is_valid()         const;
  XPathItem root()        const;
  bool  has_child()       const;
  int   length()          const;
  XPath child()           const;
  XPath add_parent(XPathItem item) const;
  XPathItem &operator[](const unsigned int i);
  const XPathItem &operator[](const unsigned int i) const;
  XPath remove_last() const;
  void add(const XPathItem &xpi);

  string get_last() const;
  XPath get_first() const;

  static XPath null;

  operator string const () const {return to_string();}
private:
  string full_string;
  bool valid;
  vector<XPathItem> items;
};

/** @brief A change occurred in the source node.
 *  May be:
 *   - An attribute change
 *   - A node added or removed */
class ChangeEvent
{
public:
  typedef enum
  {
    /** @brief Individual attribute change */
    ATTRIBUTE_CHANGED = 0,

    /** @brief New child added */
    CHILD_ADDED       = 1,

    /** @brief Child removed */
    CHILD_REMOVED     = 2,

    /** @brief Command executed --> deprecated, to remove */
    COMMAND_EXECUTED  = 3,

    /** @brief A group of changes (one change or more) has occurred simultaneously
     *  inside the node. */
    GROUP_CHANGE      = 4
  } type_t;

  type_t type;

  /** Path to the element at the origin of the event */
  XPath  path;

  string to_string() const;

  ChangeEvent();


  // DEPRECATED : to remove
  static ChangeEvent create_att_changed(Attribute *source);
  // DEPRECATED : to remove
  static ChangeEvent create_child_removed(string type, uint32_t instance);
  // DEPRECATED : to remove
  static ChangeEvent create_child_added(string type, uint32_t instance);
  // DEPRECATED : to remove
  static ChangeEvent create_command_exec(Node *source, string name);
}; 


/** @brief List of the different attribute types */
typedef enum attribute_type_enum
{
  /** @brief Integer */
  TYPE_INT     = 0,
  /** @brief String of characters */
  TYPE_STRING  = 1,
  /** @brief Boolean */
  TYPE_BOOLEAN = 2,
  /** @brief Floating point (single or double precision) */
  TYPE_FLOAT   = 3,
  /** @brief 24 bits RGB color */
  TYPE_COLOR   = 4,
  /** @brief Date / time */
  TYPE_DATE    = 5,
  /** @brief Folder path */
  TYPE_FOLDER  = 6,
  /** @brief File path */
  TYPE_FILE    = 7,
  /** @brief Serial port selection */
  TYPE_SERIAL  = 8,
  /** @brief Large array of unformatted data */
  TYPE_BLOB    = 9
} attribute_type_t;


/** @brief Item of an enumeration */
class Enumeration
{
public:
  Enumeration();
  Enumeration(const Enumeration &e);
  void operator =(const Enumeration &e);
  ~Enumeration();

  /** Name to display for this value */
  Localized name;

  /** Value */
  string value;

  /** Optionnal schema (can be nullptr) */
  NodeSchema *schema;

  /** Optionnal schema name */
  string schema_str;
};



/** @brief Schema of an attribute */
class AttributeSchema
{
public:
  friend class NodeSchema;

  AttributeSchema();
  AttributeSchema(const Node &e);
  AttributeSchema(const MXml &mx);
  AttributeSchema(const AttributeSchema &c);
  ~AttributeSchema();
  void operator =(const AttributeSchema &c);
  string type2string() const;
  string to_string() const;
  string get_default_value() const;
  string get_ihm_value(string val) const;
  void serialize(ByteArray &ba) const;
  int  unserialize(ByteArray &ba);
  bool is_valid(string s);
  bool is_valid(const ByteArray &ba) const;
  void get_valid_chars(vector<string> &cars);

  void make_default_default_value(ByteArray &res) const;

  /** Name, translations and descriptions of this attribute */
  Localized name;

  /** Attribute type */
  attribute_type_t type;

  /** Size, in bytes, for int and float (TYPE_INT or TYPE_FLOAT) */
  int size;

  /** For integers only */
  bool is_signed;

  /** For integers only */
  bool is_bytes;

  /** For integers only */
  bool is_hexa;

  /** Is it an IP address (for strings only) */
  bool is_ip;

  /** Is it some formatted long text (for strings only) */
  bool formatted_text;

  /** Optionnal unit specification, e.g. Hz, second, etc. */
  string unit;

  /** Possible extensions, for TYPE_FILE only */
  string extension;

  /** This specify the condition of validity of the attribute */
  string requirement;

  ///////////////////////////////////////////////
  /// DEFAULT MMI BEHAVIOUR
  ///////////////////////////////////////////////
  /** Is the attribute hidden? */
  bool is_hidden;

  /** Is this attribute writable? */
  bool is_read_only;

  /** Is this attribute a measure? */
  bool is_instrument;


  /** Must this attribute be saved? */
  bool is_volatile;

  bool has_unit() const {return (unit.size() > 0);}
  long int min;
  long int max;
  bool has_min, has_max;
  long int get_min();
  long int get_max();
  bool has_constraints() const;

  string regular_exp;

  vector<string> constraints;
  vector<Enumeration> enumerations;
  
  ByteArray default_value;
  
  /** ? */
  bool is_unique;

  unsigned short id;
  bool has_description() const {return (name.has_description());}
  bool has_description_fr() const {return (name.has_description());}
  bool fixed_size() const;



  int    get_int    (const ByteArray &ba) const;
  bool   get_boolean(const ByteArray &ba) const;
  float  get_float  (const ByteArray &ba) const;
  string get_string (const ByteArray &ba) const;

  int    serialize(ByteArray &ba, int value)    const;
  int    serialize(ByteArray &ba, bool value)   const;
  int    serialize(ByteArray &ba, float value)  const;
  int    serialize(ByteArray &ba, string value) const;

private:
  static Logable log;
};

/** @brief Schema for a container of a list of nodes */
class SubSchema
{
public:
  SubSchema();
  void operator =(const SubSchema &ss);
  string to_string() const;

  /** Minimum and maximum number of nodes (-1 = unspecified) */
  int min, max;

  /** Name, translations and descriptions */
  Localized name;

  /** Pointer to the schema of the children nodes. */
  NodeSchema *ptr;

  /** Name of the schema of the children node */
  string child_str;

  int default_count;

  /** Default key for indexing */
  string default_key;

  ///////////////////////////////////////////////
  /// DEFAULT MMI BEHAVIOUR
  ///////////////////////////////////////////////
  /** Display as tabular ? */
  bool display_tab;

  /** Display as tree */
  bool display_tree;

  /** Unfold by default in tree view ? */
  bool display_unfold;

  bool is_hidden;

  /** Is managed by a choice */
  bool is_exclusive;

  /** The user cannot add / remove items from this table */
  bool readonly;

  bool show_header;

  /** List of attributes to display into the table (if display_tab) */
  vector<string> resume;
  bool has_max() const {return (max > 0);}
  bool has_min() const {return (min > 0);}
};

/** @brief Schema of the reference to another node */
class RefSchema
{
public:
  RefSchema();
  Localized   name;
  NodeSchema *ptr;
  string path;
  string child_str;
  bool is_hidden;
};



/** @brief Schema describing a command */
class CommandSchema
{
public:
  CommandSchema(const MXml &mx);
  CommandSchema(const Node &model);
  CommandSchema(const CommandSchema &cs);
  void operator =(const CommandSchema &cs);

  Localized name;

  /** @brief Input parameters */
  refptr<NodeSchema> input;

  /** @brief Output parameters */
  refptr<NodeSchema> output;

private:
};

/** @brief Schema describing a node */
class NodeSchema
{
public:
  friend class RamNode;
  friend class FileSchema;
  NodeSchema(){}
  NodeSchema(const Node &elmt, FileSchema *root = nullptr, const string &name = "");
  NodeSchema(const MXml &mx);
  NodeSchema(const NodeSchema &c);
  ~NodeSchema();

  void operator =(const NodeSchema &c);
  string to_string();
  bool has_description() const {return name.has_description();}
#ifndef TESTCLI  
  bool has_key_attribute() const;
  AttributeSchema *get_key_attribute();
#endif  
  void do_inherits();
  bool has_editable_props();
  
  bool has_reference(string name) const;
  RefSchema *get_reference(string name);
  bool has_icon() const {return (icon_path.size() > 0);}

  void update_size_info();
  bool has_attribute(string name) const;
  refptr<AttributeSchema> get_attribute(string name);
  bool has_child(string name) const;

  string get_localized() const;

  NodeSchema *get_sub(string name);
  void serialize(ByteArray &ba);
  int  unserialize(ByteArray &ba);

  CommandSchema *get_command(string name);

  /** @brief Import specifications from a XML node */
  void from_xml(const MXml &mx);

  SubSchema *get_child(string name);


  /** Return the index of the child */
  int get_sub_index(const string &name) const;

  void add_attribute(refptr<AttributeSchema> schema);
  void add_sub_node(const SubSchema &schema);

  /* @returns true if nothing is configurable in this schema */
  bool is_empty() const;

  bool fixed_size;
  bool attributes_fixed_size;
  bool children_fixed_size;

  /** Sub-node specifications */
  deque<SubSchema>       children;

  /** Attributes list */
  deque<refptr<AttributeSchema> > attributes;

  deque<CommandSchema> commands;

  deque<RefSchema>       references;
  NodeSchema *inheritance;
  string icon_path;

  /** Name, translations and descriptions */
  Localized name;

  /** A mapper from the sub id to the sub index */
  map<string, int> mapper;

  /** A mapper from the att id to the att index */
  map<string, int> att_mapper;

  static Logable log;
private:
  // ??
  string inheritance_name;
};



/** @brief Model schema loaded from a file */
class FileSchema
{
public:
  FileSchema();
  ~FileSchema();

  /** @brief Construit un sch�ma � partir d'un fichier xml */
  FileSchema(string filename);
  int from_file(string filename);
  int from_string(const string &s);
  int from_xml(const MXml &mx);
  void from_element(const Node &e);
  /** @param name name of the schema if not specified in the node */
  void add_schema(const Node &e, const string &name = "");
  FileSchema(const FileSchema &c);
  void operator =(const FileSchema &c);
  NodeSchema *get_schema(string name);
  NodeSchema *root;
  string to_string();
  /** Check if the schema is complete: returns -1 if not the case, 0 if ok. */
  int check_complete();
private:

  void from_xml2(const MXml &root);
  void from_element2(const Node &e);

  void build_references();

  vector<refptr<NodeSchema> > schemas;
  Logable log;
};

/** @brief Attribute value for a node
 *  TODO: should be private! */
class Attribute: public CProvider<ChangeEvent>
{
public:
  friend class Node;
  friend class RamNode;
  friend class ChangeEvent;

  Attribute();
  Attribute(refptr<AttributeSchema> schema);
  virtual ~Attribute(){}

  /** @returns Non-zero value if failed to set the new value */
  virtual int set_value(const string &s);
  void  set_value(int i);
  void  set_value(float f);
  void  set_value(bool b);
  int   set_value(const ByteArray &ba);

  bool   get_boolean() const;
  int    get_int() const;
  float  get_float() const;
  string get_string() const;

  void  serialize(ByteArray &ba) const;
  void  unserialize(ByteArray &ba);
  void  forward_change_event();
  refptr<AttributeSchema> schema;

private:
  /** Storage place */
  ByteArray value;

  bool up2date;
  bool inhibit_event_dispatch;
  Node *node;
  NodePatron *parent;

  static Logable log;
protected:
  virtual void value_changed(){}
};



/** @cond not-documented */

class NodeIterator
{
public:
  NodeIterator(NodePatron *parent, int type, int index)
    {this->parent = parent; this->type = type; this->index = index;}
  bool operator!=(const NodeIterator& x) const
  {
    return index != x.index;
  }
  const Node operator*() const;
  Node operator++();
private:
  int index;
  NodePatron *parent;
  int type;
};

class ConstNodeIterator
{
public:
  ConstNodeIterator(const NodePatron *parent, int type, int index) {this->parent = parent; this->type = type; this->index = index;}
  bool operator!=(const ConstNodeIterator& x) const
  {
    return index != x.index;
  }
  const Node operator*() const;
  const Node operator++();
private:
  int index;
  const NodePatron *parent;
  int type;
};

class NodeList
{
public:
  NodeList(NodePatron *parent, int type){this->parent = parent; this->type = type;}
  NodeIterator begin() const
  {
    return NodeIterator(parent, type, 0);
  }
  NodeIterator end() const;
private:
  NodePatron *parent;
  int type;
};


class ConstNodeList
{
public:
  ConstNodeList(const NodePatron *parent, int type){this->parent = parent; this->type = type;}
  ConstNodeIterator begin() const
  {
    return ConstNodeIterator(parent, type, 0);
  }
  ConstNodeIterator end() const;
private:
  const NodePatron *parent;
  int type;
};

/** @endcond */

/** @brief A tree node */
class Node
{
public:
  friend class Attribute;
  friend class NodeIterator;
  friend class ConstNodeIterator;
  friend class NodeList;
  friend class ConstNodeList;
  friend class ChangeEvent;

  Node(const Node &e);
  Node(Node &e);

  /** Copy only the reference */
  void operator =(const Node &e);

  /** True if points to the same reference */
  bool operator ==(const Node &e) const;

  /** True if different references */
  bool operator !=(const Node &e) const;

  /** Uninitialized or invalid tree node ? */
  bool is_nullptr() const;

  /** Get parent node */
  Node parent() const;

  /** Change the order of a child */
  Node           down(Node child);
  
  /** Change the order of a child */
  Node           up(Node child);

  // GET CHILDREN, TYPE PRECISE
  NodeList       children(const string &type);
  ConstNodeList  children(const string &type) const;
  unsigned long  get_children_count(const string &type) const;
  Node           get_child_at(const string &type, unsigned int i);
  const Node     get_child_at(const string &type, unsigned int i) const;


  void           copy_from(const Node e);

  void           add_listener(CListener<ChangeEvent> *lst);
  void           remove_listener(CListener<ChangeEvent> *lst);

  /// (1) Gestion des attributs
  bool           has_attribute(const XPath &path) const;

  bool           get_attribute_as_boolean(const string &name) const;
  int            get_attribute_as_int(const string &name) const;
  float          get_attribute_as_float(const string &name) const;
  string         get_attribute_as_string(const string &name) const;
  ByteArray      get_attribute_as_raw(const string &name) const;

  int            set_attribute(const XPath &path, const string &value);
  int            set_attribute(const XPath &path, int value);
  int            set_attribute(const XPath &path, bool value);
  int            set_attribute(const XPath &path, float value);
  int            set_attribute(const XPath &path, const char *value);
  int            set_attribute(const XPath &path, const ByteArray &value);

  Node           add_child(Node nv);
  Node           add_child(NodeSchema *schema);
  Node           add_child(const string &sub_name);

  Node           get_child(const XPath &path);
  const Node     get_child(const XPath &path) const;

  void           remove_child(Node child);

  /** Returns the path to a given child */
  int            get_path_to(const Node &child, XPath &res);

  /// (2) Gestion des enfants
  bool           has_child(const XPath &path) const;


  /** @param root_path If not empty, a path from which all file paths will be stored
   *                   as relative paths.
   *                   If not specified, all file paths are stored as absolute paths. */
  string         to_xml(unsigned int indent = 0,
                        bool display_default_values = false,
                        bool display_spaces = true,
                        bool charset_latin = false,
                        const string root_path = "") const;

  string         to_html(unsigned int level = 0) const;


  void           serialize(ByteArray &res) const;
  void           unserialize(ByteArray &source);


  static Node create_ram_node();
  static Node create_ram_node(NodeSchema *schema);
  static Node create_ram_node_from_string(NodeSchema *schema, string content);
  static Node create_ram_node(NodeSchema *schema, string filename);

  int save(const string &filename, bool store_default_values = false);

  int load(const string &schema_file, const string &data_file);

  // ????? to deprecate
  void load(const string &filename);

  /** @returns the schema describing the structure of this node */
  virtual NodeSchema *schema() const;

  bool contains(const Node &elt);

  void get_children_recursive(const string &type, deque<Node> &res);

  ///////////////////////////////////////////////////////////////////
  // BELOW ARE DEPRECATED METHODS TO BE REMOVED
  ///////////////////////////////////////////////////////////////////

  /** Prevent any modification of this node and its children
   *  to be notified to the change listeners.
   *  The notifications are defered until unlock() is called.
   *  This enable to group multiple change events into a single one. */
  // @deprecated
  void lock();

  /** Enable notifications to be called */
  // @deprecated
  void unlock();


  // GET CHILDREN, TYPE NON PRECISE
  /** @deprecated */
  unsigned long  get_children_count() const;

  // REFERENCES
  // @deprecated
  unsigned int      get_reference_count() const;
  // @deprecated
  void              set_reference(const string &name, Node e);
  // @deprecated
  void              set_reference(const string &name, const XPath &path);
  // @deprecated
  XPath             get_reference_path(const string &name);
  // @deprecated
  const Node        get_reference_at(unsigned int i, string &name) const;


  /* to deprecate ? (ideally mask the Attribute class) */
  Attribute                *get_attribute(const XPath &path);

  /* to deprecate ? (ideally mask the Attribute class) */
  const Attribute          *get_attribute(const XPath &path) const;

  /* to deprecate! */
  string name() const {return get_attribute_as_string("name");}

  /* to deprecate! */
  string description() const;

  /* to deprecate! */
  string get_identifier(bool disp_type = true, bool bold = false) const;

  /* to deprecate! */
  string get_localized_name() const;

  // @deprecated
  bool                      has_reference(const string &name) const;

  // @deprecated
  const Node                get_reference(const string &name) const;

  // @deprecated
  string               text_resume(int indent = 0) const;



  // @deprecated
  virtual string class_name() const;

  // @deprecated
  virtual string       type() const;

  // ?
  bool is_attribute_valid(const string &name);

  virtual ~Node();
  Node();

  Localized get_localized() const;

  /* To move another place */
  string format_c_comment(int indent);

  void dispatch_event(const ChangeEvent &ce);

  /** @returns Full path to this node */
  string get_fullpath() const;

  static Logable log;

protected:
  void setup_refs();

  //refptr<NodePatron> data;
  //Node(refptr<NodePatron> data);
  NodePatron *data;
  Node(NodePatron *data);

  friend class RamNode;
  friend class RomNode;



private:
  // Should be private!
  void fromXml(const MXml &e, string root_path = "");
  string to_xml_atts(unsigned int indent = 0,
                     bool display_default_values = false,
                     bool charset_latin = false,
                     string root_path = "") const;
  void   check_min();
  void  setup_schema();
  void  setup_default_subs();
};



/** Drawing graphics from schemas */
class DotTools
{
public:
  DotTools();

  int export_html_att_table(string &res, const Node &schema);

  /** @brief Build a graphic representation of the schema using dots */
  void build_graphic(Node &schema, const string &output_filename);
private:
  string complete_dot_graph(Node section, int level);
  string get_name(const Node &e);
  string get_attribute_type_description(const Node &e);
  string get_attribute_long_description(const Node &e);
  Logable log;
};

class LatexWrapper
{
public:
  int export_att_table(string &res, const Node &schema);
private:
  string get_name(const Node &e);
  string get_attribute_type_description(const Node &e);
  string get_attribute_long_description(const Node &e);
  static Logable log;
};



/** Generate a c++ class from a node schema */
class NodeCppWrapper
{
public:
  NodeCppWrapper();

  int gen_ccp_wrapper(NodeSchema *schema, const string &path_c, const string &path_h);

  /** @brief Generate a class definition. */
  string gen_class(NodeSchema *schema, int indent = 0);

  string gen_class_impl(NodeSchema *schema);
private:

  string gen_attribute_comment(const AttributeSchema &as, int indent);

  string format_comment(int indent, const Localized &l);

  string gen_attribute_type(const AttributeSchema &as);
  string gen_indent(size_t indent);
  string gen_get_attribute_as(const AttributeSchema &as);

  Logable log;
};


}
}

#endif
