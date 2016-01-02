#include "mmi/renderers.hpp"
#include "mmi/stdview.hpp"
#include "comm/serial.hpp"

#include <string.h>
//#include <malloc.h>
#include <stdlib.h>
#include <limits.h>
#include <memory>
#include <iostream>

namespace utils
{
namespace mmi
{

RefCellEditable::RefCellEditable(const Glib::ustring& path/*, 
                                                            Node model, std::string ref_name*/) :
	Glib::ObjectBase( typeid(RefCellEditable) ),
	Gtk::EventBox(),
	Gtk::CellEditable(),
	path_( path ),
	entry_ptr_( 0 ),
	button_ptr_( 0 ),
	editing_cancelled_( false )
{
  setup("view/refcelleditable");
  //this->model = model;
  //this->ref_name = ref_name;
  Gtk::HBox *const hbox = new Gtk::HBox(false, 0);
  add (*Gtk::manage( hbox ));

  // Find model and ref from path
  // TODO
  /*Gtk::TreeModel::iterator iter = tree_model->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    //setup_row_view(row[columns.m_col_ptr]);
    update_view();
    }*/
	
  //entry_ptr_ = new Gtk::Entry();
  //hbox->pack_start (*Gtk::manage(entry_ptr_), Gtk::PACK_EXPAND_WIDGET);
  //entry_ptr_->set_has_frame (false);
  //entry_ptr_->gobj()->is_cell_renderer = true;
  
  entry_ptr_ = new Gtk::Label();
  entry_ptr_->set_label("essai");

  Gtk::Alignment *align1 = new Gtk::Alignment(0,0,1,1);
  //Gtk::Alignment *align1 = new Gtk::Alignment(Gtk::ALIGN_START, Gtk::ALIGN_START, 1, 1);//(0,0,1,1);
  //Gtk::Alignment *align2 = new Gtk::Alignment(Gtk::ALIGN_START, Gtk::ALIGN_START, 1, 1);//(0,0,1,1);

  align1->add(*entry_ptr_);
  hbox->pack_start (*align1, Gtk::PACK_EXPAND_WIDGET);
  //hbox->pack_start (*Gtk::manage(entry_ptr_), Gtk::EXPAND_WIDGET);
  //entry_ptr_->set_has_frame (false);
  //entry_ptr_->gobj()->is_cell_renderer = true;
  
  button_ptr_ = new Gtk::Button();
  hbox->pack_start (*Gtk::manage( button_ptr_ ), Gtk::PACK_SHRINK);
  button_ptr_->add (*Gtk::manage(new Gtk::Arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT)));
  
  set_can_focus(true);
  
  show_all_children();
}

RefCellEditable::~RefCellEditable()
{
}

Glib::ustring RefCellEditable::get_path() const
{
  return path_;
}

void RefCellEditable::set_text(const Glib::ustring& text)
{
  entry_ptr_->set_label(text);
}

Glib::ustring RefCellEditable::get_text() const
{
  return "TODO";//model.get_reference(ref_name).get_localized().get_localized();
}

int RefCellEditable::get_button_width()
{
	Gtk::Window window (Gtk::WINDOW_POPUP);

	Gtk::Button *const button = new Gtk::Button();
	window.add(*Gtk::manage(button));

	button->add(*Gtk::manage(new Gtk::Arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT)));
	window.move(-500, -500);
	window.show_all();
	//Gtk::Requisition requisition = window.size_request();
	//return requisition.width;
	int min,nat;
	//Gtk::Requisition r0,r1;
	window.get_preferred_width(min,nat);
	return nat;
}

/* static */ int RefCellEditable::get_color_area_width()
{
	return 16;
}

void RefCellEditable::start_editing_vfunc(GdkEvent*)
{
  //entry_ptr_->select_region(0, -1);
	//entry_ptr_->signal_activate().connect(sigc::mem_fun(*this, &RefCellEditable::on_entry_activate));
	//entry_ptr_->signal_key_press_event().connect(sigc::mem_fun(*this, &RefCellEditable::on_entry_key_press_event));
	
	button_ptr_->signal_clicked().connect (sigc::mem_fun( *this, &RefCellEditable::on_button_clicked ));
}

void RefCellEditable::on_editing_done()
{
  //std::cout << "on_editing_done " << editing_cancelled_ << std::endl;
	
  /*if (!editing_cancelled_)
  {
    int r = 0;
    int g = 0;
    int b = 0;
	
    
    std::string s = entry_ptr_->get_text();
    ByteArray ba(s);
    r = ba[0];
    g = ba[1];
    b = ba[2];
	  
    color_.set_rgb(r * 256, g * 256, b * 256);
    }*/
  
  signal_editing_done_.emit();
}

