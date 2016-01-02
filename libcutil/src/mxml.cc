#include "mxml.hpp"
#include "cutil.hpp"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

extern utils::model::MXml *closure;
std::string current_file = "";
extern int linenum;

extern int xmlparse();
extern FILE *xmlin;

# ifndef WIN
extern void xmllex_destroy();
# endif

namespace utils
{
namespace model
{

using namespace utils;



XmlAttribute::XmlAttribute(const XmlAttribute &a)
{
    *this = a;
}

XmlAttribute &XmlAttribute::operator =(const XmlAttribute &a)
{
  name = a.name;
  string_value = a.string_value;
  return *this;
}
        

MXml::MXml()
{
  name = "?";
}

MXml::MXml(const MXml &mx)
{
  *this = mx;
}

MXml &MXml::operator =(const MXml &mx)
{
  name        = mx.name;
  attributes  = mx.attributes;
  children    = mx.children;
  text        = mx.text;
  order       = mx.order;
  return *this;
}


std::string MXml::getName() const
{
	return name;
}

bool MXml::hasChild(std::string balise_name, std::string att_name, std::string att_value) const
{
  for(unsigned int i = 0; i < children.size(); i++)
	{
		if(children[i].name.compare(balise_name) == 0)
		{
		  if(children[i].hasAttribute(att_name))
		  {
		    if(children[i].getAttribute(att_name).string_value.compare(att_value) == 0)
		      return true;
		  }
		}
	}
    return false;
}

MXml MXml::getChild(std::string balise_name, std::string att_name, std::string att_value)
{
	for(unsigned int i = 0; i < children.size(); i++)
	{
		if(children[i].name.compare(balise_name) == 0)
		{
			if(children[i].getAttribute(att_name).string_value.compare(att_value) == 0)
				return children[i];
		}
	}
	log_anomaly(main_log, "XML child not found: %s in %s where %s = %s.",
	 balise_name.c_str(), name.c_str(), att_name.c_str(), att_value.c_str());
	return MXml();
}

bool MXml::hasChild(std::string name) const
{
    for(unsigned int i = 0; i < children.size(); i++)
    {
        if(children[i].name.compare(name) == 0)
            return true;
    }
    return false;
}

MXml MXml::getChild(std::string name) const
{
	for(unsigned int i = 0; i < children.size(); i++)
	{
		if(children[i].name.compare(name) == 0)
			return children[i];
	}
	log_anomaly(main_log, "Child not found: %s in %s.", name.c_str(), this->name.c_str());
	return MXml();
}

 
void MXml::get_children(std::string name, std::vector<const MXml *> &res) const
{
  unsigned int n = children.size();
  for(unsigned int i = 0; i < n; i++)
  {
    if(children[i].name.compare(name) == 0)
      res.push_back(&(children[i]));
  }
}

std::vector<MXml> MXml::getChildren(std::string name) const
{
  std::vector<MXml> res;
  for(unsigned int i = 0; i < children.size(); i++)
  {
    if(children[i].name.compare(name) == 0)
      res.push_back(children[i]);
  }
  return res;
}

bool MXml::hasAttribute(std::string name) const
{
	for(unsigned int i = 0; i < attributes.size(); i++)
	{
		if(attributes[i].name.compare(name) == 0)
			return true;
	}
	return false;
}

XmlAttribute MXml::getAttribute(std::string name) const
{
	for(unsigned int i = 0; i < attributes.size(); i++)
	{
		if(attributes[i].name.compare(name) == 0)
			return attributes[i];
	}
	//xml_trace("MXml::getAttribute(" + name + "): attribute not found.\n");

	log_anomaly(main_log, "getAttribute(%s): attribute not found in %s.", name.c_str(), this->name.c_str());
	return XmlAttribute();
}

MXml::MXml(std::string name, std::vector<XmlAttribute> *attributes, std::vector<MXml> *children)
{
  this->name = name;
  this->attributes = *attributes;
  delete attributes;
  this->children = *children;
  delete children;
}


int MXml::from_string(std::string s)
{
  linenum = 0;
  attributes.clear();
  children.clear();

  xmlin = tmpfile();
  if(xmlin == nullptr)
  {
    log_anomaly(main_log, "Unable to create temp file.");
    return -1;
  }

  fprintf(xmlin, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>%s", s.c_str());
  fseek(xmlin, 0, SEEK_SET);


  closure = nullptr;
  try
  {
    xmlparse();
  }
  catch(std::string &se)
  {
    log_anomaly(main_log, "Exception occurred while reading [%s]: %s.",
            s.c_str(), se.c_str());
    fclose(xmlin);
    return -1;
  }
  catch(...)
  {
    log_anomaly(main_log, "Unknown exception occurred while reading [%s].", s.c_str());
    fclose(xmlin);
    return -1;
  }
  fclose(xmlin);
  if(closure == nullptr)
  {
    log_anomaly(main_log, "Parse error in:\n%s", s.c_str());
    return -1;
  }
  else
  {
    *this = *closure;
    delete closure;
    return 0;
  }
}

int MXml::from_file(std::string filename)
{
  current_file = filename;
  linenum = 0;
  this->attributes.clear();
  this->children.clear();

  xmlin = fopen(filename.c_str(), "rt");
  if(xmlin == nullptr)
  {
    log_anomaly(main_log, "File not found: %s.", filename.c_str());
    return -1;
  }
	
  closure = nullptr;
  try
  {
    xmlparse();
  }
  catch(std::string s)
  {
    log_anomaly(main_log, "Exception occurred while reading [%s]: %s.",
            filename.c_str(), s.c_str());
    fclose(xmlin);
    return -1;
  }
  catch(...)
  {
    log_anomaly(main_log, "Unknown exception occurred while reading [%s].", filename.c_str());
    fclose(xmlin);
    return -1;
  }
  fclose(xmlin);

  //extern void xmllex_destroy();
  //xmllex_destroy();
# ifndef WIN
  //extern void xmllex_destroy();
  xmllex_destroy();
# endif


  if(closure == nullptr)
  {
    log_anomaly(main_log, "Parse error in %s.", filename.c_str());
    return -1;
  }
  else
  {
    *this = *closure;
    delete closure;
    return 0;
  }
}

void MXml::add_child(const MXml &mx)
{
    order.push_back(true);
    children.push_back(mx);
}

void MXml::add_text(std::string s)
{
    order.push_back(false);
    text.push_back(str::latin_to_utf8(s));
}

std::string MXml::dumpContent() const
{
    std::string res = "";
    int index_el = 0;
    int index_tx = 0;
    for(unsigned int i = 0; i < order.size(); i++)
    {
        if(order[i])
            res += children[index_el++].dump();
        else
            res += text[index_tx++];
    }
    const char *s = res.c_str();
    bool only_spaces = true;
    for(unsigned int i = 0; i < strlen(s); i++)
    {
        if(s[i] != ' ')
        {
            only_spaces = false;
            break;
        }
    }
    if(only_spaces)
        return "";
    return res;
}

std::string MXml::dump() const
{
    std::string res = "<" + name;

    if((attributes.size() == 0) && (order.size() == 0))
      return res + "/>";


    for (unsigned int i = 0; i < attributes.size(); i++) 
        res += " " + attributes[i].name + "=\"" + attributes[i].string_value + "\"";
    res += ">";
    int index_el = 0;
    int index_tx = 0;
    for(unsigned int i = 0; i < order.size(); i++)
    {
        if(order[i])
            res += children[index_el++].dump();
        else
            res += text[index_tx++];
    }
    /*
    std::vector<MXml>::iterator it2;
    for (int i = 0; i < children.size(); i++) 
        res += children[i].dump();
    res += text;*/
    res += "</" + name + ">";
    return res;
}

XmlAttribute::XmlAttribute()
{
    this->string_value = "?";
    name = "?";
}

XmlAttribute::XmlAttribute(std::string name, std::string value)
{
    this->string_value = str::latin_to_utf8(value);
    this->name = name;
}

int XmlAttribute::toInt()
{
	char temp[20];
	sprintf(temp, "%s", string_value.c_str());
	int val = -1;
        //std::cout << "Scanning '" << std::string(temp) << "' : ";
	sscanf(temp, "%d", &val);
        //printf("%d\n", val);
	return val;
}

std::string XmlAttribute::toString()  const
{
  const char *s = string_value.c_str();
  char buf[1000];
  unsigned int n = strlen(s);
  unsigned int j = 0;
  for(unsigned int i = 0; i < n; i++)
  {
    if((s[i] == '\\') && (i < n - 1) && (s[i+1] == 'G'))
    {
      buf[j++] = '"';
      i++;
    }
    else
    {
      buf[j++] = s[i];
    }
  }
  buf[j] = 0;
  return std::string(buf);
}

bool XmlAttribute::toBool()
{
  return (string_value.compare("true") == 0);
}

double XmlAttribute::toDouble()
{
	char temp[30];
	sprintf(temp, "%s", string_value.c_str());
	float val = -1;
	sscanf(temp, "%f", &val);
	return val;
}


void yyerror(const char *s)
{
    //printf("Error : %s\n", s);
}

std::string MXml::xml_string_to_ascii(std::string s)
{
    char *res = (char *) malloc(s.size()+1);
    const char *s2 = s.c_str();
    unsigned int di = 0;
    for(unsigned int i = 0; i < strlen(s2); i++)
    {
        if(((i+4) < strlen(s2)) && (s2[i] == '&') && (s2[i+1] == 'a') && (s2[i+2] == 'm') && (s2[i+3] == 'p') && (s2[i+4] == ';'))
        {
            i += 4;
            res[di++] = '&';
        }
        else if(((i+1) < strlen(s2)) && (s2[i] == '\\') && (s2[i+1] == '\\'))
        {
            i++;
            res[di++] = '\n';
        }
        else
            res[di++] = s2[i];
    }
    res[di] = 0;
    std::string sres = std::string(res);
    free(res);
    return sres;
}

std::string MXml::ascii_string_to_xml(std::string s)
{
    char *res = (char *) malloc(s.size()+100);
    const char *s2 = s.c_str();
    unsigned int di = 0;
    for(unsigned int i = 0; i < strlen(s2); i++)
    {
        if(s2[i] == '&')
        {
            res[di++] = '&';
            res[di++] = 'a';
            res[di++] = 'm';
            res[di++] = 'p';
            res[di++] = ';';
        }
        else if(s2[i] == '\n')
        {
            res[di++] = '\\';
            res[di++] = '\\';
        }
        else
            res[di++] = s2[i];
    }
    res[di] = 0;
    std::string sres = std::string(res);
    free(res);
    return sres;
}

}
}

