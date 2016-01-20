/** @file image-selecteur.cc

    Copyright 2015 J.A. / http://www.tsdconseil.fr

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

#include "tools/image-selecteur.hpp"
#include "ocvdemo.hpp"
#include <glibmm.h>



void ImageSelecteur::maj_langue()
{
  b_maj.set_label(utils::langue.get_item("b_maj"));
  b_suppr_tout.set_label(utils::langue.get_item("b_del_tout"));
  b_suppr.set_label(utils::langue.get_item("b_del"));
  b_open.set_label(utils::langue.get_item("b_open"));
  b_ajout.set_label(utils::langue.get_item("b_ajout"));
  set_title(utils::langue.get_item("titre-sel"));
}

void ImageSelecteur::maj_actif()
{
  bool ajout = true, retire = (csel != -1);

  if((nmax >= 0) && (images.size() >= (unsigned int) nmax))
    ajout = false;

  if((nmin >= 0) && (images.size() <= (unsigned int) nmin))
    retire = false;

  b_ajout.set_sensitive(ajout);//images.size() < );
  b_open.set_sensitive(csel != -1);
  b_suppr.set_sensitive(retire);
  b_suppr_tout.set_sensitive(nmin <= 0);//images.size() > 0);
  b_maj.set_sensitive(images.size() > 0);
}

void ImageSelecteur::maj_taille()
{
  maj_mosaique();
}

void ImageSelecteur::maj_selection()
{
  auto n = images.size();
  for(auto i = 0u; i < n; i++)
  {
    Image &im = images[i];
    cv::Scalar color(80,80,80);
    if(csel == (int) i)
      color = cv::Scalar(0,255,0);
    cv::rectangle(bigmat,
        cv::Rect(im.px - 3, im.py - 3, img_width + 3, img_height + 3),
                 color, 3);
  }
  journal.verbose("reshow...");
  //this->gtk_image.show();
  gtk_image.queue_draw();
}


void ImageSelecteur::maj_mosaique()
{
  int width, height;
  width = gtk_image.get_allocated_width();
  height = gtk_image.get_allocated_height();

  if((width <= 0) || (height <= 0))
    return;

  if((width != bigmat.cols) || (height != bigmat.rows))
  {
    bigmat = cv::Mat::zeros(cv::Size(width,height), CV_8UC3);
    pixbuf = Gdk::Pixbuf::create_from_data(bigmat.data,
          Gdk::Colorspace::COLORSPACE_RGB,
          false,
          8,
          bigmat.cols,
          bigmat.rows,
          3 * bigmat.cols);
    gtk_image.set(pixbuf);
  }
  else
    bigmat = cv::Scalar(0);


  auto n = images.size();

  if(n == 0)
  {
    gtk_image.queue_draw();
    return;
  }

  if((n == 0) || (width <= 0) || (height <= 0))
    return;

  nrows = (unsigned int) floor(sqrt(n));
  ncols = (unsigned int) ceil(((float) n) / nrows);

  col_width = width / ncols;
  row_height = height / nrows;

  unsigned int txt_height = 0;//30;

  img_width = col_width - 6;
  img_height = row_height - 6 - txt_height;

  journal.verbose("nrows=%d, ncols=%d.", nrows, ncols);
  //journal.verbose("bigmat: %d * %d.")

  unsigned int row = 0, col = 0;
  for(auto i = 0u; i < n; i++)
  {
    Image &im = images[i];
    im.ix = col;
    im.iy = row;
    im.px = im.ix * col_width + 3;
    im.py = im.iy * row_height + 3;



    journal.verbose("resize(%d,%d,%d,%d,%d,%d)", im.px, im.py, col_width, row_height, col_width, row_height);

    cv::Mat tmp;

    cv::cvtColor(im.mat, tmp, CV_BGR2RGB);
    cv::resize(tmp,
        bigmat(cv::Rect(im.px, im.py, img_width, img_height)),
        cv::Size(img_width, img_height));



    col++;
    if(col >= ncols)
    {
      col = 0;
      row++;
    }
  }

  maj_selection();
}

void ImageSelecteur::on_size_change(Gtk::Allocation &alloc)
{
  maj_taille();
}

ImageSelecteur::ImageSelecteur()
{
  nmin = 0;
  nmax = 100;
  has_a_video = false;
  this->add(vbox);
  vbox.pack_start(toolbar, Gtk::PACK_SHRINK);
  evt_box.add(gtk_image);
  vbox.pack_start(evt_box, Gtk::PACK_EXPAND_WIDGET);
  //set_size_request(300,200);
  set_default_size(450, 300);

  csel = -1;

  maj_taille();

  toolbar.add(b_open);
  toolbar.add(b_ajout);
  toolbar.add(b_suppr);
  toolbar.add(b_suppr_tout);
  toolbar.add(b_maj);


  b_maj.set_stock_id(Gtk::Stock::REFRESH);
  b_suppr_tout.set_stock_id(Gtk::Stock::REMOVE);
  b_suppr.set_stock_id(Gtk::Stock::REMOVE);
  b_open.set_stock_id(Gtk::Stock::OPEN);
  b_ajout.set_stock_id(Gtk::Stock::ADD);

  maj_langue();
  maj_actif();


  show_all_children(true);

  evt_box.set_can_focus(true);
  evt_box.add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

  evt_box.signal_button_press_event().connect(
                   sigc::mem_fun(*this,
                   &ImageSelecteur::on_b_pressed));
  evt_box.signal_button_release_event().connect(
                   sigc::mem_fun(*this,
                   &ImageSelecteur::on_b_released));

  evt_box.signal_key_release_event().connect(
      sigc::mem_fun(*this,
      &ImageSelecteur::on_k_released));


  gtk_image.signal_size_allocate().connect(
      sigc::mem_fun(*this,
      &ImageSelecteur::on_size_change));

  b_open.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageSelecteur::on_b_open));
  b_ajout.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageSelecteur::on_b_add));
  b_suppr.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageSelecteur::on_b_del));
  b_suppr_tout.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageSelecteur::on_b_del_tout));
  b_maj.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageSelecteur::on_b_maj));

  std::vector<Gtk::TargetEntry> listTargets;
  listTargets.push_back(Gtk::TargetEntry("text/uri-list"));
  drag_dest_set(listTargets, Gtk::DEST_DEFAULT_MOTION | Gtk::DEST_DEFAULT_DROP, Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
  signal_drag_data_received().connect(sigc::mem_fun(*this, &ImageSelecteur::on_dropped_file));
}

void ImageSelecteur::on_dropped_file(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time)
{
 if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
 {
   std::vector<Glib::ustring> file_list;

   file_list = selection_data.get_uris();

   for(auto i = 0u; i < file_list.size(); i++)
   {
     Glib::ustring path = Glib::filename_from_uri(file_list[i]);
     std::string s = path;
     journal.trace("DnD: %s.", s.c_str());
     ajoute_fichier(s);
   }
   context->drag_finish(true, false, time);
   return;
 }
 context->drag_finish(false, false, time);
}

bool ImageSelecteur::has_video()
{
  return has_a_video;
}

void ImageSelecteur::get_video_list(std::vector<std::string> &list)
{
  list.clear();
  for(auto i: images)
    list.push_back(i.fichier);
}

unsigned int ImageSelecteur::get_nb_images() const
{
  return images.size();
}

void ImageSelecteur::get_list(std::vector<cv::Mat> &list)
{
  list.clear();
  for(auto i: images)
    list.push_back(i.mat);
}


void ImageSelecteur::set_fichier(int idx, std::string s)
{
  if(s.size() == 0)
    return;

  journal.verbose("set [#%d <- %s]...", idx, s.c_str());
  Image img;
  img.fichier = s;
  std::string dummy;
  utils::files::split_path_and_filename(s, dummy, img.nom);
  std::string ext = utils::files::get_extension(img.nom);
  img.nom = utils::files::remove_extension(img.nom);

  if((ext == "mpg") || (ext == "avi") || (ext == "mp4") || (ext == "wmv"))
  {
    has_a_video = true;

    cv::VideoCapture vc(s);

    if(!vc.isOpened())
    {
      utils::mmi::dialogs::show_error("Error",
                "Error while loading video",
                "Maybe the video format is not supported.");
            return;
    }

    // Lis seulement la première image
    vc >> img.mat;

    vc.release();
  }
  else
  {
    img.mat = cv::imread(s);
    if(img.mat.data == nullptr)
    {
      utils::mmi::dialogs::show_error("Error",
          "Error while loading image",
          "Maybe the image format is not supported.");
      return;
    }
  }

  images[idx] = img;

  /*if((nmax > 0) && (images.size() >= (unsigned int) nmax))
    images.erase(images.begin());

  images.push_back(img);*/

  csel = idx;

  maj_mosaique();
  maj_actif();

  if(nmax == 1)
    on_b_maj();
}


