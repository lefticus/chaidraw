
#include <gtkmm/drawingarea.h>
#include <gdkmm/colormap.h>
#include <gdkmm/window.h>


#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>
#include <gtkmm/main.h>
#include <gtkmm/paned.h>

#include <sstream>
#include <iostream>
#include <vector>

#include <pangomm/fontdescription.h>
#include <chaiscript/chaiscript.hpp>

class DrawArea : public Gtk::DrawingArea
{
public:
  DrawArea();
  virtual ~DrawArea();
  bool timer_callback();

  void set_script(const std::string &s)
  {
    m_script = s;
  }

  void run_script(const std::string &s);

  void draw_circle(double x, double y, double radius, double r, double g, double b, double a);

  double get_width();
  double get_height();

  sigc::signal<void, std::string, int, int> signal_error_changed;
  sigc::signal<void> signal_error_cleared;

protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose* event);
  virtual bool on_button_press_event(GdkEventButton* event);

private:
  Cairo::RefPtr<Cairo::Context> m_cairo_context;
  std::string m_script;
};


DrawArea::DrawArea()
{
  add_events(Gdk::EXPOSURE_MASK|Gdk::BUTTON_PRESS_MASK );
}


DrawArea::~DrawArea()
{
}


void DrawArea::on_realize()
{
  // We need to call the base on_realize()
  Gtk::DrawingArea::on_realize();
}

bool DrawArea::on_button_press_event(GdkEventButton*e)
{
  return true;
}

double DrawArea::get_width()
{
  return get_allocation().get_width();
}

double DrawArea::get_height()
{
  return get_allocation().get_height();
}

void DrawArea::run_script(const std::string &script)
{
  // we need a ref to the gdkmm window
  Glib::RefPtr<Gdk::Window> window = get_window();

  m_cairo_context = window->create_cairo_context();
  m_cairo_context->save();
  Cairo::Matrix matrix(1, 0, 0, -1, double(get_width())/2, double(get_height()/2));
  m_cairo_context->set_matrix(matrix);
  m_cairo_context->set_source_rgba(1.0,1.0,1.0, 1.0);  
  m_cairo_context->paint();

  try {
    using namespace chaiscript;
    chaiscript::ChaiScript chai;

    chai.add(fun(&fabs), "abs");

    chai.add(chaiscript::var(this), "drawingarea");
    chai.add(chaiscript::var(m_cairo_context.operator->()), "context");

    chai.add(fun(&DrawArea::get_width), "get_width");
    chai.add(fun(&DrawArea::get_height), "get_height");

    chai.add(fun(&Cairo::Context::arc), "arc");
    chai.add(fun(&Cairo::Context::arc_negative), "arc_negative");
    chai.add(fun(&Cairo::Context::begin_new_path), "begin_new_path");
    chai.add(fun(&Cairo::Context::begin_new_sub_path), "begin_new_sub_path");
    chai.add(fun(&Cairo::Context::clip), "clip");
    chai.add(fun(&Cairo::Context::clip_preserve), "clip_preserve");
    chai.add(fun(&Cairo::Context::close_path), "close_path");
    chai.add(fun(&Cairo::Context::curve_to), "curve_to");
    chai.add(fun(&Cairo::Context::fill), "fill");
    chai.add(fun(&Cairo::Context::fill_preserve), "fill_preserve");
    chai.add(fun(&Cairo::Context::line_to), "line_to");
    chai.add(fun(&Cairo::Context::move_to), "move_to");
    chai.add(fun(&Cairo::Context::paint), "paint");
    chai.add(fun(&Cairo::Context::rectangle), "rectangle");
    chai.add(fun(&Cairo::Context::restore), "restore");
    chai.add(fun(&Cairo::Context::save), "save");
    chai.add(fun(&Cairo::Context::stroke), "stroke");
    chai.add(fun(&Cairo::Context::stroke_preserve), "stroke_preserve");
    chai.add(fun(&Cairo::Context::set_source_rgb), "set_source_rgb");
    chai.add(fun(&Cairo::Context::set_source_rgba), "set_source_rgba");

    chai(script);
    signal_error_cleared.emit();
  } catch (const chaiscript::Eval_Error &e) {
    signal_error_changed.emit(e.what(), e.position.line, e.position.column);
  }


  m_cairo_context->restore();
  m_cairo_context.clear();
}

bool DrawArea::on_expose_event(GdkEventExpose*)
{
  run_script(m_script);
  return true;
}



class ChaiDraw : public Gtk::Window
{
public:
  ChaiDraw();
  virtual ~ChaiDraw();
  
protected:
  //signal handlers:
  void on_button1_clicked();

  void on_error_changed(const std::string &err, int line, int column);
  void on_error_cleared();

  // Child widgets
  Gtk::VBox m_box0;
  Gtk::VPaned m_box1;
  Gtk::HBox m_box2;

  Gtk::ScrolledWindow m_scrolledwindow;
  Gtk::TextView m_entry;
  Gtk::Button m_button1;
  Gtk::Statusbar m_sb;

  DrawArea m_area;
};


ChaiDraw::ChaiDraw()
  : m_box0(/*homogeneous*/false, /*spacing*/5), m_box1(), m_box2(false, 5), 
    m_scrolledwindow(),
    m_entry(),
    m_button1("Go"), 
    m_sb(), 
    m_area()
{
  // box2
  m_button1.signal_clicked().connect(sigc::mem_fun(*this, &ChaiDraw::on_button1_clicked));

  m_area.signal_error_cleared.connect(sigc::mem_fun(*this, &ChaiDraw::on_error_cleared));
  m_area.signal_error_changed.connect(sigc::mem_fun(*this, &ChaiDraw::on_error_changed));

  m_entry.set_size_request(300, 100);
  m_scrolledwindow.add(m_entry);
  m_box2.pack_start(m_scrolledwindow, Gtk::PACK_EXPAND_WIDGET, 5);
  m_box2.pack_start(m_button1, Gtk::PACK_SHRINK, 5);
  
  // box1
  m_area.set_size_request(300, 300);
  m_box1.add1(m_area);
  m_box1.add2(m_box2);
    
  // box0
  m_box0.pack_start(m_box1, Gtk::PACK_EXPAND_WIDGET, 5);
  m_box0.pack_start(m_sb, Gtk::PACK_SHRINK, 5);
 
  set_border_width(10);
  add(m_box0);


  m_entry.get_buffer()->set_text("use(\"draw.chai\");\ncontext.draw_grid(drawingarea);\ncontext.set_color(1.0,.25,.75,.70);\ncontext.draw_circle(50,-20, 35);\n");

  show_all();
}


ChaiDraw::~ChaiDraw()
{
}

void ChaiDraw::on_error_changed(const std::string &err, int line, int column)
{
  m_sb.pop();
  m_sb.push(err);

  Gtk::TextIter itr = m_entry.get_buffer()->get_iter_at_line_offset(line-1, column-1);
  Gtk::TextIter itr2(itr);
  ++itr2;

  m_entry.get_buffer()->select_range(itr, itr2);

  m_entry.scroll_to(itr, 0, .5, 0);
}

void ChaiDraw::on_error_cleared()
{
  m_sb.pop();
}



void ChaiDraw::on_button1_clicked()
{
  m_area.set_script(m_entry.get_buffer()->get_text());
  m_area.queue_draw();
}


int main(int argc, char** argv)
{
  Gtk::Main main_instance (argc, argv);

  ChaiDraw chaidraw;
  Gtk::Main::run(chaidraw);

  return 0;
}