void RefCellEditable::on_button_clicked()
{
  trace("show ref. explorer..");

  if(model.is_nullptr())
  {
    anomaly("on_button_clicked(): nullptr model");
    return;
  }

  RefSchema *rs = model.schema()->get_reference(ref_name);
  if(rs != nullptr)
  {
    trace("find root..");

    Node root = model.get_child(rs->path);

    if(!root.is_nullptr())
    {
      root.log.trace("root found.");
      RefExplorerWnd *re = new RefExplorerWnd(root, rs->child_str);
      if(re->display() == 0)
      {
        trace("update ref..");
        model.set_reference(ref_name, re->get_selection());
      }
    }
    else
    {
      anomaly("root not found.");
    }
  }
  else
  {
    anomaly("ref schema not found");
  }
  signal_editing_done_.emit();
}

bool RefCellEditable::on_entry_key_press_event(GdkEventKey* event)
{
  if (event->keyval == GDK_KEY_Escape)
  {
    editing_cancelled_ = true;
		
    editing_done();
    remove_widget();
    return true;
  }	
  return false;
}

void RefCellEditable::on_entry_activate()
{
  if(!appli_view_prm.use_touchscreen)
    editing_done();
}



RefCellRenderer::RefCellRenderer(/*Node model, std::string ref_name*/) :
	Glib::ObjectBase( typeid(RefCellRenderer) ),
	Gtk::CellRenderer(),
	property_text_( *this, "text", "" ),
	property_editable_( *this, "editable", true ),
	color_cell_edit_ptr_( 0 ),
	button_width_( -1 )
{
  //this->model = model;
  //this->ref_name = ref_name;
  property_mode() = Gtk::CELL_RENDERER_MODE_EDITABLE;
  property_xpad() = 2;
  property_ypad() = 2;
}

RefCellRenderer::~RefCellRenderer()
{
}

Glib::PropertyProxy< Glib::ustring > RefCellRenderer::property_text()
{
  return property_text_.get_proxy();
}

Glib::PropertyProxy< bool > RefCellRenderer::property_editable()
{
	return property_editable_.get_proxy();
}

/* override */void RefCellRenderer::get_size_vfunc (Gtk::Widget& widget, const Gdk::Rectangle* cell_area, int* x_offset, int* y_offset, int* width, int* height) const
{
  // We cache this because it takes a really long time to get the width.
  if(button_width_ < 0)
    button_width_ = RefCellEditable::get_button_width();
	
  // Compute text width
  Glib::RefPtr<Pango::Layout> layout_ptr = widget.create_pango_layout (property_text_);
    Pango::Rectangle rect = layout_ptr->get_pixel_logical_extents();
	
    const int calc_width  = property_xpad() * 4 + rect.get_width();
    const int calc_height = property_ypad() * 4 + rect.get_height();

    // Add button width and color area width
    if( width )
        *width = calc_width + button_width_ + RefCellEditable::get_color_area_width();

    if( height )
        *height = calc_height;    
}

// TODO
#if 0
void RefCellRenderer::render_vfunc (const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, const Gdk::Rectangle& expose_area, Gtk::CellRendererState flags)
{
	// Get cell size
    int x_offset = 0, y_offset = 0, width = 0, height = 0;
    get_size (widget, cell_area, x_offset, y_offset, width, height);
    
    // Create the graphic context
    Glib::RefPtr< Cairo::Context > gc = Cairo::Context::create (window);
    
    // Get cell state
    //Gtk::StateType state;
    Gtk::StateType text_state;
    if ((flags & Gtk::CELL_RENDERER_SELECTED) != 0)
    {
        //state = Gtk::STATE_SELECTED;
        text_state = (widget.has_focus()) ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;
    }
    else
    {
        //state = Gtk::STATE_NORMAL;
        text_state = (widget.is_sensitive()) ? Gtk::STATE_NORMAL : Gtk::STATE_INSENSITIVE;
    }

    // Draw color text
    Glib::RefPtr< Gdk::Window > win = Glib::RefPtr<Gdk::Window>::cast_dynamic (window);    
    Glib::RefPtr<Pango::Layout> layout_ptr = widget.create_pango_layout( property_text_ );
    widget.get_style()->paint_layout (win,
          text_state,
          true,
          cell_area,
          widget,
          "cellrenderertext",
                                      cell_area.get_x() /*+ RefCellEditable::get_color_area_width()*/ + x_offset + /*2 **/ property_xpad(),
          cell_area.get_y() + y_offset + 2 * property_ypad(),
          layout_ptr);    
}
#endif

