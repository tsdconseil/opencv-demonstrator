#include "mmi/gtkutil.hpp"
#include "mmi/stdview.hpp"
#include "mxml.hpp"

#include <gdkmm/color.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <stdio.h>

#ifdef WIN
# include <windows.h>
# include <stdio.h>
#else
# include <sys/stat.h> 
#endif

#define DEBOGUE_VUE_IMAGE(AA)

using namespace std;

#if 0
static uint32_t grayer(uint32_t val)
{
  /*int32_t diff = 32768 - val;
  diff = diff / 3;
  return 32768 - diff;*/
  return val / 3;
}
#endif

namespace utils
{
namespace mmi
{

using namespace utils;

Gtk::Window *mainWindow;



VideoView::VideoView(uint16_t dx, uint16_t dy, bool dim_from_parent)
  : Gtk::DrawingArea(), dispatcher(16)
{
  realise  = false;
  csx       = 1;
  csy       = 1;
  this->dim_from_parent = dim_from_parent;

  signal_realize().connect(sigc::mem_fun(*this, &VideoView::on_the_realisation));
  //signal_video_update.connect(sigc::mem_fun(*this, &VideoView::on_video_update));
  dispatcher.add_listener(this, &VideoView::on_event);

  change_dim(dx,dy);
  if(!dim_from_parent)
    set_size_request(csx,csy);
}

void VideoView::get_dim(uint16_t &sx, uint16_t &sy)
{
  sx = get_allocated_width();
  sy = get_allocated_height();
}



void VideoView::change_dim(uint16_t sx, uint16_t sy)
{
  if((csx != sx) || (csy != sy))
  {
    csx = sx;
    csy = sy;
    if(!dim_from_parent)
      set_size_request(csx,csy);

    image_surface = Cairo::ImageSurface::create(Cairo::Format::FORMAT_RGB24, sx, sy);
  }
}

void VideoView::update(void *img, uint16_t sx, uint16_t sy)
{
  if(!realise)
  {
    avertissement("video view update : non realise.");
    //return;
  }

  if(dispatcher.is_full())
  {
    avertissement("VideoView::maj: FIFO de sortie pleine, on va ignorer quelques trames...");
    //return;
    dispatcher.clear();
  }

  Trame t;

  if((this->csx == sx) && (this->csy == sy))
  {
    t.img = nullptr;
    memcpy(this->image_surface->get_data(), img, 4 * sx * sy);
  }
  else
  {
    t.img = malloc(sx*sy*4);
    memcpy(t.img, img, sx*sy*4);
  }

  t.sx  = sx;
  t.sy  = sy;
  dispatcher.on_event(t);
}

void VideoView::on_event(const Trame &t)
{
  uint16_t sx = t.sx, sy = t.sy;
  change_dim(sx,sy);
  if(t.img != nullptr)
  {
    memcpy(this->image_surface->get_data(), t.img, 4 * sx * sy);
    free(t.img);
  }
  queue_draw();
}



void VideoView::draw_all()
{
  GdkEventExpose evt;
  evt.area.x = 0;
  evt.area.y = 0;
  evt.area.width = get_allocation().get_width();
  evt.area.height = get_allocation().get_height();
  on_expose_event(&evt);
}

// Pas vraiment utilsé
bool VideoView::on_expose_event(GdkEventExpose* event)
{
  do_update_view();
  return true;
}

void VideoView::do_update_view()
{
  if(!realise)
    return;

  if(!cr)
  {
    auto wnd = get_window();
    if(!wnd)
    {
      realise = false;
      return;
    }
    cr = wnd->create_cairo_context();
  }
  //trace_verbeuse("dessine(%d,%d)", csx, csy);
  cr->set_source(this->image_surface, 0, 0);
  cr->rectangle (0.0, 0.0, csx, csy);
  cr->clip();
  cr->paint();
}

// C'est ici qu'on dessine !
bool VideoView::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  this->cr = cr;
  do_update_view();
  return true;
}

void VideoView::on_the_realisation()
{
  infos("Vue video : realise.");
  realise = true;
  do_update_view();
}


void set_theme(std::string theme_name)
{
# if 0
  std::string s = "include \"";
  s += theme_name + "/gtk-2.0/gtkrc\"\n";
  s += "gtk-theme-name = \"" + theme_name + "\"\n";

  files::save_txt_file("./theme-sel.txt", s);
  gtk_rc_add_default_file("./theme-sel.txt");
  GtkSettings* settings = gtk_settings_get_default();
  gtk_rc_reparse_all_for_settings(settings, true);
# endif
}

static std::vector<std::pair<Gtk::Widget *, Gtk::Window *> > wnd_list;

static uint32_t add_color(uint32_t corig,
                          float c,
                          uint8_t r, uint8_t g, uint8_t b)
{
  int cr, cg, cb;

  int ocb = (corig >> 16) & 0xff;
  int ocg = (corig >> 8) & 0xff;
  int ocr = corig & 0xff;

  cr = (uint32_t) (c * r);
  cg = (uint32_t) (c * g);
  cb = (uint32_t) (c * b);

  cr = ocr + cr;
  cg = ocg + cg;
  cb = ocb + cb;
  if(cr > 255)
    cr = 255;
  if(cb > 255)
    cb = 255;
  if(cg > 255)
    cg = 255;
  uint32_t col = (cb << 16) | (cg << 8) | cr;
  return col;
}

static uint32_t sub_color(uint32_t corig,
                          float c,
                          uint8_t r, uint8_t g, uint8_t b)
{
  int cr, cg, cb;

  int ocb = (corig >> 16) & 0xff;
  int ocg = (corig >> 8) & 0xff;
  int ocr = corig & 0xff;

  cr = (uint32_t) (c * (255-r));
  cg = (uint32_t) (c * (255-g));
  cb = (uint32_t) (c * (255-b));

  cr = ocr - cr;
  cg = ocg - cg;
  cb = ocb - cb;

  if(cr < 0)
    cr = 0;
  if(cg < 0)
      cg = 0;
  if(cb < 0)
      cb = 0;

  uint32_t col = (cb << 16) | (cg << 8) | cr;
  return col;
}

typedef uint32_t (*update_color_t)(uint32_t orig,
                                   float c,
                                   uint8_t r, uint8_t g, uint8_t b);


void AntiliasingDrawing::draw_line(uint32_t *img,
                                   uint32_t img_width,
                                   uint32_t img_height,
                                   int x1, int y1,
                                   int x2, int y2,
                                   GColor color,
                                   bool   mat_on_white)
{
  int x, y;
  update_color_t update_color = mat_on_white ? &sub_color : &add_color;


  if((fabs(y2 - y1) > (int) img_height)
      || (fabs(x2 - x1) > (int) img_width))
  {
    //anomaly("Y2 = %d, Y1 = %d.", y2, y1);
    //::Sleep(1000);
    return;
  }

  /* Ensure x2 >= x1 */
  if(x2 < x1)
  {
    /* Swap (x1,y1) and (x2,y2) */
    int tmp;
    tmp = y1;
    y1  = y2;
    y2  = tmp;
    tmp = x1;
    x1  = x2;
    x2  = tmp;
  }


  if(y2 == y1)
  {
    y = y1;
    if((y1 >= 0) && (y1 < (int) img_height))
    {
      for(x = x1; (x < x2) && (x < (int) img_width) && (x >= 0); x++)
      {
        img[x+y*img_width] = update_color(img[x+y*img_width],
                                          1.0, color.red, color.green, color.blue);
      }
    }
  }
  else if(x1 == x2)
  {
    x = x1;

    if(y2 < y1)
    {
      int tmp;
      tmp = y2;
      y2 = y1;
      y1 = tmp;
    }

    for(y = y1; (y >= 0) && (y < y2) && (y < (int) img_height); y++)
    {
      img[x+y*img_width] = update_color(img[x+y*img_width],
          1.0, color.red, color.green, color.blue);
    }
  }
  else if(fabs(y2 - y1) > (x2 - x1))
  {
    float gradient = fabs(((float) (x2 - x1)) / (y2 - y1));
    float interx = x1;
    if(y2 > y1)
    {
      for(int y = y1; (y <= y2) && (y < (int) img_height); y++)
      {
        x = (int) floor(interx);
        float intensity = interx - x;

        if((x < 0) || (y < 0) || (x >= (int) img_width))
          continue;

        img[x+y*img_width] = update_color(img[x+y*img_width],
                             1.0 - intensity, color.red, color.green, color.blue);
        img[x+y*img_width] = update_color(img[x+y*img_width],
                             intensity, color.red, color.green, color.blue);

        interx += gradient;
      }
    }
    /* y1 > y2 */
    else
    {
      interx = x2;
      for(int y = y2; (y <= y1) && (y < (int) img_height); y++)
      {
        x = (int) floor(interx);
        float intensity = interx - x;

        if((x < 0) || (y < 0) || (x >= (int) img_width))
          continue;

        img[x+y*img_width] = update_color(img[x+y*img_width],
                             1.0 - intensity, color.red, color.green, color.blue);
        img[x+y*img_width] = update_color(img[x+y*img_width],
                             intensity, color.red, color.green, color.blue);

        interx += gradient;
      }
    }

  }
  /* x2 - x1 >= y2 - y1 */
  else
  {
    float gradient = fabs(((float)(y2 - y1)) / (x2 - x1));
    float intery   = y1;
    if(y2 < y1)
      intery = y2;
    for(x = x1; (x <= x2) && (x < (int) img_width); x++)
    {
      y = (int) floor(intery);
      float intensity = intery - y;
      if((x < 0) || (y < 0) || (y >= (int) img_height))
        continue;
      img[x+y*img_width] = update_color(img[x+y*img_width],
                1.0 - intensity, color.red, color.green, color.blue);
      img[x+y*img_width] = update_color(img[x+y*img_width],
                intensity, color.red, color.green, color.blue);
      intery += gradient;
    }
  }






# if 0
  if(y2 > y1)
  {
    float gradient = 1.0 / (y2 - y1);
    float interx = 0;
    for(int y = y1; y <= y2; y++)
    {
      unsigned int *ptr = &(buffer[x1+y*2*x_width]);
      if((x1 > 0) && (x1 + 1 < 2*MAXX) && (y > 0) && (y < MAXY))
      {
        draw_point(ptr, 1.0 - interx);
        draw_point(ptr+1, interx);
      }
      interx += gradient;
    }
  }
  else if(y2 < y1)
  {
    float gradient = 1.0 / (y1 - y2);
    float interx = 0;
    for(int y = y1; y >= y2; y--)
    {
      unsigned int *ptr = &(buffer[x1+y*2*x_width]);
      if((x1 > 0) && (x1 + 1 < 2*MAXX) && (y > 0) && (y < MAXY))
      {
        draw_point(ptr, 1.0 - interx);
        draw_point(ptr+1, interx);
      }
      interx += gradient;
    }
  }
  else
  {
    if((x1 > 0) && (x1 + 1 < 2*MAXX) && (y1 > 0) && (y1 < MAXY))
    {
      unsigned int *ptr = &(buffer[x1+y1*2*x_width]);
      draw_point(ptr, 1.0);
    }
  }
# endif
}


void show_frame_window(Gtk::Widget *frame, std::string name)
{
  unsigned int i;
  Gtk::Window *wnd = nullptr;
  for(i = 0; i < wnd_list.size(); i++)
  {
    if(wnd_list[i].first == frame)
    {
      wnd = wnd_list[i].second;
      break;
    }
  }
  if(i == wnd_list.size())
  {
    wnd = new Gtk::Window();
    wnd->add(*frame);
  }
  wnd->set_title(name);
  wnd->show_all_children(true);
  wnd->show();
}


class SimpleDialog: public Gtk::Dialog
{
public:
  SimpleDialog(std::string title, bool modal, std::string dsc1, std::string dsc2, Gtk::BuiltinStockID sid);
  ~SimpleDialog();
  void on_b_ok();
  void on_b_cancel();
  int result;
  Gtk::HButtonBox hbbox;
  Gtk::HBox hbox;
  Gtk::VBox vbox;
  Gtk::Label label1, label2;
  Gtk::Image img;
  Gtk::Button bt;
  Gtk::Image *bim;
};

SimpleDialog::~SimpleDialog()
{
  delete bim;
}

SimpleDialog::SimpleDialog(std::string title,
                           bool modal,
                           std::string dsc1,
                           std::string dsc2,
                           Gtk::BuiltinStockID sid):
    Gtk::Dialog(title, modal),
    img(Gtk::StockID(sid), Gtk::IconSize(Gtk::ICON_SIZE_DIALOG))
{
  Gtk::Box *vb = get_vbox();

  bt.set_label(langue.get_item("close"));

  if(!appli_view_prm.use_decorations)
    set_decorated(false);

  if(sid == Gtk::Stock::DIALOG_ERROR)
  {
    if(appli_view_prm.img_cancel.size() > 0)
      bim = new Gtk::Image(appli_view_prm.img_cancel);
    else
      bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::CANCEL), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  }
  else if(sid == Gtk::Stock::DIALOG_WARNING)
  {
    if(appli_view_prm.img_cancel.size() > 0)
      bim = new Gtk::Image(appli_view_prm.img_cancel);
    else
      bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::CLOSE), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  }
  else if(sid == Gtk::Stock::DIALOG_INFO)
  {
    if(appli_view_prm.img_validate.size() > 0)
      bim = new Gtk::Image(appli_view_prm.img_validate);
    else
      bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::APPLY), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  }
  bt.set_image(*bim);
  bt.set_image_position(Gtk::POS_TOP);

  label1.set_markup("<b>" + dsc1 + "</b>\n");
  label2.set_markup(dsc2 + "\n");

  //Gtk::Image *img = (Gtk::StockID(Gtk::DIALOG_ERROR));
  hbox.pack_start(img);
  hbox.pack_start(vbox);
  vbox.pack_start(label1);
  vbox.pack_start(label2);

  hbbox.set_layout(Gtk::BUTTONBOX_END);
  hbbox.pack_end(bt);
  vb->pack_start(hbox);
  vb->pack_start(hbbox);

  set_position(Gtk::WIN_POS_CENTER);

  bt.signal_clicked().connect(sigc::mem_fun(*this, &SimpleDialog::on_b_ok));

  show_all_children(true);
}

void SimpleDialog::on_b_ok()
{
  result = 0;
  hide();
}

void SimpleDialog::on_b_cancel()
{
  result = 1;
  hide();
}

bool dialogs::check_dialog(std::string title,
                        std::string short_description,
                        std::string description)
{
  Gtk::MessageDialog dial(short_description, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK_CANCEL, true);
  dial.set_title(title);
  dial.set_secondary_text(description);
  DialogManager::setup_window(&dial);
  int result = dial.run();
  if(result == Gtk::RESPONSE_OK)
    return true;
  return false;
}

