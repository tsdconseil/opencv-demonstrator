/*
 * schema2doc.cc
 *
 *  Created on: 27 août 2013
 *      Author: A
 */

#include "cutil.hpp"
#include "mxml.hpp"
#include "modele.hpp"
#include <stdio.h>
#include <iostream>

using namespace utils;
using namespace model;
using namespace std;

void print_usage()
{
  cerr << "Usage:\nschema2doc -i (schema_filename.xml) -f latex|html [-o ofilename]" << endl;
}


int main(int argc, char **argv)
{
  CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "libcutil", "schema2doc");

  if(!cmdeline.has_option("-i"))
  {
    print_usage();
    return -1;
  }

  string fp = cmdeline.get_option("-i", "");

  string fmt = cmdeline.get_option("-f", "latex");

  string ofile = cmdeline.get_option("-o", "./a.out");

  /*FileSchema fs;
  if(fs.from_file(Util::get_fixed_data_path() + PATH_SEP + "std-schema.xml"))
    return -1;*/

  /*Node n;
  if(n.load(Util::get_fixed_data_path() + PATH_SEP + "std-schema.xml", fp))
    return -1;*/

  FileSchema fs;
  if(fs.from_file(utils::get_fixed_data_path() + PATH_SEP + "std-schema.xml"))
    return -1;
  Node n = Node::create_ram_node(fs.root, fp);

  Node root = n.get_child_at("node", 0);

  if(fmt.compare("html") == 0)
  {
    DotTools du;
    string res;

    if(du.export_html_att_table(res, root))
      return -1;


    string main_title = "";
    string header = "<html><head><title>" + main_title + "</title>"
                    "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>"
                    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>"
                    "</head><body style=\"text-align:center;\">";

    res = header + res + "</body></html>";

    return files::save_txt_file(ofile, res);
  }
  else if(fmt.compare("latex") == 0)
  {
    LatexWrapper du;
    string res;

    if(du.export_att_table(res, root))
      return -1;

    return files::save_txt_file(ofile, str::to_latex(str::utf8_to_latin(res)));
  }
  else
  {
    print_usage();
    return -1;
  }
}