bool RefCellRenderer::activate_vfunc (GdkEvent*, Gtk::Widget&, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags)
{
  return true;
}

void RefCellEditable::setup_model(Node model, std::string ref_name)
{
  trace("Setup model %s ok!", ref_name.c_str());
  trace("model = %s.", model.to_xml().c_str());
  this->model = model;
  this->ref_name = ref_name;
}

Gtk::CellEditable* RefCellRenderer::start_editing_vfunc(GdkEvent *event, 
                                                        Gtk::Widget &widget, 
                                                        const Glib::ustring &path, 
                                                        const Gdk::Rectangle &background_area, 
                                                        const Gdk::Rectangle &cell_area, 
                                                        Gtk::CellRendererState flags)
{
  trace("start editing..");
  //StringListView *slv = (StringListView *) &widget;
  

  if(appli_view_prm.use_touchscreen)
    return 0;

#ifdef GLIBMM_PROPERTIES_ENABLED
  if (!property_editable())
    return 0;
#else
  if (!(g_object_get_data(G_OBJECT(gobj()), "editable")))
    return 0;
#endif
		  
  std::auto_ptr<RefCellEditable> color_cell_edit_ptr(new RefCellEditable(path));
	
  Glib::ustring text;
		
#ifdef GLIBMM_PROPERTIES_ENABLED
  text = property_text();
#else
  get_property("text", text);
#endif

  color_cell_edit_ptr->set_text (text);
  color_cell_edit_ptr->signal_editing_done().connect(sigc::mem_fun(*this, &RefCellRenderer::on_editing_done));
  color_cell_edit_ptr->show();
	
  color_cell_edit_ptr_ = Gtk::manage( color_cell_edit_ptr.release() );
  return color_cell_edit_ptr_;
}

void RefCellRenderer::edited(const Glib::ustring& path, const Glib::ustring& new_text)
{
  signal_edited_.emit (path, new_text);
}

void RefCellRenderer::on_editing_done()
{
  if (color_cell_edit_ptr_->get_editing_cancelled())
  {
    stop_editing (true);
  }
  else
  {
    edited (color_cell_edit_ptr_->get_path(), color_cell_edit_ptr_->get_text());
  }
}


static bool has_child(NodeSchema *root, std::string type, std::vector<NodeSchema *> &already_checked)
{
  unsigned int i;

  for(i = 0; i < already_checked.size(); i++)
  {
    if(already_checked[i] == root)
      return false;
  }
  already_checked.push_back(root);


  if(root->name.get_id().compare(type) == 0)
    return true;
  for(i = 0; i < root->children.size(); i++)
  {
    if(root->children[i].name.get_id().compare(type) == 0)
      return true;
    if(has_child(root->children[i].ptr, type, already_checked))
      return true;
  }
  return false;
}

static bool has_child(NodeSchema *root, std::string type)
{
  std::vector<NodeSchema *> already_checked;
  return has_child(root, type, already_checked);
}


RefExplorerWnd::RefExplorerWnd(Node model, const std::string &ref_name, const std::string &wnd_title):
  GenDialog(GenDialog::GEN_DIALOG_VALID_CANCEL), explorer(model, ref_name)
{
  setup("view/ref-explorer");
  if(wnd_title.size() > 0)
    set_title(wnd_title);
  else
    set_title(langue.get_item("sel-ref"));
  this->model    = model;
  this->ref_name = ref_name;

  vbox->pack_start(explorer, Gtk::PACK_EXPAND_WIDGET);

  explorer.add_listener(this, &RefExplorerWnd::on_change);

  set_size_request(500, 400);
  update_view();
  show_all_children(true);
}

int RefExplorerWnd::display()
{
  return display_modal();
}

int RefExplorerWnd::on_change(const RefExplorerChange &change)
{
  update_view();
  return 0;
}

void RefExplorerWnd::update_view()
{
  bool selection_ok = false;
  Node sel = get_selection();
  if(!sel.is_nullptr())
  {
    if((sel.schema()->name.get_id().compare(ref_name) == 0) || (ref_name.size() == 0))
      selection_ok = true;
  }
  enable_validation(selection_ok);
}