void dialogs::affiche_infos(std::string title, std::string short_description, std::string description)
{
  if(appli_view_prm.img_cancel.size() == 0)
  {
    Gtk::MessageDialog dial(short_description, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    dial.set_title(title);
    dial.set_secondary_text(description);
    dial.set_position(Gtk::WIN_POS_CENTER);
    DialogManager::setup_window(&dial);
    dial.run();
  }
  else
  {
    SimpleDialog dial(title,
                      true,
                      short_description,
                      description,
                      Gtk::Stock::DIALOG_INFO);
    DialogManager::setup_window(&dial);
    dial.run();
  }
}

void dialogs::affiche_infos_localisee(const std::string &id_locale)
{
  auto desc = utils::langue.get_localized(id_locale);

  Gtk::MessageDialog dial(desc.get_description(utils::model::Localized::current_language),
                          false,
                          Gtk::MESSAGE_INFO,
                          Gtk::BUTTONS_CLOSE,
                          true);
  dial.set_title(desc.get_localized());
  //dial.set_secondary_text(description);
  dial.set_position(Gtk::WIN_POS_CENTER);
  DialogManager::setup_window(&dial);
  dial.run();
}

void dialogs::affiche_avertissement_localise(const std::string &id_locale)
{
  auto desc = utils::langue.get_localized(id_locale);

  Gtk::MessageDialog dial(desc.get_description(utils::model::Localized::current_language),
                          false,
                          Gtk::MESSAGE_WARNING,
                          Gtk::BUTTONS_CLOSE,
                          true);
  dial.set_title(desc.get_localized());
  //dial.set_secondary_text(description);
  dial.set_position(Gtk::WIN_POS_CENTER);
  DialogManager::setup_window(&dial);
  dial.run();
}

void dialogs::affiche_erreur_localisee(const std::string &id_locale)
{
  auto desc = utils::langue.get_localized(id_locale);

  Gtk::MessageDialog dial(desc.get_description(utils::model::Localized::current_language),
                          false,
                          Gtk::MESSAGE_ERROR,
                          Gtk::BUTTONS_CLOSE,
                          true);
  dial.set_title(desc.get_localized());
  //dial.set_secondary_text(description);
  dial.set_position(Gtk::WIN_POS_CENTER);
  DialogManager::setup_window(&dial);
  dial.run();
}

void dialogs::affiche_erreur(std::string title,
                      std::string short_description,
                      std::string description)
{
  if(appli_view_prm.img_cancel.size() == 0)
  {
    Gtk::MessageDialog dial(short_description,
                            false,
                            Gtk::MESSAGE_ERROR,
                            Gtk::BUTTONS_CLOSE,
                            true);
    dial.set_title(title);
    dial.set_secondary_text(description);
    dial.set_position(Gtk::WIN_POS_CENTER);
    DialogManager::setup_window(&dial);
    dial.run();
  }
  else
  {
    SimpleDialog dial(title,
                      true,
                      short_description,
                      description,
                      Gtk::Stock::DIALOG_ERROR);
    DialogManager::setup_window(&dial);
    dial.run();
  }
}

void dialogs::affiche_avertissement(std::string title,
                        std::string short_description,
                        std::string description,
                        bool blocking)
{
  if(appli_view_prm.img_cancel.size() == 0)
  {
    Gtk::MessageDialog dial(short_description, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    dial.set_title(title);
    dial.set_secondary_text(description);
    dial.set_position(Gtk::WIN_POS_CENTER);
    DialogManager::setup_window(&dial);
    if(blocking)
      dial.run();
    else
      dial.show();
  }
  else
  {
    SimpleDialog dial(title,
                      true,
                      short_description,
                      description,
                      Gtk::Stock::DIALOG_WARNING);
    DialogManager::setup_window(&dial);
    if(blocking)
       dial.run();
     else
       dial.show();
  }
}

std::string dialogs::ouvrir_fichier_loc(const std::string &id_locale,
                    const std::string &filtre, const std::string &id_filtre,
                    std::string default_dir)
{
  auto desc = utils::langue.get_localized(id_locale);
  auto filt = utils::langue.get_localized(id_filtre);
  return ouvrir_fichier(desc.get_localized(),
                        filtre,
                        filt.get_localized(), "", default_dir);
}

std::string dialogs::enregistrer_fichier_loc(const std::string &id_locale,
    const std::string filtre, const std::string &id_filtre,
    const std::string &default_dir)
{
  auto desc = utils::langue.get_localized(id_locale);
  auto filt = utils::langue.get_localized(id_filtre);
  return enregistrer_fichier(desc.get_localized(),
                             filtre,
                             filt.get_localized(), "", default_dir);
}
  

std::string dialogs::enregistrer_fichier(std::string title, std::string filter, std::string filter_name,
                              std::string default_name, std::string default_dir)
{
    Gtk::FileChooserDialog dialog(title, Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    if(mainWindow != nullptr)
      dialog.set_transient_for(*mainWindow);
    dialog.set_modal(true);
    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:

    Glib::RefPtr<Gtk::FileFilter> filter_xml = Gtk::FileFilter::create();
    filter_xml->set_name(filter_name);
    filter_xml->add_mime_type(std::string("*") + filter);
    dialog.add_filter(filter_xml);

    if(default_dir.size() > 0)
      dialog.set_current_folder(default_dir);

    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          std::string filename = dialog.get_filename();

          std::string ext = utils::files::get_extension(filename);

          if(ext.size() == 0)
          {
            //auto ext2 = utils::files::get_extension(ext);
            infos("Pas d'extension precisee, ajout de %s", filter.c_str());
            if((filter.size() > 0) && (filter[0] == '.'))
              filename += filter;
          }

          dialog.hide();
          return filename;
        }
        case(Gtk::RESPONSE_CANCEL):
        {
          return "";
        }
        default:
        {
          return "";
        }
      }
    return "";
}

std::string dialogs::selection_dossier(const std::string &titre)
{
  Gtk::FileChooserDialog dialog(titre, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  if(mainWindow != nullptr)
    dialog.set_transient_for(*mainWindow);
  dialog.set_modal(true);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  dialog.set_select_multiple(false);
  dialog.set_show_hidden(false);

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result)
    {
      case(Gtk::RESPONSE_OK):
      {
        std::string filename = dialog.get_filename();
        dialog.hide();
        return filename;
      }
      case(Gtk::RESPONSE_CANCEL):
      {
        return "";
      }
      default:
      {
        return "";
      }
    }
  return "";
}

std::string dialogs::ouvrir_fichier(std::string title, std::string filter, std::string filter_name,
                              std::string default_name, std::string default_dir)
{
    Gtk::FileChooserDialog dialog(title, Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    if(mainWindow != nullptr)
      dialog.set_transient_for(*mainWindow);
    dialog.set_modal(true);
    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:

    //Gtk::FileFilter filter_xml;
    Glib::RefPtr<Gtk::FileFilter> filter_xml = Gtk::FileFilter::create();
    filter_xml->set_name(filter_name);
    //filter_xml.add_mime_type(filter);
    filter_xml->add_pattern(filter);
    dialog.add_filter(filter_xml);

    Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);
    
    dialog.set_select_multiple(false);
    dialog.set_show_hidden(false);
    if(default_dir.size() > 0)
      dialog.set_current_folder(default_dir);

    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          std::string filename = dialog.get_filename();
          dialog.hide();
          return filename;
        }
        case(Gtk::RESPONSE_CANCEL):
        {
          return "";
        }
        default:
        {
          return "";
        }
      }
    return "";
}

std::string dialogs::nouveau_fichier(std::string title, std::string filter, std::string filter_name,
                              std::string default_name, std::string default_dir)
{
    Gtk::FileChooserDialog dialog(title, Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    if(mainWindow != nullptr)
      dialog.set_transient_for(*mainWindow);
    dialog.set_modal(true);
    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:

    Glib::RefPtr<Gtk::FileFilter> filter_xml = Gtk::FileFilter::create();
    filter_xml->set_name(filter_name);
    filter_xml->add_mime_type(filter);
    dialog.add_filter(filter_xml);

    Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    //dialog.set_filename(default_name + ".xml");
    
    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          std::string filename = dialog.get_filename();
          std::string ext = files::get_extension(filename);
          if(ext.compare("") == 0)
              filename += ".xml";
          dialog.hide();
          return filename;
        }
        case(Gtk::RESPONSE_CANCEL):
        {
          return "";
        }
        default:
        {
          return "";
        }
      }
    return "";
}

Glib::ustring request_user_string(Gtk::Window *parent,
                                         Glib::ustring mainMessage, 
                                         Glib::ustring subMessage)
{
  Gtk::MessageDialog dialog(*parent, mainMessage,
        false /* use_markup */, Gtk::MESSAGE_QUESTION,
        Gtk::BUTTONS_OK_CANCEL);
  dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  dialog.set_secondary_text(subMessage);
  Gtk::Box *vb = dialog.get_vbox();
  Gtk::Entry entry;
  entry.set_text("");
  vb->pack_start(entry);
  vb->show_all();
  int result = dialog.run();

  //Handle the response:
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      std::cout << "OK clicked." << std::endl;
      return entry.get_text();
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      return "";
    }
    default:
    {
      std::cout << "Unexpected button clicked." << std::endl;
      return "";
    }
  }
}

#if 0
void CException::affiche_erreur() const
{
    std::cout << "Fatal error: " << cat << std::endl << desc << std::endl;
    Gtk::MessageDialog dial(cat, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
    dial.set_title(cat);
    dial.set_secondary_text(desc);
    dial.run();
}
#endif






JComboBox::JComboBox()
{
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  set_model(m_refTreeModel);
  //pack_start(m_Columns.m_col_id);
  pack_start(m_Columns.m_col_name, Gtk::PACK_SHRINK);
}

void JComboBox::remove_all()
{
    keys.clear();
    m_refTreeModel->clear();
    //clear();
}

void JComboBox::add_key(std::string key)
{
    keys.push_back(key);
    Gtk::TreeModel::Row row = *(m_refTreeModel->append());
    row[m_Columns.m_col_name] = key;
}

void JComboBox::set_current_key(std::string s)
{
    for(unsigned int i = 0; i < keys.size(); i++)
    {
        if(keys[i].compare(s) == 0)
        {
            set_active(i);
            return;
        }
    }
}

std::string JComboBox::get_current_key()
{
  Gtk::TreeModel::iterator iter = get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    if(row)
    {
      Glib::ustring res = row[m_Columns.m_col_name];
      return res;
    }
  }
  return "";
}

void JFrame::set_label(const Glib::ustring &s)
{
    Gtk::Label *old = lab;
    lab = new Gtk::Label();
    if(appli_view_prm.inverted_colors)
      lab->set_label(std::string("<span color=\"#00ff00\">") + s + "</span>");
    else
      lab->set_label(std::string("<span color=\"#006000\">") + s + "</span>");
    lab->set_use_markup(true);
    set_label_widget(*lab);
    lab->show();
    if(old != nullptr)
        delete old;
}

JFrame::JFrame(std::string label)
{
    lab = nullptr;
    set_border_width(20);
    set_label(label);
}




GtkKey::GtkKey(unsigned short size_x, unsigned short size_y, std::string s, bool toggle)
{
  text = s;
  this->size_x = size_x;
  this->size_y = size_y;
  this->toggle = toggle;
  sensitive = true;
  realized = false;
  clicking = false;
  set_size_request(size_x,size_y);
  signal_realize().connect(sigc::mem_fun(*this, &GtkKey::on_the_realisation));
  signal_button_press_event().connect(sigc::mem_fun(*this, &GtkKey::on_mouse));
  signal_button_release_event().connect( sigc::mem_fun( *this, &GtkKey::on_mouse));
  add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
}



bool GtkKey::on_mouse(GdkEventButton *event)
{
  //infos("mouse event.");

  if(!sensitive)
    return true;

  bool old_clicking = clicking;
  if(event->type == GDK_BUTTON_PRESS)
  {
    if(!toggle)
      this->clicking = true;
    else
      clicking = !clicking;
  }
  else if(event->type == GDK_BUTTON_RELEASE)
  {
    if(!toggle)
      this->clicking = false;
  }
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
  {
      Gdk::Rectangle r(0, 0, get_allocation().get_width(),
              get_allocation().get_height());
      win->invalidate_rect(r, false);
  }

  if((((old_clicking != clicking) && !clicking)) || (this->text.compare("SHIFT") == 0))
  {
    KeyChangeEvent kce;
    kce.active = clicking;
    kce.key = this->text;
    kce.source = this;
    infos("dispatch kce..");
    CProvider<KeyChangeEvent>::dispatch(kce);
  }

  return true;
}



void GtkKey::on_the_realisation()
{
  wnd = get_window();
  gc = wnd->create_cairo_context();
  ctx = wnd->create_cairo_context();
  lay = Pango::Layout::create(ctx);
  realized = true;
  update_view();
}



bool GtkKey::on_expose_event(GdkEventExpose* event)
{
  //infos("expose event.");
  update_view();
  return true;
}



void GtkKey::set_sensitive(bool sensitive)
{
  if(this->sensitive != sensitive)
  {
    this->sensitive = sensitive;
    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win)
    {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
  }
}

void GtkKey::update_view()
{
  if(!realized)
    return;


  Gdk::Color white, black;

  white.set_red(65535);
  white.set_blue(65535);
  white.set_green(65535);

  black.set_red(0);
  black.set_blue(0);
  black.set_green(0);

  float x = 0, y = 0,
        radius = size_y / 4;
  float degrees = 3.1415926 / 180.0;

  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
  {
    Gtk::Allocation allocation = get_allocation();
    int width = allocation.get_width();
    int height = allocation.get_height();

    //infos("width = %d, height = %d.", width, height);

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();


    cr->set_line_width(1.5);
    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle(0, 0,
            width, height);
    cr->set_source_rgb(0, 0, 0);
    cr->clip();


    float foreground;
    float background;

    float coef;
    if(sensitive)
      coef = 1.0;
    else
      coef = 0.25;

    // Fond noir
    if(appli_view_prm.inverted_colors)
    {
      background = 0.0;
      foreground = coef;
    }
    else
    {
      background = 1.0;
      foreground = 1.0 - coef;
    }

    cr->set_source_rgb(foreground, foreground, foreground);
    cr->arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cr->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cr->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cr->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cr->close_path();
    cr->stroke();

    //cr->set_source_rgb(1, 1, 1);

    if(clicking)
    {
      //cr->set_source_rgb(0.7, 0.9, 0.7);
      cr->set_source_rgb(0.7 * foreground + 0.3 * background, 0.9 * foreground + 0.1 * background, 0.7 * foreground + 0.3 * background);
      x = x + width / 10;
      y = y + height / 10;
      width = (width * 8) / 10;
      height = (height * 8) / 10;
      radius = (radius * 8) / 10;
      cr->arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
      cr->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
      cr->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
      cr->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
      cr->close_path();
      cr->fill();
      cr->set_source_rgb(background, background, background);
    }


    lay->set_text(text);
    lay->update_from_cairo_context(cr);  // gets cairo cursor position

    int tx, ty;
    lay->get_pixel_size(tx, ty);

    cr->move_to((size_x - tx) / 2, (size_y - ty) / 2);

    lay->add_to_cairo_context(cr);       // adds text to cairos stack of stuff to be drawn
    cr->stroke();                        // tells Cairo to render it's stack
    //infos("Done drawing gtkkey.");
  }
}

//GtkKeyboard *GtkKeyboard::instance = nullptr;

#if 0
GtkKeyboard *GtkKeyboard::get_instance()
{
  if(instance == nullptr)
    instance = new GtkKeyboard();
  return instance;
}
#endif

#if 0
GtkKey *GtkKeyboard::add_key(std::string s)
{
  GtkKey *key;
  key = new GtkKey(kw, kw, s);
  fixed.put(*key, cx, cy);
  //infos("Add key [%s] @ %d, %d.", s.c_str(), cx, cy);
  keys.push_back(key);
  cx += (kw + 5);
  key->add_listener(this);
  return key;
}
#endif

