#include "cutil.hpp"
#include "mxml.hpp"
#include <stdio.h>

#include <iostream>
#include <ostream>
#include <string>
#include <fstream>

//FILE *os, *toc_os;




/*std::string handle_item(const MXml &mx)
{
  
  if(mx.name.compare("em") == 0)
  {
    return std::string("<i>") + handle_children(mx) + "</i>";
  }
}


std::string handle_children(const MXml &mx)
{
  int i, j, k;
  j = 0;
  k = 0;
  for(i = 0; i < mx.order.size(); i++)
  {
    if(mx.order[i])
    {
      handle_element(mx.children[j]);
      j++;
    }
    else
    {
      fprintf(os, "%s", mx.text[k].c_str());
      k++;
    }
  }
}*/


using namespace utils;
using namespace utils::model;
using namespace std;

ofstream os, toc_os;

char current_section[255];
char current_part[255];

void handle_element(const MXml &mx)
{
  
  if(mx.name.compare("include") == 0)
  {
    std::string fn = mx.get_attribute("path").to_string();

    MXml xinc;
    infos("Loading included file: %s...", fn.c_str());
    int ret = xinc.from_file(fn);
    infos("Done.");
    if(ret == 0)
      handle_element(xinc);
    else
    {
      erreur("Unable to open %s.\n", fn.c_str());
    }
    return;
  }

  int i, j, k;
  
  
  /** Add labels to TOC */
  if(mx.name.compare("label") == 0)
  {
    toc_os << "    <toc-label name=\"" << str::utf8_to_latin(mx.get_attribute("name").to_string()) << "\" ";
    toc_os << "section=\"" << str::utf8_to_latin(current_section) << "\" ";
    toc_os << "part=\"" << str::utf8_to_latin(current_part) << "\"/>\n";
  }
  
  if(mx.name.compare("section") == 0)
  {
    std::string lname, name;
    name = mx.get_attribute("name").to_string();
    if(mx.has_attribute("label"))
      lname = mx.get_attribute("label").to_string();
    else
    {
      lname = name;
      avertissement("Section without label: '%s'.\n", lname.c_str());
    }
    sprintf(current_section, "%s", lname.c_str());
    toc_os <<// "  <toc-section label=\"%s\" name=\"%s\"/>\n", lname.c_str(), name.c_str());
        "  <toc-section label=\"" << lname << "\" name=\"" << name << "\"/>\n";
  }
  
  if(mx.name.compare("sub-section") == 0)
    {
      std::string lname, name;
      name = str::utf8_to_latin(mx.get_attribute("name").to_string());
      if(mx.has_attribute("label"))
        lname = str::utf8_to_latin(mx.get_attribute("label").to_string());
      else
      {
        lname = name;
        avertissement("Sub-section without label: '%s'.\n", lname.c_str());
      }
      toc_os << // "  <toc-sub-section label=\"%s\" name=\"%s\" section=\"%s\"/>\n",
          //lname.c_str(), name.c_str(), current_section);
          "  <toc-sub-section label=\"" << lname << "\" name=\"" << name << "\" section=\"" << current_section << "\"/>\n";
    }
  
  if(mx.name.compare("part") == 0)
  {
    std::string lname, name;
    name = str::utf8_to_latin(mx.get_attribute("name").to_string());
    if(mx.has_attribute("label"))
      lname = str::utf8_to_latin(mx.get_attribute("label").to_string());
    else
    {
      lname = name;
      avertissement("Part without label: '%s'.\n", lname.c_str());
    }
    sprintf(current_part, "%s", lname.c_str());
    toc_os << //"<toc-part label=\"%s\" name=\"%s\"/>\n", lname.c_str(), name.c_str());
        "<toc-part label=\"" << lname << "\" name=\"" << name << "\"/>\n";
  }
  
  os << "<" << str::utf8_to_latin(mx.name) << " ";
  for(i = 0; i < (int) mx.attributes.size(); i++)
  {
    os << str::utf8_to_latin(mx.attributes[i].name)
       << " = \"" << str::utf8_to_latin(mx.attributes[i].string_value) << "\" ";
  }
  os << ">";
  j = 0;
  k = 0;
  for(i = 0; i < (int) mx.order.size(); i++)
  {
    if(mx.order[i])
    {
      handle_element(mx.children[j]);
      j++;
    }
    else
    {
      os << str::utf8_to_latin(mx.text[k]);
      k++;
    }
  }
  os << "</" << mx.name << ">";
  
  if(mx.name.compare("section") == 0)
  {
    sprintf(current_section, "no section");
  }
}


int main(int argc, char **argv)
{
  CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "lcutil", "preprocess");

  current_part[0] = 0x00;
  current_section[0] = 0x00;
  
  if(argc < 2)
    argv[1] = "./root.xml";
  if(argc < 3)
    argv[2] = "./tmp.xml";
  
  printf("preprocess %s -> %s\n", argv[1], argv[2]);
  MXml mx;



  infos("Loading %s...", argv[1]);
  mx.from_file(argv[1]);
  infos("Done.");

  os.open(argv[2], std::ofstream::out);
  toc_os.open("./toc.xml", std::ofstream::out);
  //os = fopen(argv[2], "wt");
  //toc_os = fopen("./toc.xml", "wt");


  os << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
  toc_os << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
  
  toc_os << "<toc>\n";
  handle_element(mx);
  toc_os << "</toc>\n";

  os.close();
  toc_os.close();
  
  printf("Done.\n");
  return 0;
}
