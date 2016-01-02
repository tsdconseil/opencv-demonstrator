#ifndef COLORCELLRENDERER2_H_
#define COLORCELLRENDERER2_H_

#include <gdkmm.h>
#include <gtkmm/cellrenderer.h>
#include <glibmm/property.h>
#include "mmi/ColorCellEditable2.hpp"

namespace utils
{
namespace mmi
{

class ColorCellRenderer2 : public Gtk::CellRenderer
{
public:
	ColorCellRenderer2(std::vector<std::string> constraints);
	virtual ~ColorCellRenderer2();

	// Properties
	Glib::PropertyProxy< Glib::ustring > property_text();
	Glib::PropertyProxy< bool > property_editable();
	
	// Edited signal
	typedef sigc::signal<void, const Glib::ustring&, const Glib::ustring& > signal_edited_t;
	signal_edited_t& signal_edited() { return signal_edited_; };
	
protected:
	std::vector<std::string> constraints;
	Glib::Property<Glib::ustring> property_text_;
	Glib::Property<bool> property_editable_;
	signal_edited_t signal_edited_;
	ColorCellEditable2* color_cell_edit_ptr_;
	mutable int button_width_; //mutable because it is just a cache for get_size_vfunc

	// Raise the edited event
	void edited(const Glib::ustring& path, const Glib::ustring& new_text);
	
	virtual void get_size_vfunc (Gtk::Widget& widget, const Gdk::Rectangle* cell_area, int* x_offset, int* y_offset, int* width, int* height) const;

	// TODO
	//virtual void render_vfunc (const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, const Gdk::Rectangle& expose_area, Gtk::CellRendererState flags);

	virtual bool activate_vfunc (GdkEvent*, Gtk::Widget&, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);
	virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);
	
	// Manage editing_done event for color_cell_edit_ptr_
	void on_editing_done();
};

}
}

#endif /*COLORCELLRENDERER2_H_*/