GtkKey *VirtualKeyboard::add_key(std::string s)
{
  GtkKey *key;
  key = new GtkKey(kw, kw, s);
  fixed.put(*key, cx, cy);
  //infos("Add key [%s] @ %d, %d.", s.c_str(), cx, cy);
  keys.push_back(key);
  cx += (kw + 5);
  key->add_listener(this);
  return key;
}

std::string GtkKey::get_text()
{
  return text;
}
#if 0
void GtkKeyboard::update_keyboard()
{
  unsigned int i;
  if(maj_on)
  {
    char buf[2];
    buf[1] = 0;
    buf[0] = 0xc9;
    keys[1]->set_text(Util::latin_to_utf8(buf));
    buf[0] = 0xc8;
    keys[6]->set_text(Util::latin_to_utf8(buf));
    buf[0] = 0xc7;
    keys[8]->set_text(Util::latin_to_utf8(buf));
    buf[0] = 0xc0;
    keys[9]->set_text(Util::latin_to_utf8(buf));


    i = 15;
    keys[i++]->set_text('A');
    keys[i++]->set_text('Z');
    keys[i++]->set_text('E');
    keys[i++]->set_text('R');
    keys[i++]->set_text('T');
    keys[i++]->set_text('Y');
    keys[i++]->set_text('U');
    keys[i++]->set_text('I');
    keys[i++]->set_text('O');
    keys[i++]->set_text('P');

    i = 26;
    keys[i++]->set_text('Q');
    keys[i++]->set_text('S');
    keys[i++]->set_text('D');
    keys[i++]->set_text('F');
    keys[i++]->set_text('G');
    keys[i++]->set_text('H');
    keys[i++]->set_text('J');
    keys[i++]->set_text('K');
    keys[i++]->set_text('L');
    keys[i++]->set_text('M');

    keys[i++]->set_text('W');
    keys[i++]->set_text('X');
    keys[i++]->set_text('C');
    keys[i++]->set_text('V');
    keys[i++]->set_text('B');
    keys[i++]->set_text('N');
  }
  else
  {
    char buf[3];
    buf[0] = 0xe9;
    buf[1] = 0;
    keys[1]->set_text(Util::latin_to_utf8(buf));
    buf[0] = 0xe8;
    keys[6]->set_text(Util::latin_to_utf8(buf));
    buf[0] = 0xe7;
    keys[8]->set_text(Util::latin_to_utf8(buf));
    buf[0] = 0xe0;
    keys[9]->set_text(Util::latin_to_utf8(buf));

    i = 15;
    keys[i++]->set_text('a');
    keys[i++]->set_text('z');
    keys[i++]->set_text('e');
    keys[i++]->set_text('r');
    keys[i++]->set_text('t');
    keys[i++]->set_text('y');
    keys[i++]->set_text('u');
    keys[i++]->set_text('i');
    keys[i++]->set_text('o');
    keys[i++]->set_text('p');

    i = 26;
    keys[i++]->set_text('q');
    keys[i++]->set_text('s');
    keys[i++]->set_text('d');
    keys[i++]->set_text('f');
    keys[i++]->set_text('g');
    keys[i++]->set_text('h');
    keys[i++]->set_text('j');
    keys[i++]->set_text('k');
    keys[i++]->set_text('l');
    keys[i++]->set_text('m');

    keys[i++]->set_text('w');
    keys[i++]->set_text('x');
    keys[i++]->set_text('c');
    keys[i++]->set_text('v');
    keys[i++]->set_text('b');
    keys[i++]->set_text('n');
  }
}

void GtkKeyboard::on_event(const KeyChangeEvent &kce)
{
  if(kce.key.compare("SHIFT") == 0)
  {
    infos("shift detected.");
    if(kce.active)
    {
      maj_on = true;

#     if 0
      char buf[2];
      buf[1] = 0;
      buf[0] = 0xc9;
      keys[1]->set_text(Util::latin_to_utf8(buf));
      buf[0] = 0xc8;
      keys[6]->set_text(Util::latin_to_utf8(buf));
      buf[0] = 0xc7;
      keys[8]->set_text(Util::latin_to_utf8(buf));
      buf[0] = 0xc0;
      keys[9]->set_text(Util::latin_to_utf8(buf));


      i = 15;
      keys[i++]->set_text('A');
      keys[i++]->set_text('Z');
      keys[i++]->set_text('E');
      keys[i++]->set_text('R');
      keys[i++]->set_text('T');
      keys[i++]->set_text('Y');
      keys[i++]->set_text('U');
      keys[i++]->set_text('I');
      keys[i++]->set_text('O');
      keys[i++]->set_text('P');

      i = 26;
      keys[i++]->set_text('Q');
      keys[i++]->set_text('S');
      keys[i++]->set_text('D');
      keys[i++]->set_text('F');
      keys[i++]->set_text('G');
      keys[i++]->set_text('H');
      keys[i++]->set_text('J');
      keys[i++]->set_text('K');
      keys[i++]->set_text('L');
      keys[i++]->set_text('M');

      keys[i++]->set_text('W');
      keys[i++]->set_text('X');
      keys[i++]->set_text('C');
      keys[i++]->set_text('V');
      keys[i++]->set_text('B');
      keys[i++]->set_text('N');
#     endif
    }
    else
    {
      maj_on = false;

#     if 0
      char buf[3];
      buf[0] = 0xe9;
      buf[1] = 0;
      keys[1]->set_text(Util::latin_to_utf8(buf));
      buf[0] = 0xe8;
      keys[6]->set_text(Util::latin_to_utf8(buf));
      buf[0] = 0xe7;
      keys[8]->set_text(Util::latin_to_utf8(buf));
      buf[0] = 0xe0;
      keys[9]->set_text(Util::latin_to_utf8(buf));

      i = 15;
      keys[i++]->set_text('a');
      keys[i++]->set_text('z');
      keys[i++]->set_text('e');
      keys[i++]->set_text('r');
      keys[i++]->set_text('t');
      keys[i++]->set_text('y');
      keys[i++]->set_text('u');
      keys[i++]->set_text('i');
      keys[i++]->set_text('o');
      keys[i++]->set_text('p');

      i = 26;
      keys[i++]->set_text('q');
      keys[i++]->set_text('s');
      keys[i++]->set_text('d');
      keys[i++]->set_text('f');
      keys[i++]->set_text('g');
      keys[i++]->set_text('h');
      keys[i++]->set_text('j');
      keys[i++]->set_text('k');
      keys[i++]->set_text('l');
      keys[i++]->set_text('m');

      keys[i++]->set_text('w');
      keys[i++]->set_text('x');
      keys[i++]->set_text('c');
      keys[i++]->set_text('v');
      keys[i++]->set_text('b');
      keys[i++]->set_text('n');
#     endif
    }
    update_keyboard();
    //if(target_window != nullptr)
    //target_window->present();
  }
  else
  {
    /*Gdk::Event event;
    event.*/
    
    //if(target_window != nullptr)
    //target_window->present();
    const char *cs = kce.key.c_str();



    if(target_window != nullptr)
    {
    Gtk::Widget *widget = target_window->get_focus();

    if(widget != nullptr)
    {
      GdkEvent *ge = new GdkEvent();
      GdkEventKey gek;
      gek.type = GDK_KEY_PRESS;
      gek.window = Gdk::Screen::get_default()->get_root_window()->gobj();//nullptr;//this->target_window;
      gek.send_event = true;
      gek.state = 0;
      gek.keyval = cs[0];

      if(cs[1] != 0)
      {
        //gek.keyval = (((unsigned short) cs[0]) << 8) | cs[1];
        gek.keyval = (((unsigned short) cs[1]) << 8) | cs[0];
      }

      gek.length = 0;
      gek.is_modifier = 0;


      if(kce.key.compare(langue.get_item("key-space")) == 0)
      {
        gek.keyval = ' ';
      }
      else if(kce.key.compare(langue.get_item("key-tab")) == 0)
      {
        gek.keyval = 0xff09;//GDK_Tab;
        //gek.state |= GDK_CONTROL_MASK;
        gek.is_modifier = 0;//1;
        //gek.hardware_keycode = 0x09;
      }

      char buf[2];
      buf[1] = 0;

      if(kce.key.compare(langue.get_item("key-del")) == 0)
      {
        gek.keyval = 0xff08;
      }

      buf[0] = 0xe9;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe9;
      }
      buf[0] = 0xe8;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe8;
      }
      buf[0] = 0xe0;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe0;
      }
      buf[0] = 0xe7;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe7;
      }


      buf[0] = 0xc9;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc9;
      }
      buf[0] = 0xc8;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc8;
      }
      buf[0] = 0xc7;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc7;
      }
      buf[0] = 0xc0;
      if(kce.key.compare(Util::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc0;
      }

      gek.hardware_keycode = (gek.keyval & 0xff);

      gek.group = 0;


      ge->key = gek;
      if(target_window != nullptr)
        target_window->event(ge);

      infos("Key event sent = '%c'.", (char) gek.keyval);
    }
  }
  }
}

bool GtkKeyboard::on_key(GdkEventKey *event)
{
  infos("EVENT KEY: state=%x, keyval=%x, length=%x, ismod=%x, hw=%x.", event->state, event->keyval, event->length, event->is_modifier, event->hardware_keycode);
  return true;
}
#endif

void VirtualKeyboard::update_keyboard()
{
  unsigned int i;
  if(maj_on)
  {
    char buf[2];
    buf[1] = 0;
    buf[0] = 0xc9;
    keys[1]->set_text(utils::str::latin_to_utf8(buf));
    buf[0] = 0xc8;
    keys[6]->set_text(utils::str::latin_to_utf8(buf));
    buf[0] = 0xc7;
    keys[8]->set_text(utils::str::latin_to_utf8(buf));
    buf[0] = 0xc0;
    keys[9]->set_text(utils::str::latin_to_utf8(buf));


    i = 15;
    keys[i++]->set_text('A');
    keys[i++]->set_text('Z');
    keys[i++]->set_text('E');
    keys[i++]->set_text('R');
    keys[i++]->set_text('T');
    keys[i++]->set_text('Y');
    keys[i++]->set_text('U');
    keys[i++]->set_text('I');
    keys[i++]->set_text('O');
    keys[i++]->set_text('P');

    i = 26;
    keys[i++]->set_text('Q');
    keys[i++]->set_text('S');
    keys[i++]->set_text('D');
    keys[i++]->set_text('F');
    keys[i++]->set_text('G');
    keys[i++]->set_text('H');
    keys[i++]->set_text('J');
    keys[i++]->set_text('K');
    keys[i++]->set_text('L');
    keys[i++]->set_text('M');

    keys[i++]->set_text('W');
    keys[i++]->set_text('X');
    keys[i++]->set_text('C');
    keys[i++]->set_text('V');
    keys[i++]->set_text('B');
    keys[i++]->set_text('N');
  }
  else
  {
    char buf[3];
    buf[0] = 0xe9;
    buf[1] = 0;
    keys[1]->set_text(utils::str::latin_to_utf8(buf));
    buf[0] = 0xe8;
    keys[6]->set_text(utils::str::latin_to_utf8(buf));
    buf[0] = 0xe7;
    keys[8]->set_text(utils::str::latin_to_utf8(buf));
    buf[0] = 0xe0;
    keys[9]->set_text(utils::str::latin_to_utf8(buf));

    i = 15;
    keys[i++]->set_text('a');
    keys[i++]->set_text('z');
    keys[i++]->set_text('e');
    keys[i++]->set_text('r');
    keys[i++]->set_text('t');
    keys[i++]->set_text('y');
    keys[i++]->set_text('u');
    keys[i++]->set_text('i');
    keys[i++]->set_text('o');
    keys[i++]->set_text('p');

    i = 26;
    keys[i++]->set_text('q');
    keys[i++]->set_text('s');
    keys[i++]->set_text('d');
    keys[i++]->set_text('f');
    keys[i++]->set_text('g');
    keys[i++]->set_text('h');
    keys[i++]->set_text('j');
    keys[i++]->set_text('k');
    keys[i++]->set_text('l');
    keys[i++]->set_text('m');

    keys[i++]->set_text('w');
    keys[i++]->set_text('x');
    keys[i++]->set_text('c');
    keys[i++]->set_text('v');
    keys[i++]->set_text('b');
    keys[i++]->set_text('n');
  }
}

void VirtualKeyboard::on_event(const KeyChangeEvent &kce)
{
  infos("kc detected.");
  if(kce.key.compare("SHIFT") == 0)
  {
    infos("shift detected.");
    if(kce.active)
    {
      maj_on = true;

#     if 0
      char buf[2];
      buf[1] = 0;
      buf[0] = 0xc9;
      keys[1]->set_text(utils::str::latin_to_utf8(buf));
      buf[0] = 0xc8;
      keys[6]->set_text(utils::str::latin_to_utf8(buf));
      buf[0] = 0xc7;
      keys[8]->set_text(utils::str::latin_to_utf8(buf));
      buf[0] = 0xc0;
      keys[9]->set_text(utils::str::latin_to_utf8(buf));


      i = 15;
      keys[i++]->set_text('A');
      keys[i++]->set_text('Z');
      keys[i++]->set_text('E');
      keys[i++]->set_text('R');
      keys[i++]->set_text('T');
      keys[i++]->set_text('Y');
      keys[i++]->set_text('U');
      keys[i++]->set_text('I');
      keys[i++]->set_text('O');
      keys[i++]->set_text('P');

      i = 26;
      keys[i++]->set_text('Q');
      keys[i++]->set_text('S');
      keys[i++]->set_text('D');
      keys[i++]->set_text('F');
      keys[i++]->set_text('G');
      keys[i++]->set_text('H');
      keys[i++]->set_text('J');
      keys[i++]->set_text('K');
      keys[i++]->set_text('L');
      keys[i++]->set_text('M');

      keys[i++]->set_text('W');
      keys[i++]->set_text('X');
      keys[i++]->set_text('C');
      keys[i++]->set_text('V');
      keys[i++]->set_text('B');
      keys[i++]->set_text('N');
#     endif
    }
    else
    {
      maj_on = false;

#     if 0
      char buf[3];
      buf[0] = 0xe9;
      buf[1] = 0;
      keys[1]->set_text(utils::str::latin_to_utf8(buf));
      buf[0] = 0xe8;
      keys[6]->set_text(utils::str::latin_to_utf8(buf));
      buf[0] = 0xe7;
      keys[8]->set_text(utils::str::latin_to_utf8(buf));
      buf[0] = 0xe0;
      keys[9]->set_text(utils::str::latin_to_utf8(buf));

      i = 15;
      keys[i++]->set_text('a');
      keys[i++]->set_text('z');
      keys[i++]->set_text('e');
      keys[i++]->set_text('r');
      keys[i++]->set_text('t');
      keys[i++]->set_text('y');
      keys[i++]->set_text('u');
      keys[i++]->set_text('i');
      keys[i++]->set_text('o');
      keys[i++]->set_text('p');

      i = 26;
      keys[i++]->set_text('q');
      keys[i++]->set_text('s');
      keys[i++]->set_text('d');
      keys[i++]->set_text('f');
      keys[i++]->set_text('g');
      keys[i++]->set_text('h');
      keys[i++]->set_text('j');
      keys[i++]->set_text('k');
      keys[i++]->set_text('l');
      keys[i++]->set_text('m');

      keys[i++]->set_text('w');
      keys[i++]->set_text('x');
      keys[i++]->set_text('c');
      keys[i++]->set_text('v');
      keys[i++]->set_text('b');
      keys[i++]->set_text('n');
#     endif
    }
    update_keyboard();
    //if(target_window != nullptr)
    //target_window->present();
  }
  else
  {
    /*Gdk::Event event;
    event.*/

    //if(target_window != nullptr)
    //target_window->present();
    const char *cs = kce.key.c_str();



    if(target_window != nullptr)
    {
    Gtk::Widget *widget = target_window->get_focus();

    if(widget != nullptr)
    {
      GdkEvent *ge = new GdkEvent();
      GdkEventKey gek;
      gek.type = GDK_KEY_PRESS;
      gek.window = Gdk::Screen::get_default()->get_root_window()->gobj();//nullptr;//this->target_window;
      gek.send_event = true;
      gek.state = 0;
      gek.keyval = cs[0];

      if(cs[1] != 0)
      {
        //gek.keyval = (((unsigned short) cs[0]) << 8) | cs[1];
        gek.keyval = (((unsigned short) cs[1]) << 8) | cs[0];
      }

      gek.length = 0;
      gek.is_modifier = 0;


      if(kce.key.compare(langue.get_item("key-space")) == 0)
      {
        gek.keyval = ' ';
      }
      else if(kce.key.compare(langue.get_item("key-tab")) == 0)
      {
        gek.keyval = 0xff09;//GDK_Tab;
        //gek.state |= GDK_CONTROL_MASK;
        gek.is_modifier = 0;//1;
        //gek.hardware_keycode = 0x09;
      }

      char buf[2];
      buf[1] = 0;

      if(kce.key.compare(langue.get_item("key-del")) == 0)
      {
        gek.keyval = 0xff08;
      }

      buf[0] = 0xe9;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe9;
      }
      buf[0] = 0xe8;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe8;
      }
      buf[0] = 0xe0;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe0;
      }
      buf[0] = 0xe7;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xe7;
      }


      buf[0] = 0xc9;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc9;
      }
      buf[0] = 0xc8;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc8;
      }
      buf[0] = 0xc7;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc7;
      }
      buf[0] = 0xc0;
      if(kce.key.compare(utils::str::latin_to_utf8(buf)) == 0)
      {
        gek.keyval = 0xc0;
      }

      gek.hardware_keycode = (gek.keyval & 0xff);

      gek.group = 0;


      ge->key = gek;
      if(target_window != nullptr)
        target_window->event(ge);

      infos("Key event sent = '%c'.", (char) gek.keyval);
    }
  }
  }
}