Node RefExplorerWnd::get_selection()
{
  return explorer.get_selection();
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

RefExplorer::RefExplorer()
  : log("libcutil/ref-explorer")
{
  valid = false;
  init_done = false;
}

void RefExplorer::setup(Node model, 
                        std::string ref_name, 
                        const std::string &title)
{
  this->model    = model;
  this->ref_name = ref_name;

  set_label(title);

  if(!init_done)
  {
    tree_view.set_headers_visible(false);
    scroll.add(tree_view);
    scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scroll.set_border_width(5);
    add(scroll);
  }

  tree_model = Gtk::TreeStore::create(columns);
  tree_view.set_model(tree_model);

  Gtk::TreeViewColumn *tvc = Gtk::manage(new Gtk::TreeViewColumn());
  Gtk::CellRendererText *crt = new Gtk::CellRendererText();
  tvc->pack_start(*crt, true);
  tvc->add_attribute(crt->property_markup(), columns.m_col_name);
  tree_view.append_column(*tvc);

  tree_view.signal_row_activated().connect(sigc::mem_fun(*this,
      &RefExplorer::on_treeview_row_activated));

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = tree_view.get_selection();
  refTreeSelection->signal_changed().connect(
      sigc::mem_fun(*this, &RefExplorer::on_selection_changed));

  populate();

  update_view();
  show_all_children(true);

  init_done = true;
}

RefExplorer::RefExplorer(Node model, std::string ref_name, const std::string &title) 
  : log("libcutil/ref-explorer")
{
  valid = false;
  init_done = false;
  setup(model, ref_name, title);
}

static bool has_such_child(Node root, const string &type)
{
  unsigned int i, j, n, m;

  if(root.schema()->name.get_id().compare(type) == 0)
    return true;

  if(root.has_child(type))
  {
    cout << "HAS SUCH CHILD: " << type << endl << root.to_xml() << endl;
    return true;
  }
  
  n = root.schema()->children.size();
  
  for(i = 0; i < n; i++)
  {
    string sname = root.schema()->children[i].name.get_id();

    m = root.get_children_count(sname);

    for(j = 0; j < m; j++)
    {
      if(has_such_child(root.get_child_at(sname, j), type))
        return true;
    }
  }
  return false;
}


void RefExplorer::populate()
{
  unsigned int i, j, n;
  tree_model->clear();
  for(i = 0; i < model.schema()->children.size(); i++)
  {
    SubSchema ss = model.schema()->children[i];
    NodeSchema *schema = ss.ptr;

    bool candidate = has_child(schema, ref_name) || (ref_name.size() == 0);

    log.trace("schema[%s]: candidate = %s.", schema->name.get_id().c_str(), candidate ? "true" : "false");
    
    if(candidate)
    {
      n = model.get_children_count(schema->name.get_id());
      for(j = 0; j < n; j++)
      {
        Node ch = model.get_child_at(schema->name.get_id(), j);

        if((ref_name.size() == 0) || has_such_child(ch, ref_name) )
        {
          Gtk::TreeModel::Row subrow = *(tree_model->append());
          subrow[columns.m_col_name] = ch.get_identifier(false);
          subrow[columns.m_col_ptr] = ch;
          populate(ch, subrow);
        }
      }
    }
  }

  tree_view.expand_all();
}

void RefExplorer::populate(Node root, Gtk::TreeModel::Row row)
{
  unsigned int i, j, n;
  for(i = 0; i < root.schema()->children.size(); i++)
  {
    SubSchema ss = root.schema()->children[i];
    NodeSchema *schema = ss.ptr;

    n = root.get_children_count(schema->name.get_id());
    for(j = 0; j < n; j++)
    {
      Node ch = root.get_child_at(schema->name.get_id(), j);
      if((ref_name.size() == 0) || has_such_child(ch, ref_name))
      {
        Gtk::TreeModel::Row subrow = *(tree_model->append(row.children()));
        subrow[columns.m_col_name] = ch.get_identifier(false);
        subrow[columns.m_col_ptr] = ch;
        populate(ch, subrow);
      }
    }
  }
}

void RefExplorer::update_view()
{
  valid = false;
  Node sel = get_selection();
  if(!sel.is_nullptr())
  {
    if((sel.schema()->name.get_id().compare(ref_name) == 0) || (ref_name.size() == 0))
      valid = true;
  }
}

bool RefExplorer::is_valid()
{
  return valid;
}

void RefExplorer::clear_table()
{
  
}

void RefExplorer::on_treeview_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *)
{
  Gtk::TreeModel::iterator iter = tree_model->get_iter(path);
  if(iter)
  {
    //Gtk::TreeModel::Row row = *iter;
    //setup_row_view(row[columns.m_col_ptr]);
    update_view();

    RefExplorerChange ch;
    CProvider<RefExplorerChange>::dispatch(ch);
  }
}

Node RefExplorer::get_selection()
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

void RefExplorer::on_selection_changed()
{
  update_view();
  RefExplorerChange ch;
  CProvider<RefExplorerChange>::dispatch(ch);
}

}
}

