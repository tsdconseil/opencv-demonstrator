#include "vue-collection.hpp"
#ifdef WIN
# include <windows.h>
#endif

namespace ocvext{

void VueCollection::gere_b_ouvrir_image()
{
  auto noeud = arbre.get_selection();

  if(noeud.has_attribute("chemin"))
  {
    auto s = noeud.get_attribute_as_string("chemin");
    infos("Ouverture [%s]...", s.c_str());

# ifdef WIN
    ::ShellExecute(NULL, "open", s.c_str(), NULL, NULL, SW_SHOW);
# endif
  }
}

VueCollection::VueCollection(): vue(200,300,true)
{
  //fenetre.set_title(titre);
  vbox_princ.pack_start(barre_outil, Gtk::PACK_SHRINK);
  vbox_princ.pack_start(hpaned, Gtk::PACK_EXPAND_WIDGET);

  b_ouvrir_image.set_sensitive(false);
  b_ouvrir_image.set_label("Ouvrir avec windows");
  barre_outil.append(b_ouvrir_image, sigc::mem_fun(*this, &VueCollection::gere_b_ouvrir_image));

  fenetre.add(vbox_princ);
  hpaned.add1(arbre);
  hpaned.add2(vbox);
  //vbox.pack_start(vue);
  vue_en_cours = nullptr;
  fenetre.show_all_children(true);
  arbre.add_listener(this, &VueCollection::gere_change_sel);
  hpaned.set_position(300);
  std::vector<std::string> ids;
  ids.push_back("img-collection");
  ids.push_back("img-spec");
  arbre.set_liste_noeuds_affiches(ids);
}

void VueCollection::gere_change_sel(const utils::mmi::SelectionChangeEvent &sce)
{
  //trace_verbeuse("changement selection.");
  auto sel = sce.new_selection;

  auto type = sel.schema()->name.get_id();


  bool est_image = (type == "img-spec");
  b_ouvrir_image.set_sensitive(est_image);

  if(vue_en_cours != nullptr)
    vbox.remove(*vue_en_cours);
  vue_en_cours = nullptr;

  if(type == "img-collection")
  {
    //cv::Mat m(1,1,CV_8UC3,cv::Scalar(0,0,0));
    //vue.maj(m);

    std::string s;
    for(auto &l: sel.children("log-item"))
    {
      s += l.get_attribute_as_string("name");
      s += "\n";
    }
    texte.get_buffer()->set_text(s);

    vbox.pack_start(texte, Gtk::PACK_EXPAND_WIDGET);
    vue_en_cours = &texte;
  }
  else if(type == "img-spec")
  {
    vbox.pack_start(vue, Gtk::PACK_EXPAND_WIDGET);
    vue_en_cours = &vue;
    vue.maj(sel.get_attribute_as_string("chemin"));
  }
  else
    avertissement("VueCollection : type non gere (%s)", type.c_str());
  fenetre.show_all_children(true);
}

void VueCollection::affiche(utils::model::Node &modele)
{
  //auto s = modele.to_xml();
  //infos("Affichage collection, modele :\n%s", s.c_str());


  this->modele = modele;

  arbre.set_model(modele);

  // modele_arbre = utils::model::Node(modele.schema());
  // modele_arbre.copy_from(modele);
  // Construction modele arbre


  fenetre.show_all_children(true);
  fenetre.show();

}

void VueCollection::ferme()
{
  fenetre.hide();
}



}