bool VirtualKeyboard::on_key(GdkEventKey *event)
{
  infos("EVENT KEY: state=%x, keyval=%x, length=%x, ismod=%x, hw=%x.", event->state, event->keyval, event->length, event->is_modifier, event->hardware_keycode);
  return true;
}

void GtkKey::set_text(char c)
{
  char tmp[2];
  tmp[0] = c;
  tmp[1] = 0;
  std::string s(tmp);
  set_text(tmp);
}

void GtkKey::set_text(std::string s)
{
  this->text = s;
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
  {
    Gdk::Rectangle r(0, 0, get_allocation().get_width(),
		     get_allocation().get_height());
    win->invalidate_rect(r, false);
  }
}

#if 0
void GtkKeyboard::display(Gtk::Window *target_window)
{
  infos("Display(target_window).");
  currently_active = true;
  this->target_window = target_window;
  show_all_children(true);
  show();
}

void GtkKeyboard::close()
{
  currently_active = false;
  target_window = nullptr;
  hide();
}


bool GtkKeyboard::is_currently_active()
{
  return currently_active;
}

void GtkKeyboard::set_valid_chars(std::vector<std::string> &vchars)
{
  infos("update vchars..");
  bool all_maj = true, has_maj = false;

  for(unsigned int i = 0; i < vchars.size(); i++)
  {
    char c = (vchars[i])[0];
    if((c >= 'a') && (c <= 'z'))
    {
      all_maj = false;
      break;
    }
    if((c >= 'A') && (c <= 'Z'))
    {
      has_maj = true;
      break;
    }
  }
  if(all_maj & has_maj)
  {
    KeyChangeEvent kce;
    kce.key = "SHIFT";
    kce.active = true;
    on_event(kce);
  }

  for(unsigned int i = 0; i < keys.size(); i++)
  {
    bool found = false;
    std::string ks = keys[i]->get_text();

    if(ks.compare(langue.get_item("key-space")) == 0)
      ks = ' ';
    if(ks.compare(langue.get_item("key-tab")) == 0)
      continue;
    if(ks.compare(langue.get_item("key-del")) == 0)
      continue;
    if(ks.compare("SHIFT") == 0)
      continue;

    for(unsigned int j = 0; j < vchars.size(); j++)
    {
      if(ks.compare(vchars[j]) == 0)
      {
        found = true;
        break;
      }
    }
    keys[i]->set_sensitive(found);
  }
}
#endif

void VirtualKeyboard::set_valid_chars(const std::vector<std::string> &vchars)
{
  infos("update vchars..");
  bool all_maj = true, has_maj = false;

  for(unsigned int i = 0; i < vchars.size(); i++)
  {
    char c = (vchars[i])[0];
    if((c >= 'a') && (c <= 'z'))
    {
      all_maj = false;
      break;
    }
    if((c >= 'A') && (c <= 'Z'))
    {
      has_maj = true;
      break;
    }
  }
  if(all_maj & has_maj)
  {
    KeyChangeEvent kce;
    kce.key = "SHIFT";
    kce.active = true;
    on_event(kce);
  }

  for(unsigned int i = 0; i < keys.size(); i++)
  {
    bool found = false;
    std::string ks = keys[i]->get_text();

    if(ks.compare(langue.get_item("key-space")) == 0)
      ks = ' ';
    if(ks.compare(langue.get_item("key-tab")) == 0)
      continue;
    if(ks.compare(langue.get_item("key-del")) == 0)
      continue;
    if(ks.compare("SHIFT") == 0)
      continue;

    for(unsigned int j = 0; j < vchars.size(); j++)
    {
      if(ks.compare(vchars[j]) == 0)
      {
        found = true;
        break;
      }
    }
    keys[i]->set_sensitive(found);
  }
}
#if 0
GtkKeyboard::GtkKeyboard()
{
  GtkKey *key;

  this->target_window = nullptr;//target_window;
  currently_active = false;
  maj_on = false;

  set_decorated(false);
  add(frame);
  frame.add(hbox);
  kw = 40;

  int ox = 5, oy = 5;

  cx = ox + kw/2;
  cy = oy;
  add_key(utils::str::latin_to_utf8("&"));
  add_key(utils::str::latin_to_utf8("�"));//langue.getItem("e-aigu"));
  add_key("\"");
  add_key("'");
  add_key("(");
  add_key("-");
  add_key(utils::str::latin_to_utf8("�"));//langue.getItem("e-grave"));
  add_key("_");
  add_key(utils::str::latin_to_utf8("�"));//langue.getItem("cedille"));
  add_key(utils::str::latin_to_utf8("�"));//(langue.getItem("a-grave"));
  add_key(")");
  add_key("=");
  add_key(".");

  key = new GtkKey(3*kw/2, kw, langue.get_item("key-del"));
  fixed.put(*key, cx, cy);
  keys.push_back(key);
  key->add_listener(this);


  oy += kw + 5;


  cx = ox;
  cy = oy;
  key = new GtkKey(3*kw/2, kw, langue.get_item("key-tab"));
  fixed.put(*key, cx, cy);
  keys.push_back(key);
  key->add_listener(this);


  cx += (3*kw/2+5);
  //cx = ox+3*kw/2;// - kw /3;
  cy = oy;
  add_key("a");
  add_key("z");
  add_key("e");
  add_key("r");
  add_key("t");
  add_key("y");
  add_key("u");
  add_key("i");
  add_key("o");
  add_key("p");

  cx = ox;
  cy = oy + kw + 5;
  key = new GtkKey(3*kw/2, kw, "SHIFT", true);
  fixed.put(*key, cx, cy);
  keys.push_back(key);
  cx += (3*kw/2+5);
  key->add_listener(this);

  add_key("q");
  add_key("s");
  add_key("d");
  add_key("f");
  add_key("g");
  add_key("h");
  add_key("j");
  add_key("k");
  add_key("l");
  add_key("m");

  cx = ox + kw/2 + kw / 3;
  cy = oy + 2 * (kw + 5);
  add_key("w");
  add_key("x");
  add_key("c");
  add_key("v");
  add_key("b");
  add_key("n");

  //cy = 3 * (kw + 5);
  //cx = 4*kw;
  key = new GtkKey(4*kw, kw, langue.get_item("key-space"));
  fixed.put(*key, cx, cy);
  keys.push_back(key);

  key->add_listener(this);


  cx = ox+11*(kw+5) + 25;
  cy = oy;
  add_key("7");
  add_key("8");
  add_key("9");
  cx = ox+11*(kw+5) + 25;
  cy = oy+kw + 5;
  add_key("4");
  add_key("5");
  add_key("6");
  cx = ox+10*(kw+5) + 25;
  cy = oy+2 * (kw + 5);
  add_key("0");
  add_key("1");
  add_key("2");
  add_key("3");

  update_keyboard();

  /*cx = 11*(kw+5) + 30;
  cy = 3 * (kw + 5);
  key = new GtkKey(2*kw, kw, "0");
  fixed.put(*key, cx, cy);
  keys.push_back(key);*/


  //frame.set_label("toto");
  frame.set_border_width(3);

  fixed.set_size_request((kw + 5) * 14 + 40, (kw + 5) * 4 + 5);
  hbox.pack_start(fixed, Gtk::PACK_SHRINK);
  show_all_children(true);

  //signal_key_press_event().connect(sigc::mem_fun( *this, &GtkKeyboard::on_key));
  signal_key_release_event().connect(sigc::mem_fun( *this, &GtkKeyboard::on_key));
  add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
  //this->signal_set_focus().connect(sigc::mem_fun(*this, &GtkKeyboard::on_focus));
  //this->signal_focus_out_event().connect(sigc::mem_fun(*this, &GtkKeyboard::on_focus_out));
  this->signal_focus_in_event().connect(sigc::mem_fun(*this, &GtkKeyboard::on_focus_in));
}
#endif

VirtualKeyboard::~VirtualKeyboard()
{

}

VirtualKeyboard::VirtualKeyboard(Gtk::Window *target_window)
{
  GtkKey *key;

  this->target_window = target_window;
  maj_on = false;

  add(hbox);
  kw = 40;

  int ox = 5, oy = 5;

  cx = ox + kw/2;
  cy = oy;
  add_key(utils::str::latin_to_utf8("&"));
  add_key(utils::str::latin_to_utf8("�"));//langue.getItem("e-aigu"));
  add_key("\"");
  add_key("'");
  add_key("(");
  add_key("-");
  add_key(utils::str::latin_to_utf8("�"));//langue.getItem("e-grave"));
  add_key("_");
  add_key(utils::str::latin_to_utf8("�"));//langue.getItem("cedille"));
  add_key(utils::str::latin_to_utf8("�"));//(langue.getItem("a-grave"));
  add_key(")");
  add_key("=");
  add_key(".");

  key = new GtkKey(3*kw/2, kw, langue.get_item("key-del"));
  fixed.put(*key, cx, cy);
  keys.push_back(key);
  key->add_listener(this);


  oy += kw + 5;


  cx = ox;
  cy = oy;
  key = new GtkKey(3*kw/2, kw, langue.get_item("key-tab"));
  fixed.put(*key, cx, cy);
  keys.push_back(key);
  key->add_listener(this);


  cx += (3*kw/2+5);
  //cx = ox+3*kw/2;// - kw /3;
  cy = oy;
  add_key("a");
  add_key("z");
  add_key("e");
  add_key("r");
  add_key("t");
  add_key("y");
  add_key("u");
  add_key("i");
  add_key("o");
  add_key("p");

  cx = ox;
  cy = oy + kw + 5;
  key = new GtkKey(3*kw/2, kw, "SHIFT", true);
  fixed.put(*key, cx, cy);
  keys.push_back(key);
  cx += (3*kw/2+5);
  key->add_listener(this);

  add_key("q");
  add_key("s");
  add_key("d");
  add_key("f");
  add_key("g");
  add_key("h");
  add_key("j");
  add_key("k");
  add_key("l");
  add_key("m");

  cx = ox + kw/2 + kw / 3;
  cy = oy + 2 * (kw + 5);
  add_key("w");
  add_key("x");
  add_key("c");
  add_key("v");
  add_key("b");
  add_key("n");

  //cy = 3 * (kw + 5);
  //cx = 4*kw;
  key = new GtkKey(4*kw, kw, langue.get_item("key-space"));
  fixed.put(*key, cx, cy);
  keys.push_back(key);

  key->add_listener(this);


  cx = ox+11*(kw+5) + 25;
  cy = oy;
  add_key("7");
  add_key("8");
  add_key("9");
  cx = ox+11*(kw+5) + 25;
  cy = oy+kw + 5;
  add_key("4");
  add_key("5");
  add_key("6");
  cx = ox+10*(kw+5) + 25;
  cy = oy+2 * (kw + 5);
  add_key("0");
  add_key("1");
  add_key("2");
  add_key("3");

  update_keyboard();

  /*cx = 11*(kw+5) + 30;
  cy = 3 * (kw + 5);
  key = new GtkKey(2*kw, kw, "0");
  fixed.put(*key, cx, cy);
  keys.push_back(key);*/


  //frame.set_label("toto");
  set_border_width(3);

  fixed.set_size_request((kw + 5) * 14 + 40, (kw + 5) * 4 + 5);
  hbox.pack_start(fixed, Gtk::PACK_SHRINK);
  show_all_children(true);

  //signal_key_press_event().connect(sigc::mem_fun( *this, &GtkKeyboard::on_key));
  signal_key_release_event().connect(sigc::mem_fun( *this, &VirtualKeyboard::on_key));
  add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
  //this->signal_set_focus().connect(sigc::mem_fun(*this, &GtkKeyboard::on_focus));
  //this->signal_focus_out_event().connect(sigc::mem_fun(*this, &GtkKeyboard::on_focus_out));
  //this->signal_focus_in_event().connect(sigc::mem_fun(*this, &VirtualKeyboard::on_focus_in));
}

#if 0
void GtkKeyboard::on_focus(Gtk::Widget *w)
{
  if(target_window != nullptr)
    target_window->present();
}

bool GtkKeyboard::on_focus_in(GdkEventFocus *gef)
{
  infos("focus in.");
  if(target_window != nullptr)
  {
    target_window->present();
  }
  return true;
}
#endif


Gtk::Frame *WizardStep::get_frame()
{
    return &(parent->mid);
}