void ImageSelecteur::ajoute_fichier(std::string s)
{
  if(s.size() == 0)
    return;

  journal.verbose("Ajout [%s]...", s.c_str());

  images.resize(images.size() + 1);
  set_fichier(images.size() - 1, s);
}



std::string ImageSelecteur::media_open_dialog()
{
  auto fs = OCVDemo::get_instance()->get_fileschema();
  auto schema = fs->get_schema("media-schema");
  auto mod = utils::model::Node::create_ram_node(schema);

  if(utils::mmi::NodeDialog::display_modal(mod))
    return "";

  int sel = mod.get_attribute_as_int("sel");
  if(sel == 0)
  {
    // image par défaut
    // TODO!
  }
  else if(sel == 1)
  {
    // Fichier
    return mod.get_attribute_as_string("file-schema/path");
  }
  else if(sel == 2)
  {
    // Caméra
    char bf[2];
    bf[0] = '0' + mod.get_attribute_as_int("cam-schema/idx");
    bf[1] = 0;
    return std::string(bf);
  }

  // URL
  return mod.get_attribute_as_string("url-schema/url");

  /*name = utils::langue.get_item("wiz0-name");
  title = utils::langue.get_item("wiz0-title");
  description = utils::langue.get_item("wiz0-desc");*/

# if 0
  std::string s, title = utils::langue.get_item("title-open");

  Gtk::FileChooserDialog dialog(title, Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  dialog.set_transient_for(*this);
  dialog.set_modal(true);
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  if(result != Gtk::RESPONSE_OK)
    return "";

  return dialog.get_filename();
# endif
}

void ImageSelecteur::on_b_add()
{
  journal.verbose("on b open...");
  ajoute_fichier(media_open_dialog());
  maj_actif();
}

void ImageSelecteur::on_b_open()
{
  journal.verbose("on b open...");
  set_fichier(this->csel, media_open_dialog());
  maj_actif();
}

void ImageSelecteur::on_b_del()
{
  journal.verbose("on b del().");
  if(csel != -1)
  {
    journal.verbose("del %d...", csel);
    images.erase(csel + images.begin(), 1 + csel + images.begin());
    if(csel >= (int) images.size())
      csel--;
    maj_mosaique();
  }
  maj_actif();
}

void ImageSelecteur::on_b_del_tout()
{
  journal.verbose("on b del tout().");
  images.clear();
  maj_mosaique();
  maj_actif();
}

void ImageSelecteur::on_b_maj()
{
  journal.verbose("on b maj.");
  ImageSelecteurRefresh evt;
  utils::CProvider<ImageSelecteurRefresh>::dispatch(evt);
}

void ImageSelecteur::raz()
{
  has_a_video = false;
  images.clear();
  csel = -1;
  maj_mosaique();
}



bool ImageSelecteur::on_b_pressed(GdkEventButton *event)
{
  unsigned int x = event->x, y = event->y;
  journal.verbose("bpress %d, %d", x, y);

  csel = -1;
  for(auto i = 0u; i < images.size(); i++)
  {
    auto &img = images[i];
    if((x > img.px) && (y > img.py)
        && (x < img.px + this->img_width)
        && (y < img.py + this->img_height))
    {
      csel = i;
      break;
    }
  }

  maj_actif();
  maj_selection();
  return true;
}

bool ImageSelecteur::on_b_released(GdkEventButton *event)
{
  journal.verbose("brel");

  evt_box.grab_focus();

  return true;
}

bool ImageSelecteur::on_k_released(GdkEventKey *event)
{
  if(csel == -1)
    return false;

  if(event->keyval == GDK_KEY_Delete)
  {
    this->on_b_del();
    return true;
  }
  else if(event->keyval == GDK_KEY_Down)
  {
    if(csel + ncols < images.size())
    {
      csel += ncols;
      maj_selection();
    }
    return true;
  }
  else if(event->keyval == GDK_KEY_Up)
  {
    journal.trace("Key up.");
    if(csel >= (int) ncols)
    {
      csel -= ncols;
      maj_selection();
    }
    else
      journal.trace("Refu: csel = %d, ncols = %d.", csel, ncols);
    return true;
  }
  else if(event->keyval == GDK_KEY_Left)
  {
    //if(images[csel].ix > 0)
    if(csel > 0)
    {
      csel--;
      maj_selection();
    }
    return true;
  }
  else if(event->keyval == GDK_KEY_Right)
  {
    if(/*(images[csel].ix + 1 < ncols) &&*/ (csel + 1 < (int) images.size()))
    {
      csel++;
      maj_selection();
    }
    return true;
  }
  return false;
}





