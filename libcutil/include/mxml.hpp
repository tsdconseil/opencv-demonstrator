#ifndef MXML_H
#define MXML_H


#include "trace.hpp"

#include <vector>
#include <string>



namespace utils
{
namespace model
{

class XmlAttribute
{
public:
    XmlAttribute();
    XmlAttribute(std::string name, std::string value);
    XmlAttribute(const XmlAttribute &a);
    XmlAttribute &operator =(const XmlAttribute &a);
    std::string name;
    std::string string_value;
    int to_int() const;
    bool to_bool() const;
    std::string to_string() const;
    double to_double() const;
};

/** @brief Objet xml */
class MXml
{
public:
  std::string name;
  std::vector<XmlAttribute> attributes;
  std::vector<MXml> children;
  std::vector<std::string> text;
  // Indique l'ordre des �l�ment (true = element, false = text)
  std::vector<bool> order;

  MXml();
  MXml(const MXml &mx);
  MXml &operator =(const MXml &mx);

  void add_child(const MXml &mx);
  void add_text(std::string s);

  /** Charge un fichier xml    */
  int from_file(std::string filename);
  int from_string(std::string s);
  MXml(std::string name, std::vector<XmlAttribute> *attributes, std::vector<MXml> *children);
  std::string dump() const;
  std::string dump_content() const;
  std::vector<MXml> get_children(std::string name) const;
  void get_children(std::string name, std::vector<const MXml *> &res) const;
  std::string get_name() const;
  MXml get_child(std::string name) const;
  MXml get_child(std::string balise_name, std::string att_name, std::string att_value);
  XmlAttribute get_attribute(std::string name) const;
  bool has_attribute(std::string name) const;
  bool has_child(std::string name) const;
  bool has_child(std::string balise_name, std::string att_name, std::string att_value) const;
  /** @brief convert "&amp;" to "&", "\\" to "\r\n" */
  static std::string xml_string_to_ascii(std::string s);
  /** @brief convert "&amp;" to "&" */
  static std::string ascii_string_to_xml(std::string s);
};

}
}


#endif