void Wizard::start()
{
  current_index = 0;
  state = -1;

  dialog.set_title(title);
  dialog.set_default_size(600,350);
  dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  

  b_cancel.set_label(langue.get_item("Cancel"));
  b_prec.set_label(langue.get_item("Previous"));
  b_next.set_label(langue.get_item("Next"));


  b_prec.set_border_width(1);
  b_next.set_border_width(1);
  b_cancel.set_border_width(1);
  low_buts.pack_start(b_prec);
  low_buts.pack_start(b_next);//, Gtk::PACK_EXPAND_PADDING, 50,10);
  low_buts.pack_start(b_cancel);//, Gtk::PACK_EXPAND_PADDING, 10,10);

  //low_buts.set_child_secondary(b_cancel, true);
  low_buts.set_border_width(5);
  low_buts.set_layout(Gtk::BUTTONBOX_END);

  //high.add(hvbox);
  hvbox.pack_start(main_label,        Gtk::PACK_SHRINK, 5);
  hvbox.pack_start(description_label, Gtk::PACK_SHRINK, 10);



  dialog.add(vbox);


  //wizlab.set_border_width(5);
  hhvbox.pack_start(hvbox, Gtk::PACK_EXPAND_WIDGET);
  //Gtk::Image *buttonImage_ = new Gtk::Image("img/wiznew.png");
  //wizlab.set_image(*buttonImage_);
  hhvbox.pack_end(wizlab, Gtk::PACK_SHRINK, 10);

  Gtk::EventBox *eventBox = new Gtk::EventBox();
  eventBox->add(hhvbox);

  // TODO
  //eventBox->modify_bg(Gtk::STATE_NORMAL, Gdk::Color("white"));

  vbox.pack_start(*eventBox, Gtk::PACK_SHRINK);
  vbox.pack_start(mid, Gtk::PACK_EXPAND_WIDGET);
  vbox.pack_start(low_buts, Gtk::PACK_SHRINK);


  b_cancel.signal_clicked().connect(sigc::mem_fun(*this,&Wizard::on_cancel) );
  b_prec.signal_clicked().connect(sigc::mem_fun(*this,&Wizard::on_prec) );
  b_next.signal_clicked().connect(sigc::mem_fun(*this,&Wizard::on_next) );

  // TODO
  //b_cancel.set_flags(Gtk::CAN_DEFAULT);
  //b_next.set_flags(Gtk::CAN_DEFAULT);

  current = first;

  update_view();

  hvbox.set_border_width(2);

  description_label.set_single_line_mode(false);
  main_label.set_selectable(false);
  description_label.set_selectable(false);

  description_label.set_justify(Gtk::JUSTIFY_LEFT);
  main_label.set_justify(Gtk::JUSTIFY_LEFT);

  current->enter();

  dialog.show_all_children(true);
  Gtk::Main::run(dialog);
}

void Wizard::update_view()
{
  printf("Current step = %s.\n", current->name.c_str());
  b_prec.set_sensitive(current->enable_prec() && (current != steps[0]));
  b_next.set_sensitive(current->enable_next());
  //b_end.set_sensitive(current->enable_end());
  if(current->enable_end())
  {
    b_cancel.set_label(langue.get_item("Terminate"));
    if(current->enable_next())
      b_next.grab_default();
    else
      b_cancel.grab_default();
  }
  else
  {
    b_cancel.set_label(langue.get_item("Cancel"));
    if(current->enable_next())
      b_next.grab_default();
  }

  main_label.set_markup("<b>" + current->title + "</b>");
  description_label.set_text(current->description);
}

void Wizard::set_icon(const string &ipath)
{
  wizlab.set(ipath);
}

void Wizard::add_step(WizardStep *step)
{
  if(steps.size() == 0)
    first = step;
  steps.push_back(step);
  step->parent = this;
}

WizardStep *Wizard::get_step(std::string name)
{
    for(unsigned int i = 0; i < steps.size(); i++)
    {
        if(steps[i]->name.compare(name) == 0)
            return steps[i];
    }
    printf("Step %s not found.\n", name.c_str());
    return nullptr;
}

void Wizard::set_current(std::string name)
{
    WizardStep *ws = get_step(name);
    printf("Going to step \"%s\"...\n", name.c_str());
    if(ws == nullptr)
    {
      printf("Step not defined !\n");
      return;
    }
    mid.remove();
    current = ws;
    update_view();
    current->enter();
    mid.show_all_children(true);
    
}

void Wizard::on_cancel()
{
  dialog.hide();
  if(current->enable_end())
  {
    current->validate();
    state = 0;
  }
}

void Wizard::on_prec()
{
  on_prec_step(current->name);
}

void Wizard::on_next()
{
  current->validate();
  on_next_step(current->name);
  
}

const int NotebookManager::POS_FIRST = 0xffff;
const int NotebookManager::POS_LAST  = 0xfffe;

#ifndef LINUX
static bool registered_icon_size = false;
#endif

NotebookManager::NotebookManager()
{
  current_page = 0;
  this->signal_switch_page().connect(
      sigc::mem_fun(*this, &NotebookManager::on_switch_page));
}

unsigned int NotebookManager::nb_pages() const
{
  return pages.size();
}

void NotebookManager::on_switch_page(Gtk::Widget *page, int page_num)
{
  if(pages.size() > 0)
  {
    PageChange pc;

    pc.previous_widget  = nullptr;
    pc.new_widget       = nullptr;

    if(current_page != -1)
      pc.previous_widget = pages[current_page]->widget;

    if((page_num < 0) || (page_num >= (int) pages.size()))
    {
      erreur("Invalid page num: %d.", page_num);
      return;
    }


    current_page      = page_num;
    pc.new_widget     = pages[current_page]->widget;
    CProvider<PageChange>::dispatch(pc);
  }
}

void NotebookManager::on_b_close(Page *page)
{
  infos("on close(%s).", page->name.c_str());
  current_page = -1;
  /*Gtk::Notebook::remove_page(*page->align);*/


  this->remove_page(*page->widget);
  PageClose pc;
  pc.closed_widget = page->widget;
  CProvider<PageClose>::dispatch(pc);
}

int NotebookManager::add_page(int position,
                              Gtk::Widget &widget,
                              std::string name,
                              std::string icon_path,
                              std::string description)
{
  Page *p = new Page();
  p->widget = &widget;
  p->name   = name;
  p->icon_path = icon_path;
  p->index = -1;
  //p->scroll.add(widget);
  p->scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  pages.push_back(p);

  if(position == POS_FIRST)
  {

  }
  else if(position == POS_LAST)
  {

  }
  else
  {

  }


  Gtk::HBox *ybox = new Gtk::HBox();
  if(icon_path.size() > 0)
  {
    if(!files::file_exists(icon_path))
      erreur("Image not found: %s.", icon_path.c_str());
    else
      ybox->pack_start(*(new Gtk::Image(icon_path)));
  }
  ybox->pack_start(*(new Gtk::Label(" " + name)));

  Gtk::Button *button = new Gtk::Button();
  ybox->pack_start(*button);

  Gtk::IconSize icon_size;

# ifdef LINUX
  icon_size = Gtk::ICON_SIZE_MENU;
# else
  std::string isize = "tiny";

  isize = "";


  if(!registered_icon_size)
  {
    icon_size = Gtk::IconSize::register_new(isize, 6, 6);
    registered_icon_size = true;
  }
  else
    icon_size = Gtk::IconSize::from_name(isize);
# endif

  Gtk::Image *img = new Gtk::Image(Gtk::Stock::CLOSE, icon_size);//Gtk::ICON_SIZE_MENU);
  button->add(*img);

  button->signal_clicked().connect(
      sigc::bind(
      sigc::mem_fun(*this,
      &NotebookManager::on_b_close),
      p));

  //p->align = new Gtk::Alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP, 1.0, 1.0);
  //p->align->add(p->scroll);
  //p->scroll.add(widget);

  //p->align->add(widget);

  //append_page(*p->align, *ybox);

  append_page(widget, *ybox);
  ybox->show_all_children();

  if(description.size() > 0)
  {
    ybox->set_has_tooltip();
    ybox->set_tooltip_markup(description);
  }


  //this->append_page(p->scroll);

  return p->index;
}

NotebookManager::Page::~Page()
{
  delete align;
}

int NotebookManager::remove_page(Gtk::Widget &widget)
{
  infos("Remove page..");
  current_page = -1;
  for(uint32_t i = 0; i < pages.size(); i++)
  {
    if(pages[i]->widget == &widget)
    {
      //this->remove(widget);
      Gtk::Notebook::remove_page(*pages[i]->align);

      std::deque<Page *>::iterator it;
      for(it =  pages.begin(); it !=  pages.end(); it++)
      {
        Page *cur = *it;
        if(cur->widget == pages[i]->widget)
        {
          pages.erase(it);
          infos("Delete page..");
          delete pages[i];
          return 0;
        }
      }
      erreur("Could not remove page from deque.");
      return -1;
    }
  }
  erreur("Page not found.");
  return -1;
}

Gtk::Widget *NotebookManager::get_current_page()
{
  if((unsigned int) current_page >= pages.size())
    return nullptr;
  return pages[current_page]->widget;
}

int NotebookManager::set_current_page(Gtk::Widget &widget)
{
  current_page = 0;//...;
  for(uint32_t i = 0; i < pages.size(); i++)
  {
    if(pages[i]->widget == &widget)
    {
      // TODO
      return 0;
    }
  }
  return -1;
}


void TreeManager::maj_langue()
{
  populate();
}

bool TreeManager::verifie_type_gere(const std::string &s)
{
  if(s == "action")
    return false;
  if(ids.size() == 0)
    return true;
  for(const auto &id: ids)
    if(s == id)
      return true;
  return false;
}

bool TreeManager::a_enfant_visible(const utils::model::Node noeud)
{
  for(const auto &ch: noeud.schema()->children)
    if(verifie_type_gere(ch.child_str))
      return true;
  return false;
}

void TreeManager::populate()
{
  tree_model->clear();
  populate(model, nullptr);
  tree_view.expand_all();
}

void TreeManager::populate(Node m, const Gtk::TreeModel::Row *row)
{
  //for(unsigned int i = 0; i < m.schema()->children.size(); i++)
  for(const auto &ss: m.schema()->children)
  {
    //SubSchema ss = m.schema()->children[i];
    const NodeSchema *schema = ss.ptr;

    if(!verifie_type_gere(schema->name.get_id()))
      continue;

    unsigned int n = m.get_children_count(schema->name.get_id());
    for(unsigned int j = 0; j < n; j++)
    {
      Node no = m.get_child_at(schema->name.get_id(), j);
      const Gtk::TreeModel::Row *subrow;
      //Gtk::TreeModel::iterator subrow;

      if(row == nullptr)
        subrow = &(*(tree_model->append()));
      else
        subrow = &(*(tree_model->append(row->children())));

      std::string s = no.get_identifier(false);

      if(a_enfant_visible(no))
        s = "<b>" + s + "</b>";

      (*subrow)[columns.m_col_name] = s;
      (*subrow)[columns.m_col_ptr]  = no;

      if(has_pic(schema))
        (*subrow)[columns.m_col_pic] = get_pics(schema);
      populate(no, subrow);
    }
  }
}

void TreeManager::on_event(const ChangeEvent &e)
{
  infos("Model change.");
  populate();
}

/*void TreeManager::on_event(const StructChangeEvent &e)
{
  infos("Model structural change.");
  populate();
}*/

Node TreeManager::get_selection()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = tree_view.get_selection();
  Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  Node result;
  if(iter)
  {
    Gtk::TreeModel::Row ligne = *iter;
    result = ligne[columns.m_col_ptr];
  }
  return result;
}

void TreeManager::on_selection_changed()
{
  if(!lock)
  {
    lock = true;
    Node m = get_selection();
    if(!m.is_nullptr())
      setup_row_view(m);
    lock = false;
  }
}

bool TreeManager::MyTreeView::on_button_press_event(GdkEventButton *event)
{
  bool return_value = TreeView::on_button_press_event(event);


  /* Right click */
  if((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
  {
    infos("bpress event");
    Node m = parent->get_selection();
    if(m.is_nullptr())
    {
      infos("no selection");
      return return_value;
    }

    Gtk::Menu *popup = new Gtk::Menu();

    //Gtk::Menu::MenuList &menulist = parent->popup_menu.items();

    // TODO
    //menulist.clear();

    if(m.has_child("action"))
    {
      for(const Node &action: m.children("action"))
      {
        if(!action.get_attribute_as_boolean("enable"))
          continue;

        std::string aname = action.get_attribute_as_string("name");

        if(action.get_attribute_as_string("fr").size() > 0)
        {
          aname = action.get_attribute_as_string("fr");
        }
        aname = utils::str::latin_to_utf8(aname);
        std::pair<Node, std::string> pr;
        pr.first = m;
        pr.second = action.get_attribute_as_string("name");

        infos("add action: %s", pr.second.c_str());

        /*menulist.push_back(
          Gtk::Menu_Helpers::MenuElem(aname,
          sigc::bind<std::pair<Node, std::string> >(
          sigc::mem_fun(parent, &TreeManager::on_menup), pr)));*/
        Gtk::MenuItem *item = Gtk::manage(new Gtk::MenuItem(aname, true));
         item->signal_activate().connect(
             sigc::bind<std::pair<Node, std::string>>(
                   sigc::mem_fun(parent, &TreeManager::on_menup), pr));
        popup->append(*item);
      }
    }

    infos("Show popup...");
    popup->accelerate(*this);
    popup->show_all();
    popup->show_all_children(true);
    /*parent->popup_menu*/popup->popup(event->button, event->time);
  }
  /* Double click */
  else if((event->type == GDK_2BUTTON_PRESS) && (event->button == 1))
  {
    //infos("dclick event");
    Node m = parent->get_selection();
    if(m.is_nullptr())
    {
      //infos("no selection");
      return return_value;
    }

    if(m.has_child("action"))
    {
      for(uint32_t i = 0; i < m.get_children_count("action"); i++)
      {

        Node action = m.get_child_at("action", i);

        if(!action.get_attribute_as_boolean("enable"))
          continue;

        if(!action.get_attribute_as_boolean("default"))
          continue;

        std::string aname = action.get_attribute_as_string("name");

        //parent->infos("action: %s", aname.c_str());

        parent->on_menu(m, aname);
        return return_value;
      }
    }
  }
  return return_value;
}

void TreeManager::on_menup(std::pair<Node, std::string> pr)
{
  on_menu(pr.first, pr.second);
}

void TreeManager::on_menu(Node elt, std::string action)
{
  for(uint32_t i = 0; i < menu_functors.size(); i++)
  {
    if(menu_functors[i]->action.compare(action) == 0)
    {
      menu_functors[i]->call(elt);
    }
  }
}

void TreeManager::set_selection(Gtk::TreeModel::Row &root, Node reg)
{

  typedef Gtk::TreeModel::Children type_children;
  type_children children = root.children();
  for(type_children::iterator iter = children.begin();
  iter != children.end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;
    Node e = row[columns.m_col_ptr];
    if(e == reg)
    {
      tree_view.get_selection()->select(row);
      return;
    }
    set_selection(row, reg);
  }
}

int TreeManager::set_selection(Node reg)
{
  typedef Gtk::TreeModel::Children type_children;
  type_children children = tree_model->children();
  for(type_children::iterator iter = children.begin();
  iter != children.end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;
    Node e = row[columns.m_col_ptr];
    if(e == reg)
    {
      tree_view.get_selection()->select(row);
      return 0;
    }
    set_selection(row, reg);
  }
  return -1;
}

TreeManager::MyTreeView::MyTreeView(TreeManager *parent) : Gtk::TreeView()
{
  this->parent = parent;
}

void TreeManager::on_treeview_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *)
{
  Gtk::TreeModel::iterator iter = tree_model->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    setup_row_view(row[columns.m_col_ptr]);
  }
}

void TreeManager::setup_row_view(Node ptr)
{
  SelectionChangeEvent sce;
  sce.new_selection = ptr;
  dispatch(sce);
}

