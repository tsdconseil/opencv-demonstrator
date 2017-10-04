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
  virtual void add_vector(std::string name, void *data, uint32_t n);
  virtual void get_vector(std::string name, uint32_t index, void *data, uint32_t n);

  // GET CHILDREN, TYPE PRECISE
  virtual unsigned long     get_children_count(const std::string &type) const = 0;
  virtual Node           get_child_at(const std::string &type, unsigned int i) = 0;
  virtual const Node     get_child_at(const std::string &type, unsigned int i) const = 0;

  virtual unsigned long     get_children_count(int type) const = 0;
  virtual Node           get_child_at(int type, unsigned int i) = 0;
  virtual const Node     get_child_at(int type, unsigned int i) const = 0;

  // GET ATTRIBUTES
  virtual unsigned long     get_attribute_count() const = 0;
  virtual Attribute        *get_attribute_at(unsigned int i) = 0;
  virtual const Attribute  *get_attribute_at(unsigned int i) const = 0;

  // MODIFY
  virtual Node              add_child(const std::string sub_name) = 0;
  virtual void              add_children(const std::string &type, const std::vector<const MXml *> &lst) = 0;
  virtual void              remove_child(Node child) = 0;

  // REFERENCES
  virtual unsigned int      get_reference_count() const = 0;
  virtual void              set_reference(const std::string &name, Node e) = 0;
  virtual void              set_reference(const std::string &name, const XPath &path) = 0;
  virtual XPath             get_reference_path(const std::string &name) = 0;
  virtual const Node        get_reference_at(unsigned int i, std::string &name) const = 0;


  virtual Node              get_parent() = 0;

  virtual std::string            class_name() const;

  void                      reference();
  void                      dereference();
  virtual void              discard() {}

  void                      lock();
  void                      unlock();




  int nb_references;

  NodeSchema *schema;
  std::string    type;

  static journal::Logable log;

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
  std::string get_name();

  std::string name;
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

  void              add_children(const std::string &type, const std::vector<const MXml *> &lst);

  unsigned int      get_reference_count() const;
  const Node        get_reference_at(unsigned int i, std::string &name) const;
  void              set_reference(const std::string &name, Node e);
  void              set_reference(const std::string &name, const XPath &path);
  XPath             get_reference_path(const std::string &name);

  ///////////////////////////////////////////////////////////
  ///// Not const getter                                /////
  ///////////////////////////////////////////////////////////
  unsigned long     get_attribute_count() const;
  Attribute        *get_attribute_at(unsigned int i);
  const Attribute  *get_attribute_at(unsigned int i) const;

  // GET CHILD, TYPE PRECISE
  unsigned long     get_children_count(const std::string &type) const;
  Node              get_child_at(const std::string &type, unsigned int i);
  const Node        get_child_at(const std::string &type, unsigned int i) const;

  unsigned long     get_children_count(int type) const;
  Node              get_child_at(int type, unsigned int i);
  const Node        get_child_at(int type, unsigned int i) const;

  // MODIFY
  Node              add_child(const std::string sub_name);
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
    std::string type;
    std::deque<Node> nodes;
  };

  //std::deque<Node>     children;
  std::deque<NodeCol> children;

  // Obsolete
  std::deque<Reference>   references;
  std::deque<Attribute>   attributes;
  RamNode *parent;



  /** To distinguish between the child of the same types */
  std::string sub_type;

  friend class Node;


};



/** @endcond */

}
}

#endif
