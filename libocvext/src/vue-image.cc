#include "vue-image.hpp"
#include "ocvext.hpp"


namespace ocvext {



#define DEBOGUE_VUE_IMAGE(AA)
//AA


std::vector<Cairo::RefPtr<Cairo::Context>> *VueImage::old_srf =
    new std::vector<Cairo::RefPtr<Cairo::Context>>();

VueImage::VueImage(uint16_t dx, uint16_t dy, bool dim_from_parent)
  : Gtk::DrawingArea(), dispatcher(16)
{
  cr_alloue = false;
  alloc_x = alloc_y = -1;
  realise  = false;
  csx       = 1;
  csy       = 1;
  this->dim_from_parent = dim_from_parent;

  //signal_size_allocate().
  signal_realize().connect(sigc::mem_fun(*this, &VueImage::on_the_realisation));
  dispatcher.add_listener(this, &VueImage::on_event);

  change_dim_interne(dx,dy);
  if(!dim_from_parent)
    set_size_request(csx,csy);
  //show();
}

void VueImage::get_dim(uint16_t &sx, uint16_t &sy)
{
  sx = get_allocated_width();
  sy = get_allocated_height();
  //infos("get allocation : %d * %d", sx, sy);
  /*if(alloc_x > 0)
  {
    sx = alloc_x;
    sy = alloc_y;
  }*/
}


void VueImage::change_dim_interne(uint16_t sx, uint16_t sy)
{
  if((csx != sx) || (csy != sy))
  {
    DEBOGUE_VUE_IMAGE(infos("change dimension surface : (%d,%d) -> (%d,%d)", csx, csy, sx, sy));
    csx = sx;
    csy = sy;
    //old = image_surface;
    image_surface = Cairo::ImageSurface::create(Cairo::Format::FORMAT_RGB24, sx, sy);
    /*if(en_attente.img.cols > 0)
    {
      DEBOGUE_VUE_IMAGE(infos("Envoi image en attente..."));
      maj_surface(en_attente.img);
    }
    else
      infos("pas d'image en attente");*/
  }
}


void VueImage::change_dim(uint16_t sx, uint16_t sy)
{
  if((csx != sx) || (csy != sy))
  {
    DEBOGUE_VUE_IMAGE(infos("change dimension surface : (%d,%d) -> (%d,%d)", csx, csy, sx, sy));

    csx = sx;
    csy = sy;
    if(!dim_from_parent)
      set_size_request(csx,csy);

    //old = image_surface;
    image_surface = Cairo::ImageSurface::create(Cairo::Format::FORMAT_RGB24, sx, sy);
    /*if(en_attente.img.cols > 0)
    {
      DEBOGUE_VUE_IMAGE(infos("Envoi image en attente..."));
      maj_surface(en_attente.img);
    }
    else
      DEBOGUE_VUE_IMAGE(infos("pas d'image en attente"));*/
  }
}

void VueImage::maj(const std::string &chemin_fichier)
{
  auto img = ocvext::imread(chemin_fichier);

  DEBOGUE_VUE_IMAGE(infos("lecture image debug [%s] => %d * %d", chemin_fichier.c_str(), img.cols, img.rows));

  if(img.cols > 0)
    maj(img);
}

void VueImage::maj(const cv::Mat &img)
{
  cv::Mat img2, img3;

  ocvext::adapte_en_bgr(img, img2);
  if(img2.type() != CV_8UC3)
  {
    avertissement("devrait etre BGR 8 bits, mais type = %d.", img2.type());
    return;
  }
  cv::cvtColor(img2, img3, CV_BGR2BGRA);

  en_attente.img = img3;

  //maj_surface(en_attente.img);

  if(!realise)
  {
    avertissement("maj vue image : non realise.");
    //en_attente.img = img.clone();
    return;
    //return;
  }

  if(dispatcher.is_full())
  {
    avertissement("maj vue image: FIFO de sortie pleine, on va ignorer quelques trames...");
    //return;
    dispatcher.clear();
  }

  Trame t;

  /*if((this->csx == img.cols) && (this->csy == img.rows))
  {
    t.img = cv::Mat();
    memcpy(image_surface->get_data(), img3.ptr(), 4 * img.cols * img.rows);
  }
  else*/
  {
    t.img = img3.clone();
  }
  dispatcher.on_event(t);
}


void VueImage::maj_surface(const cv::Mat &I)
{
  cv::Mat O;
  uint16_t sx, sy;
  get_dim(sx, sy);
  //infos("resize (%d,%d) --> (%d,%d)", I.cols, I.rows, sx, sy);
  affiche_dans_cadre(I, O, cv::Size(sx, sy), cv::Scalar(0));
  ///cv::resize(t.img, O, cv::Size(sx, sy));
  //uint16_t sx = t.img.cols, sy = t.img.rows;
  change_dim_interne(sx,sy);

  DEBOGUE_VUE_IMAGE(infos("memcpy (sx = %d, sy = %d, type = %d)...", sx, sy, O.type());)
  memcpy(this->image_surface->get_data(), O.ptr(), 4 * sx * sy);
  DEBOGUE_VUE_IMAGE(infos("ok"));
}

int VueImage::on_event(const Trame &t)
{
  /*if(t.img.cols != 0)
  {
    maj_surface(t.img);
  }*/
  uint16_t sx, sy;
  get_dim(sx, sy);
  //infos("queue_draw (alloc = %d * %d)...", sx, sy);
  queue_draw();
  //draw_all();
  //queue_draw_area(0, 0, csx, csy);
  //do_update_view();
  //show();
  return 0;
}



void VueImage::draw_all()
{

  //void MyArea::force_redraw()
  /*{
    auto win = get_window();
    if (win)
    {
      Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect(r, false);
    }
  }*/

  DEBOGUE_VUE_IMAGE(infos("draw all -> generation expose event..."));
  GdkEventExpose evt;
  evt.area.x = 0;
  evt.area.y = 0;
  evt.area.width = get_allocation().get_width();
  evt.area.height = get_allocation().get_height();
  gere_expose_event(&evt);
}

// Pas vraiment utilsé
bool VueImage::gere_expose_event(GdkEventExpose* event)
{
  int sx = get_allocated_width();
  int sy = get_allocated_height();
  DEBOGUE_VUE_IMAGE(infos("expose event : alloc = %d * %d", sx, sy));
  do_update_view();
  return true;
}

void VueImage::do_update_view()
{
  DEBOGUE_VUE_IMAGE(trace_verbeuse("do update view..."));
  if(!realise)
  {
    DEBOGUE_VUE_IMAGE(infos(" => non realise."));
    return;
  }

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
  DEBOGUE_VUE_IMAGE(trace_verbeuse("dessine(%d,%d)", csx, csy));

  if(this->en_attente.img.cols > 0)
  {
    maj_surface(en_attente.img);

  //image_surface = Cairo::ImageSurface::create(Cairo::Format::FORMAT_RGB24, csx, csy);
    cr->set_source(image_surface, 0, 0);
    cr->rectangle (0.0, 0.0, csx, csy);
    cr->clip();
    cr->paint();
  }
}

// C'est ici qu'on dessine !
bool VueImage::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{


  uint16_t sx, sy;
  get_dim(sx, sy);

  DEBOGUE_VUE_IMAGE(infos("on draw (allocx = %d, allocy = %d, csx = %d, csy = %d)...", sx, sy, csx, csy));

  if((sx != csx) || (sy != csy))
  {
    if(en_attente.img.cols > 0)
    {
      DEBOGUE_VUE_IMAGE(infos("image en attente : %d * %d, type = %d",
          en_attente.img.cols, en_attente.img.rows, en_attente.img.type()));

      //cv::Mat O, img2, img1;
      //ocvext::adapte_en_bgr(en_attente.img, img1);
      //cv::cvtColor(img1, img2, CV_BGR2BGRA);
      //maj_surface(en_attente.img);

#     if 0
      //infos("resize (%d,%d) --> (%d,%d)", t.img.cols, t.img.rows, sx, sy);
      cv::Mat O, img2, img1;
      ocvext::adapte_en_bgr(en_attente.img, img1);

      cv::cvtColor(img1, img2, CV_BGR2BGRA);
      affiche_dans_cadre(img2, O, cv::Size(sx, sy), cv::Scalar(0));
      //cv::resize(img2, O, cv::Size(sx, sy));
      //uint16_t sx = t.img.cols, sy = t.img.rows;
      change_dim(sx,sy);
      infos("memcpy (sx = %d, sy = %d, type = %d)...", sx, sy, O.type());
      memcpy(this->image_surface->get_data(), O.ptr(), 4 * sx * sy);
#     endif

      //maj(en_attente.img);
      //return true;
      //en_attente.img = cv::Mat();
    }
  }


  //if(!cr_alloue)
  if(!(cr == this->cr))

  {
    DEBOGUE_VUE_IMAGE(trace_majeure("CR different !"));
    if(cr_alloue)
    {
      // Pour contourner bogue GTK sous Windows 10,
      // il ne faut pas supprimer le CR
      // sînon ("")
      old_srf->push_back(this->cr);
      DEBOGUE_VUE_IMAGE(infos("destruction cr..."));
      this->cr.clear();
    }
    DEBOGUE_VUE_IMAGE(infos("copie cr..."));
    cr_alloue = true;
    this->cr = cr;
    DEBOGUE_VUE_IMAGE(infos("copie cr ok."));
  }

  do_update_view();
  return true;
}

void VueImage::on_the_realisation()
{
  uint16_t sx, sy;
  get_dim(sx, sy);
  DEBOGUE_VUE_IMAGE(infos("Vue image : realisation (alloc = %d * %d).", sx, sy));
  realise = true;

  if(sx <= 1)
    // attends l'allocation
    return;

  if(en_attente.img.cols > 0)
  {
    DEBOGUE_VUE_IMAGE(infos("image en attente : %d * %d", en_attente.img.cols, en_attente.img.rows));

    /*cv::Mat O, img2, img1;
    ocvext::adapte_en_bgr(en_attente.img, img1);
    cv::cvtColor(img1, img2, CV_BGR2BGRA);*/


    on_event(en_attente);
    //en_attente.img = cv::Mat(); // Sinon, on perd le fil...
  }
  else
    do_update_view();
}

/*void VueImage::on_size_allocate(Gtk::Allocation &allocation)
{
  alloc_x = allocation.get_width();
  alloc_y = allocation.get_height();
  //set_size_request(alloc_x, alloc_y);
  //set_allocation(allocation);
  infos("Vue image : allocation (%d * %d).", alloc_x, alloc_y);
  if(en_attente.img.cols > 0)
  {
    infos("image en attente : %d * %d", en_attente.img.cols, en_attente.img.rows);
    maj(en_attente.img);
    //draw_all();
    //en_attente.img = cv::Mat();
  }

  //queue_resize();
}*/


DialogueImage::DialogueImage()
{
  dialogue.get_vbox()->pack_start(vview, Gtk::PACK_EXPAND_WIDGET);
  auto img_cancel = new Gtk::Image(Gtk::StockID(Gtk::Stock::CANCEL),
      Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  b_close.set_image(*img_cancel);
  b_close.set_border_width(4);
  hbox.pack_end(b_close, Gtk::PACK_SHRINK);
  hbox.set_layout(Gtk::BUTTONBOX_END);
  b_close.signal_clicked().connect(sigc::mem_fun(this, &DialogueImage::gere_b_fermer));

  dialogue.get_vbox()->pack_start(hbox, Gtk::PACK_SHRINK);
  dialogue.set_size_request(800, 600);
}


void DialogueImage::gere_b_fermer()
{
  dialogue.hide();
}

void DialogueImage::affiche(cv::Mat &I, const std::string &titre, const std::string &infos)
{
  dialogue.set_title(titre);
  dialogue.set_position(Gtk::WIN_POS_CENTER);
  b_close.set_label(utils::langue.get_item("fermer"));

  dialogue.show_all_children(true);
  vview.maj(I);
  DEBOGUE_VUE_IMAGE(infos("dlg.run..."));
  dialogue.run();
}

}