Glib::RefPtr<Gdk::Pixbuf> TreeManager::get_pics(const NodeSchema *schema)
{
  for(unsigned int i = 0; i < pics.size(); i++)
  {
    if(pics[i].second == schema)
    {
      //infos("Returning pic.");
      return pics[i].first;
    }
  }
  erreur("pic not found for %s", schema->name.get_id().c_str());
  return Glib::RefPtr<Gdk::Pixbuf>();
}

bool TreeManager::has_pic(const NodeSchema *schema)
{
  for(unsigned int i = 0; i < pics.size(); i++)
  {
    if(pics[i].second == schema)
      return true;
  }
  return false;
}

void TreeManager::load_pics(const NodeSchema *sc)
{
  //assert(sc != nullptr);
  //infos("load pics...");
  //auto s = sc->to_string();
  //infos(string("schem = ") + s);

  for (uint32_t i = 0; i < pics_done.size(); i++) {
    if (pics_done[i] == sc)
      return;
  }

  pics_done.push_back(sc);

  if((!has_pic(sc))
      && sc->has_icon())
  {
    std::pair<Glib::RefPtr<Gdk::Pixbuf>, const NodeSchema *> p;
    p.second = sc;
    std::string filename = utils::get_img_path()
    + files::get_path_separator() + sc->icon_path;
    infos(std::string("Loading pic: ") + filename);
    if(!files::file_exists(filename))
      erreur("picture loading: " + filename + " not found.");
    else
    {
      p.first  = Gdk::Pixbuf::create_from_file(filename);
      pics.push_back(p);
      //infos("ok.");
    }
  }
  for(unsigned int i = 0; i < sc->children.size(); i++)
    load_pics(sc->children[i].ptr);
}


TreeManager::TreeManager(): tree_view(this)
{

}

void TreeManager::set_liste_noeuds_affiches(const std::vector<std::string> &ids)
{
  this->ids = ids;
}

void TreeManager::set_model(Node model)
{
  lock = false;
  this->model = model;

  load_pics(model.schema());

  tree_view.set_headers_visible(false);
  add(scroll);
  scroll.add(tree_view);
  scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scroll.set_border_width(5);


  tree_model = Gtk::TreeStore::create(columns);
  tree_view.set_model(tree_model);
  populate();


  Gtk::TreeViewColumn *tvc = Gtk::manage(new Gtk::TreeViewColumn());
  Gtk::CellRendererPixbuf *crp = Gtk::manage(new Gtk::CellRendererPixbuf());
  tvc->pack_start(*crp, false);
  tvc->add_attribute(crp->property_pixbuf(), columns.m_col_pic);

  Gtk::CellRendererText *crt = new Gtk::CellRendererText();
  tvc->pack_start(*crt, true);
  tvc->add_attribute(crt->property_markup(), columns.m_col_name);
  tree_view.append_column(*tvc);

  tree_view.signal_row_activated().connect(sigc::mem_fun(*this,
      &TreeManager::on_treeview_row_activated));

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = tree_view.get_selection();
  refTreeSelection->signal_changed().connect(
      sigc::mem_fun(*this, &TreeManager::on_selection_changed));

  popup_menu.accelerate(tree_view);

  model.add_listener((CListener<ChangeEvent> *) this);
  //model.add_listener((Listener<StructChangeEvent> *) this);
}


TreeManager::TreeManager(Node model): tree_view(this)
{
  set_model(model);
}

#if 0
DialogManager *DialogManager::instance;
DialogManager::Placable *DialogManager::current_window = nullptr;

DialogManager *DialogManager::get_instance()
{
  if(instance == nullptr)
    instance = new DialogManager();
  return instance;
}

DialogManager::DialogManager()
{
  setup("view", "dialog-manager");
  lock = false;
  last_resize_tick = 0;
  last_focus_tick = 0;
}

bool DialogManager::on_timeout(int tn)
{
  infos("timeout.");

  if(current_window != nullptr)
  {
    if(appli_view_prm.use_touchscreen)
    {
      infos("present kb.");

      //GtkKeyboard::get_instance()->set_keep_above();
    }
    infos("present kw.");
    //current_window->raise();//show();//present();
    current_window->set_keep_above();
  }

  /* stop timer */
  return false;
}

void DialogManager::forward_focus()
{
  DialogManager *dm = DialogManager::get_instance();

  if(!appli_view_prm.use_touchscreen)
    return;

  if(dm->lock)
  {
    dm->warning("locked.");
  }

  if(!dm->lock)
  {
    dm->lock = true;


    uint64_t tick = OSThread::get_tick_count_ms();

    if(tick - dm->last_focus_tick < 200)
    {
      dm->infos("forward focus: differed.");
      dm->lock = false;
      return;
    }
    dm->infos("forward focus: now.");
    dm->last_focus_tick = tick;

    // Creation of a new object prevents long lines and shows us a little
      // how slots work.  We have 0 parameters and bool as a return value
      // after calling sigc::bind.
    sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*dm,
                &DialogManager::on_timeout), 0);

    // This is where we connect the slot to the Glib::signal_timeout()
    sigc::connection conn = Glib::signal_timeout().connect(my_slot,
            50);

    /*if(current_window != nullptr)
    {
      if(appli_view_prm.use_touchscreen)
      {
        GtkKeyboard::get_instance()->present();
      }
      current_window->present();
    }*/
    dm->lock = false;
  }
}

void DialogManager::dispose()
{
  if(current_window == nullptr)
    return;

  std::deque<Placable *>::iterator it;
  for (it = windows_stack.begin(); it != windows_stack.end(); it++)
  {
    if(current_window == *it)
    {
      windows_stack.erase(it);
      break;
    }
  }


  current_window = nullptr;
  if(appli_view_prm.use_touchscreen)
  {
    //GtkKeyboard::get_instance()->close();
  }

  if(windows_stack.size() > 0)
  {
    DialogManager::setup(windows_stack[windows_stack.size() - 1]);
  }
}

void DialogManager::update_sizes()
{
# if 0
  uint32_t ox, oy;
  if(!lock)
  {
    lock = true;

    uint64_t tick = OSThread::get_tick_count_ms();

    if(tick - last_resize_tick < 200)
    {
      lock = false;
      return;
    }
    last_resize_tick = tick;


  uint32_t screen_x, screen_y;
  int sx1, sy1, sx2, sy2;
  bool show_keyboard = appli_view_prm.use_touchscreen;
  int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  int delta = 0;//25;

  if(current_window == nullptr)
  {
    lock = false;
    return;
  }

  infos("Re-setup window..");

  if(!show_keyboard)
  {
    lock = false;
    return;
  }

  //current_window->show();
  current_window->resize(10,10);

  //screen_x = current_window->get_screen()->get_width();
  //screen_y = current_window->get_screen()->get_height();
  get_screen(current_window, ox, oy, screen_x, screen_y);

  current_window->get_size(sx1, sy1);

  if(show_keyboard)
    GtkKeyboard::get_instance()->get_size(sx2, sy2);
  else
  {
    sx2 = sy2 = 0;
  }

  current_window->set_position(Gtk::WIN_POS_NONE);
  current_window->resize(sx1, sy1);



  if(show_keyboard)
  {
    ///GtkKeyboard::get_instance()->show();
    //GtkKeyboard::get_instance()->display(current_window);
    /* ------------------------------
     *            sx1                        ^
     *     -------------------               |
     *     |                 |  sy1          |
     *     |                 |               |
     *     -------------------               |
     *                         delta         |  screen_y
     *     -------------------               |
     *     |                 |  sy2          |
     *     |                 |               |
     *     -------------------               |
     *            sx2                        |
     * -------------------------------
     *                screen_x
     */

    /* y1 + sy1 + delta + sy2 + y1 = screen_y */
    if(sy1 + sy2 + 2 * delta <= (int) screen_y)
    {
      y1 = (screen_y - sy1 - 2 * delta - sy2) / 2;
      infos("Screen is large enough.");
    }
    else
    {
      warning("Screen is too small: scroll bar is mandatory.");

      // Force the y dimension:
      // delta * 3 + sy1 + sy2 = screen_y
      sy1 = screen_y - (delta * 3 + sy2);
      y1  = delta;
      // Adjust sx1 to take the scroll into account
      sx1 += delta;
      current_window->force_scroll(sx1, sy1);
      current_window->resize(sx1, sy1);
    }

    x2 = (screen_x - sx2) / 2;
    y2 = y1 + sy1 + delta;

    GtkKeyboard::get_instance()->move(ox + x2, oy + y2);
  }
  else
  {
    if((sy1 + delta) <= (int) screen_y)
    {
      infos("Screen is large enough.");
      y1 = (screen_y - sy1) / 2;
    }
    else
    {
      warning("Screen is too small: scroll bar is mandatory.");
      // Force the y dimension:
      // delta * 3 + sy1 + sy2 = screen_y
      sy1 = screen_y - delta * 2;
      y1  = delta;
      // Adjust sx1 to take the scroll into account
      sx1 += delta;
      current_window->force_scroll(sx1, sy1);
      current_window->resize(sx1, sy1);
    }
  }
  x1 = (screen_x - sx1) / 2;
  current_window->move(x1 + ox, y1 + oy);
  infos("SCR=(%d,%d), W1=(%d,%d), W2=(%d,%d).",
        screen_x, screen_y,
        sx1, sy1,
        sx2, sy2);
  infos("--> P1=(%d,%d), P2=(%d,d).",
        x1, y1, x2, y2);
  lock = false;
  }
# endif
  //wnd->present();
}
#endif

void DialogManager::get_screen(Gtk::Window *wnd, uint32_t &ox, uint32_t &oy, uint32_t &dx, uint32_t &dy)
{
  if(appli_view_prm.fixed_size)
  {
    ox = appli_view_prm.ox;
    oy = appli_view_prm.oy;
    dx = appli_view_prm.dx;
    dy = appli_view_prm.dy;
  }
  else
  {
    ox = 0;
    oy = 0;
    dx = wnd->get_screen()->get_width();
    dy = wnd->get_screen()->get_height();
    /* for task bar */
    dy -= 40;
  }
}

class DefaultPlacable: public DialogManager::Placable
{
public:
  DefaultPlacable(Gtk::Window *wnd){this->wnd = wnd;}
  Gtk::Window *get_window(){return wnd;}
  void force_scroll(int dx, int dy){}
  void unforce_scroll(){}
private:
  Gtk::Window *wnd;
};

void DialogManager::setup_window(Gtk::Window *wnd, bool fullscreen)
{
  DefaultPlacable dp(wnd);
  setup_window(&dp, fullscreen);
# if 0
  uint32_t screen_x, screen_y;
    int sx1, sy1;
    int x1 = 0, y1 = 0;
    uint32_t ox, oy;

    wnd->show_all_children(true);
    wnd->resize(10,10);

    get_screen(wnd, ox, oy, screen_x, screen_y);

    TraceManager::infos(TraceManager::AL_NORMAL, "view", "SETUP WND: ox=%d,oy=%d,sx=%d,sy=%d.", ox,oy,screen_x,screen_y);

    wnd->get_size(sx1, sy1);
    if(fullscreen)
    {
      if(sx1 < (int) screen_x)
        sx1 = screen_x;
      if(sy1 < (int) screen_y)
        sy1 = screen_y;
    }
    wnd->set_position(Gtk::WIN_POS_NONE);
    wnd->resize(sx1, sy1);

    y1 = (screen_y - sy1) / 2;
    x1 = (screen_x - sx1) / 2;

    if((sy1 <= (int) screen_y) && ((sx1 <= (int) screen_x)))
    {
      TraceManager::infos(TraceManager::AL_NORMAL, "view", "Screen is large enough.");
    }
    else
    {
      TraceManager::infos(TraceManager::AL_WARNING, "view", "Screen is too small: scroll bar is mandatory.");
      // Force the y dimension:
      // delta * 3 + sy1 + sy2 = screen_y
      if(sy1 > (int) screen_y)
      {
        y1  = 0;
        sy1 = screen_y;
        // Adjust sx1 to take the scroll into account
        sx1 += 30;
      }
      if(sx1 > (int) screen_x)
      {
        x1  = 0;
        sx1 = screen_x;
        // Adjust sx1 to take the scroll into account
        sy1 += 30;
      }

      wnd->resize(sx1, sy1);
    }

    wnd->move(ox + x1, oy + y1);
    TraceManager::infos(TraceManager::AL_NORMAL, "view", "SCR=(%d,%d), WREQ=(%d,%d).",
          screen_x, screen_y,
          sx1, sy1);
    TraceManager::infos(TraceManager::AL_NORMAL, "view", "--> POS=(%d,%d).",
          x1, y1);
    wnd->present();
#   endif
}

void DialogManager::setup_window(Placable *wnd, bool fullscreen)
{
  uint32_t screen_x, screen_y;
  int sx1, sy1;
  int x1 = 0, y1 = 0;
  uint32_t ox, oy;

  wnd->get_window()->show_all_children(true);
  wnd->get_window()->resize(10,10);

  get_screen(wnd->get_window(), ox, oy, screen_x, screen_y);

  /*TraceManager::infos(AL_NORMAL, "view",
                      "SETUP WND (FS = %s): ox=%d,oy=%d,sx=%d,sy=%d.",
                      fullscreen ? "true" : "false",
                      ox,oy,screen_x,screen_y);*/

  wnd->get_window()->get_size(sx1, sy1);
  if(fullscreen)
  {
    if(sx1 < (int) screen_x)
      sx1 = screen_x;
    if(sy1 < (int) screen_y)
      sy1 = screen_y;
  }

  wnd->get_window()->set_position(Gtk::WIN_POS_NONE);
  wnd->get_window()->resize(sx1, sy1);

  y1 = (screen_y - sy1) / 2;
  x1 = (screen_x - sx1) / 2;

  if((sy1 <= (int) screen_y) && ((sx1 <= (int) screen_x)))
  {
    //TraceManager::infos(AL_NORMAL, "view",
    //    "Screen is large enough.");
  }
  else
  {
    avertissement("Screen is too small: scroll bar is mandatory.");
    // Force the y dimension:
    // delta * 3 + sy1 + sy2 = screen_y
    if(sy1 > (int) screen_y)
    {
      y1  = 0;
      sy1 = screen_y;
      // Adjust sx1 to take the scroll into account
      sx1 += 30;
    }
    if(sx1 > (int) screen_x)
    {
      x1  = 0;
      sx1 = screen_x;
      // Adjust sx1 to take the scroll into account
      sy1 += 30;
      if(sy1 > (int) screen_y)
      {
        sy1 = screen_y;
        y1  = 0;
      }
    }

    wnd->force_scroll(sx1, sy1);
    wnd->get_window()->resize(sx1, sy1);
  }

  wnd->get_window()->move(ox + x1, oy + y1);
  /*TraceManager::infos(AL_NORMAL, "view", "SCR=(%d,%d,%d,%d), WREQ=(%d,%d).",
        ox, oy,
        screen_x, screen_y,
        sx1, sy1);
  TraceManager::infos(AL_NORMAL, "view", "--> POS=(%d,%d).",
        x1, y1);*/
  wnd->get_window()->present();
}


