#ifndef MODELE_PRIVATE_HPP
#define MODELE_PRIVATE_HPP

namespace utils
{
namespace model
{

/** @cond not-documented */

/** @internal
 *  @brief An abstract class for implementing a node */
class NodePatron: public     CProvider<ChangeEvent>,
                  protected  CListener<ChangeEvent>
{
public:
  friend class Node;
  friend class ChangeEvent;

  NodePatron();

  virtual ~NodePatron(){}

  ///////////////////////////////////////////////////////////
  //// METHODES VIRTUELLES PURES A IMPLEMENTER (PATRON)  ////
  ///////////////////////////////////////////////////////////
  virtual void add_vector(string name, void *data, uint32_t n);
  virtual void get_vector(string name, uint32_t index, void *data, uint32_t n);

  // GET CHILDREN, TYPE PRECISE
  virtual unsigned long     get_children_count(const string &type) const = 0;
  virtual Node           get_child_at(const string &type, unsigned int i) = 0;
  virtual const Node     get_child_at(const string &type, unsigned int i) const = 0;

  virtual unsigned long     get_children_count(int type) const = 0;
  virtual Node           get_child_at(int type, unsigned int i) = 0;
  virtual const Node     get_child_at(int type, unsigned int i) const = 0;

  // GET ATTRIBUTES
  virtual unsigned long     get_attribute_count() const = 0;
  virtual Attribute        *get_attribute_at(unsigned int i) = 0;
  virtual const Attribute  *get_attribute_at(unsigned int i) const = 0;

  // MODIFY
  virtual Node              add_child(const string &sub_name) = 0;
  virtual void              remove_child(Node child) = 0;

  // REFERENCES
  virtual unsigned int      get_reference_count() const = 0;
  virtual void              set_reference(const string &name, Node e) = 0;
  virtual void              set_reference(const string &name, const XPath &path) = 0;
  virtual XPath             get_reference_path(const string &name) = 0;
  virtual const Node        get_reference_at(unsigned int i, string &name) const = 0;


  virtual Node              get_parent() = 0;

  virtual string            class_name() const;

  void                      reference();
  void                      dereference();
  virtual void              discard() {}

  void                      lock();
  void                      unlock();

  int nb_references;

  NodeSchema *schema;
  string    type;

  static Logable log;

protected:
  bool ignore;
  bool change_detected;
  ChangeEvent last_change;

  /** Instance number inside parent */
  int instance;
  bool inhibit_event_raise;
  bool event_detected;
private:
  bool locked;


};

/** @internal
 *  @brief Concrete RAM reference (aka pointer) */
class Reference
{
public:
  void set_reference(Node elt);
  Node get_reference();
  string get_name();

  string name;
  XPath path;
  Node ptr;
};

/** @internal
 *  RAM implementation of a node */
class RamNode : public    NodePatron
{
public:
  //////////////////////////////////////////////////////////
  //// MODIFIERS                                        ////
  //////////////////////////////////////////////////////////

  Node              get_parent();

  unsigned int      get_reference_count() const;
  const Node        get_reference_at(unsigned int i, string &name) const;
  void              set_reference(const string &name, Node e);
  void              set_reference(const string &name, const XPath &path);
  XPath             get_reference_path(const string &name);

  ///////////////////////////////////////////////////////////
  ///// Not const getter                                /////
  ///////////////////////////////////////////////////////////
  unsigned long     get_attribute_count() const;
  Attribute        *get_attribute_at(unsigned int i);
  const Attribute  *get_attribute_at(unsigned int i) const;

  // GET CHILD, TYPE PRECISE
  unsigned long     get_children_count(const string &type) const;
  Node              get_child_at(const string &type, unsigned int i);
  const Node        get_child_at(const string &type, unsigned int i) const;

  unsigned long     get_children_count(int type) const;
  Node              get_child_at(int type, unsigned int i);
  const Node        get_child_at(int type, unsigned int i) const;

  // MODIFY
  Node              add_child(const string &sub_name);
  void              remove_child(Node child);



  void on_event(const ChangeEvent &ce);

  /////////////////////////////////////////////////////////////////
  /// Constructors, copy operator                              ////
  /////////////////////////////////////////////////////////////////
  RamNode();
  RamNode(NodeSchema *schema);
  virtual ~RamNode();
  void operator =(const RamNode &e);

  //////////////////////////////////////////////////////////
  //// A CLASSER                                        ////
  //////////////////////////////////////////////////////////

private:
  // TODO: class children by type
  class NodeCol
  {
  public:
    string type;
    deque<Node> nodes;
  };

  //deque<Node>     children;
  deque<NodeCol> children;

  // Obsolete
  deque<Reference>   references;
  deque<Attribute>   attributes;
  RamNode *parent;



  /** To distinguish between the child of the same types */
  string sub_type;

  friend class Node;


};



/** @endcond */

}
}

#endif
