/** @file export-doc.cc

    Copyright 2016 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of OCVDemo.

    OCVDemo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OCVDemo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/


#include "ocvdemo.hpp"


std::string OCVDemo::export_demos(utils::model::Node &cat, Localized::Language lg)
{
  std::string s;
  auto nb_demos = cat.get_children_count("demo");
  for(auto i = 0u; i < nb_demos; i++)
  {
    auto demo = cat.get_child_at("demo", i);
    s += "<tr>";
    if(i == 0)
    {
      s += "<td rowspan=\"" + utils::str::int2str(nb_demos) + "\"><b>";
      s +=  cat.get_localized().get_value(lg);
      s += "</b></td>";
    }
    s += "<td>" + demo.get_localized().get_value(lg) + "</td>";


    //s += "<td>" + demo.get_localized().get_description(lg) + "</td>";

    auto id = demo.name();
    NodeSchema *ns = fs_racine->get_schema(id);

    if(ns != nullptr)
    {
      std::string s2 = "<br/><img src=\"imgs/" + id + ".jpg\" alt=\"" + demo.get_localized().get_value(lg) + "\"/>";
      s += "<td>" + ns->name.get_description(lg) + s2 + "</td>";
    }
    else
    {
      s += "<td></td>";
      avertissement("Pas de schema / description pour %s.", id.c_str());
    }

    s += "</tr>";
  }
  return s;
}

std::string OCVDemo::export_html(Localized::Language lg)
{
  std::string s = "<table class=\"formtable\">\n";
  if(lg == Localized::Language::LANG_FR)
    s += std::string(R"(<tr><td colspan="2"><b>Démonstration</b></td><td><b>Description</b></td></tr>)") + "\n";
  else
    s += std::string(R"(<tr><td colspan="2"><b>Demonstration</b></td><td><b>Description</b></td></tr>)") + "\n";
  for(auto cat1: tdm.children("cat"))
  {
    for(auto cat2: cat1.children("cat"))
    {
      s += export_demos(cat2, lg);
    }
    s += export_demos(cat1, lg);
  }
  s += "</table>\n";

  return s;
}

void OCVDemo::export_captures()
{
  for(auto cat1: tdm.children("cat"))
  {
    for(auto cat2: cat1.children("cat"))
    {
      export_captures(cat2);
    }
    export_captures(cat1);
  }
}

void OCVDemo::export_captures(utils::model::Node &cat)
{
  auto nb_demos = cat.get_children_count("demo");
  for(auto i = 0u; i < nb_demos; i++)
  {

    auto demo = cat.get_child_at("demo", i);
    auto id = demo.get_attribute_as_string("name");

    // Quelques démo requièrent une caméra pour fonctionner
    if(!demo.get_attribute_as_boolean("export"))
      continue;

    trace_majeure("Export démo [%s]...", id.c_str());

    // Arrête tout
    setup_demo(tdm.get_child_at("cat", 0));
    signal_une_trame_traitee.clear();

    // Démarre nouvelle démo
    setup_demo(demo);
    infos("Attente fin traitement...");

    // Il n'y a qu'un seul thread GTK => doit laisser la main
    // aux autres tâches
    //signal_une_trame_traitee.wait(); => non car bloquant pour GTK
    while(!signal_une_trame_traitee.is_raised())
    {
      if(Gtk::Main::events_pending())
        Gtk::Main::iteration();
    }

    infos("Fin traitement ok.");

    std::string s = "../../site/data/log/opencv/demo/list/imgs/";

    s += id + ".jpg";

    Mat A = mosaique.get_global_img();

    if(A.data == nullptr)
    {
      avertissement("A.data == nullptr");
      continue;
    }

    cv::pyrDown(A, A);
    cv::pyrDown(A, A);

    trace_verbeuse("taille finale = %d * %d.", A.cols, A.rows);
    imwrite(s, A);
    if(!utils::files::file_exists(s))
      avertissement("Echec lors de la sauvegarde du fichier.");
  }
}