#if 0
void DialogManager::setup(Placable *wnd)
{
# if 0
  unsigned int i;
  uint32_t screen_x, screen_y;
  int sx1, sy1, sx2, sy2;
  bool show_keyboard = appli_view_prm.use_touchscreen;
  int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  int delta = 0;//25;
  uint32_t ox, oy;

  for(i = 0; i < windows_stack.size(); i++)
  {
    if(windows_stack[i] == wnd)
      break;
  }
  if(i == windows_stack.size())
    windows_stack.push_back(wnd);

  infos("setup window..");

  current_window = wnd;

  wnd->show_all_children(true);
  wnd->resize(10,10);

  //screen_x = wnd->get_screen()->get_width();
  //screen_y = wnd->get_screen()->get_height();
  get_screen(wnd, ox, oy, screen_x, screen_y);

  wnd->get_size(sx1, sy1);

  if(show_keyboard)
    GtkKeyboard::get_instance()->get_size(sx2, sy2);
  else
  {
    sx2 = sy2 = 0;
  }

  wnd->set_position(Gtk::WIN_POS_NONE);
  wnd->resize(sx1, sy1);

  if(show_keyboard)
  {
    ///GtkKeyboard::get_instance()->show();
    GtkKeyboard::get_instance()->display(wnd);
    /* ------------------------------
     * d1         sx1                        ^
     *     -------------------               |
     *     |                 |  sy1          |
     *     |                 |               |
     *     -------------------               |
     * d2                      delta         |  screen_y
     *     -------------------               |
     *     |                 |  sy2          |
     *     |                 |               |
     *     -------------------               |
     * d3         sx2                        |
     * -------------------------------
     *                screen_x
     */

    /* y1 + sy1 + delta + sy2 + y1 = screen_y */
    if(sy1 + sy2 + delta <= (int) screen_y)
    {
      y1 = (screen_y - sy1 - delta - sy2) / 2;
      infos("Screen is large enough.");
    }
    else
    {
      warning("Screen is too small: scroll bar is mandatory.");

      // Force the y dimension:
      // delta * 3 + sy1 + sy2 = screen_y
      sy1 = screen_y - (delta * 3 + sy2);
      y1  = delta;
      // Adjust sx1 to take the scroll into account
      sx1 += delta;
      wnd->force_scroll(sx1, sy1);
      wnd->resize(sx1, sy1);
    }

    x2 = (screen_x - sx2) / 2;
    y2 = y1 + sy1 + delta;

    GtkKeyboard::get_instance()->move(ox + x2, oy + y2);
    GtkKeyboard::get_instance()->present();
  }
  else
  {
    if((sy1 + delta) <= (int) screen_y)
    {
      infos("Screen is large enough.");
      y1 = (screen_y - sy1) / 2;
    }
    else
    {
      warning("Screen is too small: scroll bar is mandatory.");
      // Force the y dimension:
      // delta * 3 + sy1 + sy2 = screen_y
      sy1 = screen_y - delta * 2;
      y1  = delta;
      // Adjust sx1 to take the scroll into account
      sx1 += delta;
      wnd->force_scroll(sx1, sy1);
      wnd->resize(sx1, sy1);
    }
  }
  x1 = (screen_x - sx1) / 2;
  wnd->move(ox + x1, oy + y1);
  infos("SCR=(%d,%d), W1=(%d,%d), W2=(%d,%d).",
        screen_x, screen_y,
        sx1, sy1,
        sx2, sy2);
  infos("--> P1=(%d,%d), P2=(%d,%d).",
        x1, y1, x2, y2);

  wnd->present();
# endif
}
#endif

ColorRectangle::ColorRectangle(const GColor &col, uint16_t width, uint16_t height)
{
  this->width  = width;
  this->height = height;
  this->col    = col;
  realized = false;
  set_size_request(width, height);
  signal_realize().connect(sigc::mem_fun(*this, &ColorRectangle::on_the_realisation));
  signal_draw().connect(sigc::mem_fun(*this,&ColorRectangle::on_expose_event));
}

void ColorRectangle::update_color(const GColor &col)
{
  this->col = col;
  update_view();
}

ColorRectangle::~ColorRectangle()
{

}

bool ColorRectangle::on_expose_event(const Cairo::RefPtr<Cairo::Context> &cr)
{
  //infos("exposed");
  update_view();
  //sc_instance->show_all_children(true);
  return true;
}

void ColorRectangle::on_the_realisation()
{
  //infos("realized.");
  wnd = get_window();
  gc = wnd->create_cairo_context();
  realized = true;
  update_view();
  sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this,
                                            &ColorRectangle::on_timer), 0);
  //Glib::signal_timeout().connect(my_slot, 250);
}

bool ColorRectangle::on_timer(int unused)
{
  update_view();
  return true;
}

void ColorRectangle::update_view()
{
  if(!realized)
    return;

  //infos("update_view..");

  Gdk::Color gdkcolor = col.to_gdk();

  gc->set_source_rgb(gdkcolor.get_red(), gdkcolor.get_green(), gdkcolor.get_blue());

  uint16_t y;
  for(y = 0; y < height; y++)
  {
    gc->move_to(0, y);
    gc->line_to(width, y);
  }
  //infos("done.");
}

ColorButton::ColorButton(Attribute *model):
    al(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0)
{
  lock = false;
  this->model = model;
  std::string col = model->get_string();


  if(appli_view_prm.use_touchscreen)
    crec = new ColorRectangle(GColor(col), 90, 20);
  else
    crec = new ColorRectangle(GColor(col), 90, 12);
  al.add(*crec);
  box.set_border_width(2);
  box.pack_start(al, false, false, 3);
  box.show_all();
  add(box);
  show_all_children(true);
  signal_clicked().connect(
                 sigc::mem_fun(*this,
                 &ColorButton::on_b_pressed));
  model->add_listener(this);
}

void ColorButton::on_b_pressed()
{
  infos("on_b_pressed..");

# if 0
  if(model->schema.constraints.size() > 0)
  {
    std::vector<GColor> colors;

    for(i = 0; i < model->schema.constraints.size(); i++)
    {
      colors.push_back(GColor(model->schema.constraints[i]));
    }

    // Initial color selection.
    uint32_t initial;

    for(i = 0; i < colors.size(); i++)
    {
      if(colors[i].to_string().compare(model->value) == 0)
      {
        initial = i;
        break;
      }
    }

    if(i == colors.size())
    {
      anomaly("Initial color not found in palette: %s.", model->value.c_str());
      initial = 0;
    }

    ColorPaletteDialog dialog(colors, initial);
    if(dialog.display_modal() == 0)
    {
      if(dialog.color_index >= model->schema.constraints.size())
      {
        erreur("Invalid color index: %d >= %d.", dialog.color_index, model->schema.constraints.size());
      }

      infos("color change confirmed: %d -> %s.",
            dialog.color_index,
            model->schema.constraints[dialog.color_index].c_str());
      model->set_value(model->schema.constraints[dialog.color_index]);
    }
    return;
  }
 #endif

  ColorDialog dlg;
  if(dlg.display(GColor(model->get_string()), model->schema->constraints) == 0)
  {
    infos("user response = ok.");
    GColor csel = dlg.get_color();
    std::string scol = csel.to_string();
    model->set_value(scol);
  }
  else
  {
    infos("user response = cancel.");
  }


  /*Gtk::ColorSelectionDialog dlg;

  if(!appli_view_prm.use_decorations)
  {
    dlg.set_decorated(false);
  }
  if(appli_view_prm.img_validate.size() > 0)
  {
    Gtk::Button *b_close_ptr = dlg.add_button(langue.getItem("cancel"), Gtk::RESPONSE_CANCEL);
    Gtk::Button *b_apply_ptr = dlg.add_button(langue.getItem("valid"), Gtk::RESPONSE_ACCEPT);
    Gtk::Image *bim = new Gtk::Image(appli_view_prm.img_cancel);
    b_close_ptr->set_image(*bim);
    b_close_ptr->set_image_position(Gtk::POS_TOP);
    bim = new Gtk::Image(appli_view_prm.img_validate);
    b_apply_ptr->set_image(*bim);
    b_apply_ptr->set_image_position(Gtk::POS_TOP);
  }
  if(appli_view_prm.use_touchscreen)
  {
    dlg.set_keep_above();
  }

  {
    Gtk::ColorSelection *csel = dlg.get_colorsel();
    csel->set_current_color(GColor(model->value).to_gdk());
  }

  if(dlg.run() == Gtk::RESPONSE_OK)
  {
    infos("user response = ok.");
    Gtk::ColorSelection *csel = dlg.get_colorsel();
    Gdk::Color col = csel->get_current_color();
    GColor col2(col);
    std::string scol = col2.to_string();
    model->set_value(scol);
  }
  else
  {
    infos("user response = cancel.");
  }*/
}

void ColorButton::on_event(const ChangeEvent &ce)
{
  if(!lock)
  {
    lock = true;

    //if((ce.type == ChangeEvent::ATTRIBUTE_CHANGED)
    //  && (ce.source == model))
    if(ce.type == ChangeEvent::ATTRIBUTE_CHANGED)
    {
      infos("model change.");
      crec->update_color(GColor(model->get_string()));
    }

    lock = false;
  }
}

ColorButton::~ColorButton()
{
  delete crec;
}

ColorDialog::ColorDialog()
: GenDialog(GenDialog::GEN_DIALOG_VALID_CANCEL, langue.get_item("title-sel-color"))
{
  //set_title(langue.getItem("color-selection"));
  //add(vbox);
  vbox->pack_start(cs, Gtk::PACK_SHRINK);
  /*vbox.pack_start(toolbar, Gtk::PACK_SHRINK);

  Gtk::Image *bim;

  if(appli_view_prm.img_validate.size() > 0)
    bim = new Gtk::Image(appli_view_prm.img_validate);
  else
    bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::APPLY), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  tool_valid.set_icon_widget(*bim);

  if(appli_view_prm.img_cancel.size() > 0)
  {
    bim = new Gtk::Image(appli_view_prm.img_cancel);
  }
  else
    bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::CANCEL), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  tool_cancel.set_icon_widget(*bim);
  toolbar.insert(tool_cancel, -1,
                   sigc::mem_fun(*this, &ColorDialog::on_b_cancel));
  toolbar.insert(sep2, -1);
  sep2.set_expand(true);
  sep2.set_property("draw", false);
  toolbar.insert(tool_valid, -1,
                 sigc::mem_fun(*this, &ColorDialog::on_b_apply));

  tool_valid.set_label(langue.getItem("b-valid"));
  tool_cancel.set_label(langue.getItem("b-cancel"));*/

  show_all_children(true);
}

int ColorDialog::display(GColor initial_color, std::vector<std::string> constraints)
{
  unsigned int i;

  if(constraints.size() > 0)
  {
    std::vector<GColor> colors;

    for(i = 0; i < constraints.size(); i++)
    {
      colors.push_back(GColor(constraints[i]));
    }

    // Initial color selection.
    uint32_t initial = 0;

    for(i = 0; i < colors.size(); i++)
    {
      if(colors[i].to_string().compare(initial_color.to_string()) == 0)
      {
        initial = i;
        break;
      }
    }

    if(i == colors.size())
    {
      erreur("Initial color not found in palette: %s.", initial_color.to_string().c_str());
      initial = 0;
    }

    ColorPaletteDialog dialog(colors, initial);
    if(dialog.display_modal() == 0)
    {
      if(dialog.color_index >= constraints.size())
      {
        erreur("Invalid color index: %d >= %d.", dialog.color_index, constraints.size());
        return -1;
      }

      infos("color change confirmed: %d -> %s.",
            dialog.color_index,
            constraints[dialog.color_index].c_str());

      //model->set_value(model->schema.constraints[dialog.color_index]);
      
      selected = constraints[dialog.color_index];

      return 0;
    }
    else
      return -1;
  }
  else
  {
    cs.set_current_color(initial_color.to_gdk());
    int res = this->display_modal();
    if(res == 0)
      selected = GColor(cs.get_current_color());
    return res;
  }
}

void ColorDialog::on_valid()
{
  //result_ok = true;
  //hide();
  //DialogManager::get_instance()->dispose();
  //GtkKeyboard::get_instance()->close();
}

void ColorDialog::on_cancel()
{
  //hide();
  //DialogManager::get_instance()->dispose();
  //GtkKeyboard::get_instance()->close();
}

GColor ColorDialog::get_color()
{
  return selected;//GColor(cs.get_current_color());
}

void ColorDialog::force_scroll(int dx, int dy)
{

}

void ColorDialog::unforce_scroll()
{

}

GenDialog::GenDialog(GenDialogType type, std::string title)
{
  result_ok = false;
  this->type = type;

  set_position(Gtk::WIN_POS_CENTER);
  set_decorated(appli_view_prm.use_decorations);
  vbox = get_vbox();

  if(title.size() > 0)
  {
    if(appli_view_prm.use_decorations)
    {
      set_title(title);
    }
    else
    {
      label_title.set_markup(std::string("<b>") + title + "</b>");
      vbox->pack_start(label_title, Gtk::PACK_SHRINK);
    }
  }

  if(appli_view_prm.use_button_toolbar)
  {
    vbox->pack_end(toolbar, Gtk::PACK_SHRINK);

    Gtk::Image *bim;

    if(appli_view_prm.img_validate.size() > 0)
      bim = new Gtk::Image(appli_view_prm.img_validate);
    else
      bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::APPLY), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
    tool_valid.set_icon_widget(*bim);

    if(appli_view_prm.img_cancel.size() > 0)
    {
      bim = new Gtk::Image(appli_view_prm.img_cancel);
    }
    else
      bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::CANCEL), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
    tool_cancel.set_icon_widget(*bim);

    if(!((type == GEN_DIALOG_APPLY_CANCEL) || (type  == GEN_DIALOG_VALID_CANCEL)))
      toolbar.insert(sep2, -1);
    toolbar.insert(tool_cancel, -1,
                     sigc::mem_fun(*this, &GenDialog::on_b_cancel_close));
    if((type == GEN_DIALOG_APPLY_CANCEL) || (type  == GEN_DIALOG_VALID_CANCEL))
    toolbar.insert(sep2, -1);
    sep2.set_expand(true);
    sep2.set_property("draw", false);
    if((type == GEN_DIALOG_APPLY_CANCEL) || (type  == GEN_DIALOG_VALID_CANCEL))
    toolbar.insert(tool_valid, -1,
                   sigc::mem_fun(*this, &GenDialog::on_b_apply_valid));

    tool_valid.set_label(langue.get_item("b-valid"));
    tool_cancel.set_label(langue.get_item("b-cancel"));
  }
  else
  {
    hbox.pack_end(b_cancel, Gtk::PACK_SHRINK);
    if((type == GEN_DIALOG_APPLY_CANCEL) || (type  == GEN_DIALOG_VALID_CANCEL))
    hbox.pack_end(b_valid, Gtk::PACK_SHRINK);
    hbox.set_layout(Gtk::BUTTONBOX_END);

    b_cancel.set_border_width(4);
    b_valid.set_border_width(4);

    b_cancel.set_label(langue.get_item("b-cancel"));
    b_valid.set_label(langue.get_item("b-valid"));
    vbox->pack_end(hbox, Gtk::PACK_SHRINK);

    b_valid.signal_clicked().connect(sigc::mem_fun(*this, &GenDialog::on_b_apply_valid));
    b_cancel.signal_clicked().connect(sigc::mem_fun(*this, &GenDialog::on_b_cancel_close));
  }

  show_all_children(true);
}


/*if(appli_view_prm.use_touchscreen)
  set_keep_above();*/


void GenDialog::on_b_apply_valid()
{
  result_ok = true;
  infos("on_b_apply_valid: hide...");
  hide();
  infos("Will call on_apply...");
  if(type == GEN_DIALOG_APPLY_CANCEL)
    on_apply();
  else if(type == GEN_DIALOG_VALID_CANCEL)
    on_valid();
  else
    erreur("unmanaged dialog type.");
  infos("done.");
}

