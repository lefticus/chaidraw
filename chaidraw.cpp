
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
#include <gtkmm/colorbutton.h>

#include <sstream>
#include <iostream>
#include <vector>

#include <pangomm/fontdescription.h>
#include <chaiscript/chaiscript.hpp>


struct Color
{
  Color(double t_r, double t_g, double t_b)
    : r(t_r), g(t_g), b(t_b), a(1.0)
  { }

  Color(double t_r, double t_g, double t_b, double t_a)
    : r(t_r), g(t_g), b(t_b), a(t_a)
  { }

  double r;
  double g;
  double b;
  double a;
};

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

  sigc::signal<void, std::string, int, int, int, int> signal_error_changed;
  sigc::signal<void> signal_error_cleared;

protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose* event);
  virtual bool on_button_press_event(GdkEventButton* event);

private:
  void chaiscript_initialize();
  Cairo::RefPtr<Cairo::Context> m_cairo_context;
  std::string m_script;
  chaiscript::ChaiScript m_chai;
  chaiscript::ChaiScript::State m_saved_state;
};


DrawArea::DrawArea()
{
  add_events(Gdk::EXPOSURE_MASK|Gdk::BUTTON_PRESS_MASK );
  chaiscript_initialize();
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

void DrawArea::chaiscript_initialize()
{
  using namespace chaiscript;

  m_chai.add(user_type<Color>(), "Color");
  m_chai.add(fun(&Color::r), "r");
  m_chai.add(fun(&Color::g), "g");
  m_chai.add(fun(&Color::b), "b");
  m_chai.add(fun(&Color::a), "a");
  m_chai.add(bootstrap::copy_constructor<Color>("Color"));
  m_chai.add(constructor<Color (double, double, double)>(), "Color");
  m_chai.add(constructor<Color (double, double, double, double)>(), "Color");

  m_chai.add_global_const(const_var(Color(0,0,0,1)), "Black");
  m_chai.add_global_const(const_var(Color(1,0,0,1)), "Red");
  m_chai.add_global_const(const_var(Color(0,1,0,1)), "Green");
  m_chai.add_global_const(const_var(Color(0,0,1,1)), "Blue");


  m_chai.add(fun(&fabs), "abs");

  m_chai.add(fun(&DrawArea::get_width), "get_width");
  m_chai.add(fun(&DrawArea::get_height), "get_height");

  m_chai.add(fun(&Cairo::Context::arc), "arc");
  m_chai.add(fun(&Cairo::Context::arc_negative), "arc_negative");
  m_chai.add(fun(&Cairo::Context::begin_new_path), "begin_new_path");
  m_chai.add(fun(&Cairo::Context::begin_new_sub_path), "begin_new_sub_path");
  m_chai.add(fun(&Cairo::Context::clip), "clip");
  m_chai.add(fun(&Cairo::Context::clip_preserve), "clip_preserve");
  m_chai.add(fun(&Cairo::Context::close_path), "close_path");
  m_chai.add(fun(&Cairo::Context::curve_to), "curve_to");
  m_chai.add(fun(&Cairo::Context::fill), "fill");
  m_chai.add(fun(&Cairo::Context::fill_preserve), "fill_preserve");
  m_chai.add(fun(&Cairo::Context::line_to), "line_to");
  m_chai.add(fun(&Cairo::Context::move_to), "move_to");
  m_chai.add(fun(&Cairo::Context::paint), "paint");
  m_chai.add(fun(&Cairo::Context::rectangle), "rectangle");
  m_chai.add(fun(&Cairo::Context::restore), "restore");
  m_chai.add(fun(&Cairo::Context::save), "save");
  m_chai.add(fun(&Cairo::Context::stroke), "stroke");
  m_chai.add(fun(&Cairo::Context::stroke_preserve), "stroke_preserve");
  m_chai.add(fun(&Cairo::Context::set_source_rgb), "set_source_rgb");
  m_chai.add(fun(&Cairo::Context::set_source_rgba), "set_source_rgba");

  m_saved_state = m_chai.get_state();
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
    m_chai.set_state(m_saved_state);

    m_chai.add(chaiscript::var(this), "drawingarea");
    m_chai.add(chaiscript::var(m_cairo_context.operator->()), "context");

    m_chai(script);
    signal_error_cleared.emit();
  } catch (const chaiscript::Eval_Error &e) {
    signal_error_changed.emit(e.what(), e.start_position.line, e.start_position.column, 
        e.end_position.line, e.end_position.column);
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
  void on_gobutton_clicked();
  void on_insertcolor_clicked();

  void on_error_changed(const std::string &err, int line, int column, int endline, int endcolumn);
  void on_error_cleared();

  // Child widgets
  Gtk::VBox m_main_box;
  Gtk::VPaned m_main_pain;
  Gtk::HBox m_edit_box;
  Gtk::VBox m_button_box;
  Gtk::HBox m_color_box;

  Gtk::ScrolledWindow m_code_text_scroll;
  Gtk::TextView m_code_text;
  Gtk::Button m_go_btn;
  Gtk::Button m_insert_color_btn;
  Gtk::ColorButton m_color_btn;
  Gtk::Statusbar m_status_bar;

  DrawArea m_area;
};


ChaiDraw::ChaiDraw()
  : m_main_box(/*homogeneous*/false, /*spacing*/0),
    m_edit_box(false, 0), 
    m_button_box(false, 0), 
    m_color_box(false,0),
    m_go_btn("Go"),
    m_insert_color_btn("Insert Color")
{
  m_color_btn.set_use_alpha();

  m_insert_color_btn.signal_clicked().connect(sigc::mem_fun(*this, &ChaiDraw::on_insertcolor_clicked));

  m_color_box.pack_start(m_color_btn, Gtk::PACK_SHRINK, 0);
  m_color_box.pack_start(m_insert_color_btn, Gtk::PACK_SHRINK, 0);

  // box3
  m_button_box.pack_start(m_go_btn, Gtk::PACK_SHRINK, 5);
  m_button_box.pack_start(m_color_box, Gtk::PACK_SHRINK, 5); 
  
  // box2
  m_go_btn.signal_clicked().connect(sigc::mem_fun(*this, &ChaiDraw::on_gobutton_clicked));

  m_area.signal_error_cleared.connect(sigc::mem_fun(*this, &ChaiDraw::on_error_cleared));
  m_area.signal_error_changed.connect(sigc::mem_fun(*this, &ChaiDraw::on_error_changed));

  m_code_text.set_size_request(300, 100);
  m_code_text_scroll.add(m_code_text);
  m_edit_box.pack_start(m_code_text_scroll, Gtk::PACK_EXPAND_WIDGET, 5);
  m_edit_box.pack_start(m_button_box, Gtk::PACK_SHRINK, 5);
  
  // box1
  m_area.set_size_request(300, 300);
  m_main_pain.add1(m_area);
  m_main_pain.add2(m_edit_box);
    
  // box0
  m_main_box.pack_start(m_main_pain, Gtk::PACK_EXPAND_WIDGET, 5);
  m_main_box.pack_start(m_status_bar, Gtk::PACK_SHRINK, 5);
 
  set_border_width(10);
  add(m_main_box);


  m_code_text.get_buffer()->set_text("use(\"draw.chai\");\ncontext.draw_grid(drawingarea);\ncontext.set_color(Red);\ncontext.draw_circle(50,-20, 35);\n");

  show_all();
}


ChaiDraw::~ChaiDraw()
{
}

void ChaiDraw::on_error_changed(const std::string &err, int line, int column, int endline, int endcolumn)
{
  m_status_bar.pop();
  m_status_bar.push(err);

  try {
    Gtk::TextIter itr = m_code_text.get_buffer()->get_iter_at_line_offset(line-1, column-1);
    Gtk::TextIter itr2 = m_code_text.get_buffer()->get_iter_at_line_offset(endline-1, endcolumn-1);

    if (itr == itr2)
    {
      ++itr2;
    }

    m_code_text.get_buffer()->select_range(itr, itr2);

    m_code_text.scroll_to(itr, 0, .5, 0);
  } catch (const std::exception &) {
  }
}

void ChaiDraw::on_error_cleared()
{
  m_status_bar.pop();
}

void ChaiDraw::on_gobutton_clicked()
{
  m_area.set_script(m_code_text.get_buffer()->get_text());
  m_area.queue_draw();
}

void ChaiDraw::on_insertcolor_clicked()
{
  Gdk::Color color = m_color_btn.get_color();

  std::stringstream ss;
  ss<<"context.set_color("
    << color.get_red_p()
    << ", " << color.get_green_p()
    << ", " << color.get_blue_p()
    << ", " << m_color_btn.get_alpha()/65535.0 << ");";

  m_code_text.get_buffer()->insert_at_cursor(ss.str());
}

int main(int argc, char** argv)
{
  Gtk::Main main_instance (argc, argv);

  ChaiDraw chaidraw;
  Gtk::Main::run(chaidraw);

  return 0;
}

