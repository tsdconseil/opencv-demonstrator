#ifndef COLORCELLEDITABLE2_H_
#define COLORCELLEDITABLE2_H_

#include <gtkmm/celleditable.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/box.h>
#include <gdkmm/color.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>

#include <gtkmm/drawingarea.h>
#include <cairomm/context.h>

namespace utils
{
namespace mmi
{

class ColorArea : public Gtk::DrawingArea
{
public:
	ColorArea()
	{};
	virtual ~ColorArea()
	{};
	
	Gdk::Color get_color() const { return color_; };
	void set_color(const Gdk::Color& value) { color_ = value; };
	
	bool on_expose_event(GdkEventExpose* event)
	{
		  // This is where we draw on the window
		  Glib::RefPtr<Gdk::Window> window = get_window();
		  if(window)
		  {
		    Gtk::Allocation allocation = get_allocation();
		    const int width = allocation.get_width();
		    const int height = allocation.get_height();

		    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

		    // clip to the area indicated by the expose event so that we only redraw
		    // the portion of the window that needs to be redrawn
		    cr->rectangle(event->area.x, event->area.y,
		            event->area.width, event->area.height);
		    cr->clip();

		    cr->rectangle(0, 0, width, height);
		    cr->set_source_rgb (color_.get_red_p(), color_.get_green_p(), color_.get_blue_p());
		    cr->fill();
		  }

		  return true;		
	}
	
protected:
	Gdk::Color color_;
};

class ColorCellEditable2 : public Gtk::EventBox, public Gtk::CellEditable
{
public:
	// Ctor/Dtor
	ColorCellEditable2(const Glib::ustring& path, std::vector<std::string> constraints);
	virtual ~ColorCellEditable2();
	
	// Return creation path
	Glib::ustring get_path() const;

	// Get and set text
	void set_text(const Glib::ustring& text);
	Glib::ustring get_text() const;
	
	// Editing cancelled
	bool get_editing_cancelled() const { return editing_cancelled_; };
	
	// Return button width
	static int get_button_width();
	
	// Return ColorArea width
	static int get_color_area_width();
	
	// Signal for editing done
	typedef sigc::signal<void> signal_editing_done_t;
	signal_editing_done_t& signal_editing_done() { return signal_editing_done_; };
	
protected:
	std::vector<std::string> constraints;
	Glib::ustring path_;
	ColorArea* color_area_ptr_;
	Gtk::Entry* entry_ptr_;
	Gtk::Button* button_ptr_;
	Gdk::Color color_;
	bool editing_cancelled_;
	signal_editing_done_t signal_editing_done_;
	
	
	/* override */virtual void start_editing_vfunc(GdkEvent* event);
	/* override */virtual void on_editing_done();
	/* override */virtual void on_entry_activate();
	
	// Manage button_clicked signal
	virtual void on_button_clicked();
	
	// Manage entry_key_press_event signal
	bool on_entry_key_press_event(GdkEventKey* event);
};

}
}

#endif /*COLORCELLEDITABLE2_H_*/