void GenDialog::on_b_cancel_close()
{
  result_ok = false;
  hide();
  if(type == GEN_DIALOG_CLOSE)
    on_close();
  else
    on_cancel();
}

void GenDialog::enable_validation(bool enable)
{
  if(appli_view_prm.use_touchscreen)
    tool_valid.set_sensitive(enable);
  else
    b_valid.set_sensitive(enable);
}

int GenDialog::display_modal()
{
  DialogManager::setup_window((Placable *) this, false);
  run();
  if(result_ok)
    return 0;
  return 1;
}

Gtk::Window *GenDialog::get_window()
{
  return this;
}

ColorPaletteDialog::ColorPaletteDialog(const std::vector<GColor> &colors,
                                       uint32_t initial_color)
  : GenDialog(GenDialog::GEN_DIALOG_VALID_CANCEL,
    langue.get_item("title-sel-color")),
    palette(colors, initial_color)
{
  this->colors = colors;
  this->vbox->pack_start(palette, Gtk::PACK_SHRINK);
  show_all_children(true);
}

ColorPaletteDialog::~ColorPaletteDialog()
{

}

void ColorPaletteDialog::on_valid()
{
  color_index = palette.current_color;
  infos("on valid.");
}

void ColorPaletteDialog::on_cancel()
{
  infos("on cancel");
}

ColorPalette::ColorPalette(const std::vector<GColor> &colors,
                           uint32_t initial_color)
{
  this->colors = colors;
  ncols = 5;
  if(colors.size() < (unsigned int) ncols)
    ncols = colors.size();
  nrows = (colors.size() + ncols - 1) / ncols;
  realized = false;
  current_color = initial_color;

  c_width  = 60;
  c_height = 50;
  width   = c_width * ncols;
  height  = c_height * nrows;

  infos("ncolors = %d, ncolumns = %d, nrows = %d.", colors.size(), ncols, nrows);
  infos("set_size_request(%d,%d)", width, height);
  set_size_request(width, height);
  signal_realize().connect(sigc::mem_fun(*this, &ColorPalette::on_the_realisation));
  //signal_expose_event().connect(sigc::mem_fun(*this,&ColorPalette::on_expose_event));
  signal_button_press_event().connect(sigc::mem_fun( *this, &ColorPalette::on_mouse ) );
  //signal_button_release_event().connect(sigc::mem_fun( *this, &ColorPalette::on_mouse_release ) );
  set_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK);
  set_double_buffered(true);


# ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Connect the signal handler if it isn't already a virtual method override:
  signal_draw().connect(sigc::mem_fun(*this, &ColorPalette::on_draw), false);
# endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
}

bool ColorPalette::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  update_view();
  return true;
}

bool ColorPalette::on_mouse(GdkEventButton *event)
{
  int x = (int) event->x;
  int y = (int) event->y;
  /* Left click? */
  if(event->button == 1)
  {
    infos("left click @%d, %d.", x, y);
    unsigned int col, row;

    col = x / c_width;
    row = y / c_height;

    infos("col = %d, row = %d.", col, row);

    if((col < (unsigned int) ncols) && (row < (unsigned int) nrows))
    {
      unsigned int color = col + row * ncols;
      if(color < colors.size())
      {
        infos("color = %d.", color);
        current_color = color;
        update_view();
      }
      else
      {
        erreur("invalid color index: %d.", color);
      }
    }

  }
  return true;
}

ColorPalette::~ColorPalette()
{

}


void ColorPalette::update_view()
{
  if(!realized)
  {
    infos("update view: not realized.");
    return;
  }
  //GColor background = appli_view_prm.background_color;

  infos("update view..");

  float degrees = 3.1415926 / 180.0;

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();

  infos("width = %d, height = %d.", width, height);

  Glib::RefPtr<Gdk::Window> wnd = get_window();

  Cairo::RefPtr<Cairo::Context> cr = wnd->create_cairo_context();
  cr->set_line_width(1.5);
  cr->set_source_rgb(1, 0, 1);
  cr->rectangle(0, 0, width, height);
  cr->clip();

  float foreground;
  float background;

  // Fond noir
  if(appli_view_prm.inverted_colors)
  {
    background = 0.0;
    foreground = 1.0;
  }
  else
  {
    background = 0.8;
    foreground = 0.0;
  }

  // Clear display
  cr->set_source_rgb(background, background, background);
  cr->rectangle(0, 0, width, height);
  cr->fill();
  //cr->stroke();

  for(unsigned int col = 0; col < (unsigned int) ncols; col++)
  {
    for(unsigned int row = 0; row < (unsigned int) nrows; row++)
    {
      unsigned int x, y, width, height;
      GColor color = colors[col + row * ncols];

      x = col * c_width;
      y = row * c_height;
      float radius = c_height / 4.0;
      width = c_width;
      height = c_height;

      if(current_color == col + row * ncols)
      {
        infos("SEL col %d row %d ccol %d.", col, row, current_color);
        cr->set_source_rgb(foreground, foreground, foreground);
        cr->arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
        cr->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
        cr->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
        cr->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
        cr->close_path();
        cr->stroke();
      }

      //cr->set_source_rgb(0.7 * foreground + 0.3 * background, 0.9 * foreground + 0.1 * background, 0.7 * foreground + 0.3 * background);
      infos("color(%d) = %d.%d.%d.", col + row * ncols, color.red, color.green, color.blue);
      cr->set_source_rgb(((float) color.red) / 256.0, ((float) color.green) / 256.0, ((float) color.blue) / 256.0);
      x = x + width / 10;
      y = y + height / 10;
      width = (width * 8) / 10;
      height = (height * 8) / 10;
      radius = (radius * 8) / 10;
      cr->arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
      cr->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
      cr->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
      cr->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
      cr->close_path();
      cr->fill();
      cr->stroke();
    }
  }
  cr->stroke();
}



bool ColorPalette::on_expose_event(GdkEventExpose *event)
{
  infos("exposed");
  update_view();
  return true;
}

void ColorPalette::on_the_realisation()
{
  infos("realized.");
  realized = true;
}

SensitiveLabel::SensitiveLabel(std::string path)
{
  this->path = path;
  add(label);
  set_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK);
  signal_button_press_event().connect(sigc::mem_fun(this, &SensitiveLabel::on_button_press_event));
}

bool SensitiveLabel::on_button_press_event(GdkEventButton *event)
{
  if((event->type == GDK_2BUTTON_PRESS) && (event->button == 1))
  {
    //printf("dispatch(%s)..\n", path.c_str());

    LabelClick lc;
    lc.path = path;
    lc.type = LabelClick::VAL_CLICK;
    dispatch(lc);
  }
  else if((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
  {
    //printf("dispatch(%s)..\n", path.c_str());

    LabelClick lc;
    lc.path = path;
    lc.type = LabelClick::SEL_CLICK;
    dispatch(lc);
  }
  return true;
}


void ProgressDialog::set_widget(Gtk::Widget *wid)
{
  this->wid = wid;
  iframe.add(*wid);
}

void ProgressDialog::maj_texte(const std::string &s)
{
  label.set_markup(s);
}

ProgressDialog::ProgressDialog(): event_fifo(4)
{
  wid = nullptr;
  set_position(Gtk::WIN_POS_CENTER);

  if(!appli_view_prm.use_decorations)
    set_decorated(false);

  progress.set_pulse_step(0.01);
  set_default_size(300,200);

  Gtk::Box *box = this->get_vbox();

  Gtk::Image *bim;
  if(appli_view_prm.img_cancel.size() > 0)
    bim = new Gtk::Image(appli_view_prm.img_cancel);
  else
    bim = new Gtk::Image(Gtk::StockID(Gtk::Stock::CANCEL), Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  bt.set_icon_widget(*bim);

  toolbar.insert(sep, -1);
  sep.set_expand(true);
  sep.set_property("draw", false);
  toolbar.insert(bt, -1,
                       sigc::mem_fun(*this, &ProgressDialog::on_b_cancel));

  box->pack_start(label, Gtk::PACK_SHRINK);
  box->pack_start(progress, Gtk::PACK_SHRINK);
  box->pack_start(iframe, Gtk::PACK_EXPAND_WIDGET);
  box->pack_end(toolbar, Gtk::PACK_SHRINK);

  label.set_line_wrap(true);

  gtk_dispatcher.add_listener(this, &ProgressDialog::on_event);
  utils::hal::thread_start(this, &ProgressDialog::thread_entry);
}

ProgressDialog::~ProgressDialog()
{
  iframe.remove();
  event_fifo.push(Event::EXIT);
  exit_done.wait();
}

void ProgressDialog::on_b_cancel()
{
  canceled = true;
  hide();
}

bool ProgressDialog::on_timer(int index)
{
  //trace_verbeuse("on timer: in_pr = %d.", (int) in_progress);
  if(!in_progress || canceled)
    return false;
  progress.pulse();
  return true;
}

void ProgressDialog::on_event(const Bidon &bidon)
{
  trace_verbeuse("Now stopping.");
  hide();
}

void ProgressDialog::thread_entry()
{
  for(;;)
  {
    Event e = event_fifo.pop();

    switch(e)
    {
      case Event::START:
      {
        in_progress = true;
        //can_start.wait();
        //callback_done.clear();
        functor->call();
        //callback_done.raise();

        in_progress = false;

        if(!canceled)
        {
          Bidon bidon;
          gtk_dispatcher.on_event(bidon);
        }
        break;
      }
      case Event::EXIT:
        infos("Exit thread...");
        exit_done.raise();
        return;
    }
  }
}

void ProgressDialog::setup(std::string title, std::string text)
{
  trace_verbeuse("setup(%s)...", title.c_str());
  canceled = false;

  this->set_title(title);

  label.set_markup(text);

  if(utils::mmi::mainWindow != nullptr)
    set_transient_for(*utils::mmi::mainWindow);

  bt.set_label(langue.get_item("cancel"));

  show_all_children(true);

  sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this,
               &ProgressDialog::on_timer), 0);

   // This is where we connect the slot to the Glib::signal_timeout()
   /*sigc::connection conn = */
  Glib::signal_timeout().connect(my_slot, 50);

 DialogManager::setup_window(this);

 trace_verbeuse("raising...");
 //can_start.raise();

 event_fifo.push(Event::START);

  int res = run();
  trace_verbeuse("Progress dialog done, res = %d.", res);
  if(res == Gtk::RESPONSE_CANCEL)
  {

  }
}

GtkLed::GtkLed(unsigned int size, bool is_red, bool is_mutable)
{
  this->is_yellow = false;
  this->size = size;
  this->is_red = is_red;
  this->is_mutable = is_mutable;
  this->is_sensitive = true;
  realized = false;
  is_lighted = false;
  set_size_request(size,size);
  //signal_realize().connect(sigc::mem_fun(*this, &GtkLed::on_the_realisation));
  signal_button_press_event().connect( sigc::mem_fun( *this, &GtkLed::on_mouse));
  add_events(Gdk::BUTTON_PRESS_MASK);
}

bool GtkLed::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  int rayon = width < height ? width : height;

  float b, g, r;
  float other, main;
  if(is_lighted)
  {
    other = 0.1;
    main  = 0.9;
  }
  else
  {
    other = 0;
    main  = 0.2;
  }
  if(is_yellow)
  {
    b = other;
    g = (main * 2) / 3;
    r = main;
  }
  else if(is_red)
  {
    b = other;
    g = other;
    r = main;
  }
  else
  {
    b = other;
    g = main;
    r = other;
  }

  if(!this->is_sensitive)
  {
    b /= 3;
    g /= 3;
    r /= 3;
  }

  cr->set_source_rgb(r, g, b);
  cr->arc(width/2, height/2, rayon/2, 0, 6.29);
  cr->fill();
  cr->set_source_rgb(1.0, 1.0, 1.0);
  cr->arc(width/2, height/2, rayon/2, 0, 6.29);
  return true;
}

void GtkLed::set_mutable(bool is_mutable)
{
  this->is_mutable = is_mutable;
}


bool GtkLed::is_on()
{
  return is_lighted;
}

bool GtkLed::on_mouse(GdkEventButton *event)
{
  //printf("on_mouse, is_mut = %d.\n", is_mutable); fflush(0);
  if(is_mutable)
  {

    is_lighted = !is_lighted;
    update_view();
    LedEvent le;
    le.source = this;
    dispatch(le);
  }
  return true;
}

void GtkLed::light(bool on)
{
  is_lighted = on;
  update_view();
}

void GtkLed::set_red(bool is_red)
{
  this->is_red = is_red;
  update_view();
}

void GtkLed::set_sensitive(bool sensistive)
{
  this->is_sensitive = sensistive;
  update_view();
}

void GtkLed::set_yellow()
{
  this->is_yellow = true;
  update_view();
}

/*void GtkLed::on_the_realisation()
{
  wnd = get_window();
  gc = wnd->create_cairo_context();
  realized = true;
  printf("GTKLED : realize.\n"); fflush(0);
  update_view();
}*/

/*bool GtkLed::on_expose_event(GdkEventExpose* event)
{
  update_view();
  return true;
}*/

void GtkLed::update_view()
{
  //queue_draw();
  auto win = get_window();
  if(win)
  {
    auto w = get_allocation().get_width(), h = get_allocation().get_height();
    Gdk::Rectangle r(0, 0, w, h);
    //printf("Invalidate rect %d, %d\n", w, h); fflush(0);
    win->invalidate_rect(r, false);
  }
}


FenetreVisibiliteToggle::FenetreVisibiliteToggle()
{
  lock = false;
  visible = false;
  this->fenetre = nullptr;
}

bool FenetreVisibiliteToggle::est_visible() const
{
  return visible;
}

void FenetreVisibiliteToggle::config(Gtk::Window *fenetre,
          Glib::RefPtr<Gtk::ActionGroup> agroup,
          const std::string &id)
{
  this->fenetre = fenetre;
  this->toggle  = Gtk::ToggleAction::create(id, langue.get_section("menu").get_item(id), "", false);
  agroup->add(toggle, sigc::mem_fun(*this, &FenetreVisibiliteToggle::on_toggle));
  fenetre->signal_delete_event().connect(sigc::mem_fun(*this, &FenetreVisibiliteToggle::gere_evt_delete));
}

bool FenetreVisibiliteToggle::gere_evt_delete(GdkEventAny *evt)
{
  infos("DELETE detecte sur fenetre.");
  visible = false;
  lock = true;
  fenetre->hide();
  toggle->set_active(false);
  lock = false;
  return true;
}

void FenetreVisibiliteToggle::affiche(bool visible_)
{
  infos("Toggle affiche (%d)", (int) visible_);
  visible = visible_;
  lock = true;
  toggle->set_active(visible);
  lock = false;
  if(visible)
  {
    infos("Show (fenetre).");
    fenetre->show();
  }
  else
    fenetre->hide();
}

void FenetreVisibiliteToggle::on_toggle()
{
  if(!lock)
  {
    visible = !visible;
    if(visible)
      fenetre->show();
    else
      fenetre->hide();
  }
}


}
}

namespace utils
{
void Util::show_error(std::string title, std::string content)
{
    Gtk::MessageDialog dial(title, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
    dial.set_title(title);
    dial.set_secondary_text(content);
    dial.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    dial.run();
}

void Util::show_warning(std::string title, std::string content)
{
    Gtk::MessageDialog dial(title, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE, true);
    dial.set_title(title);
    dial.set_secondary_text(content);
    dial.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    dial.run();
}
}




