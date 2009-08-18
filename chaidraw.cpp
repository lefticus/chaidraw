
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

void DrawArea::draw_circle(double x, double y, double radius, double r, double g, double b, double a)
{
  m_cairo_context->set_source_rgba(r,g,b,a);
  m_cairo_context->arc(x,y,radius, 0.0, 2 * M_PI);
  m_cairo_context->fill();
}

bool DrawArea::on_button_press_event(GdkEventButton*e)
{
  return true;
}

void DrawArea::run_script(const std::string &script)
{
  // we need a ref to the gdkmm window
  Glib::RefPtr<Gdk::Window> window = get_window();

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();

  double displayedwidth;
  double displayedheight;

  if (width < height)
  {
    displayedwidth = 1;
    displayedheight = 1*height/width;
  } else {
    displayedheight = 1;
    displayedwidth = 1*width/height;
  }

  m_cairo_context = window->create_cairo_context();
  m_cairo_context->save();
  Cairo::Matrix matrix(1, 0, 0, -1, double(width)/2, double(height/2));
  m_cairo_context->set_matrix(matrix);
  m_cairo_context->set_source_rgba(1.0,1.0,1.0, 1.0);  
  m_cairo_context->paint();


  m_cairo_context->set_source_rgba(0,0,0,.15);

  m_cairo_context->move_to(-double(width)/2, 0);
  m_cairo_context->line_to(double(width)/2, 0);
  m_cairo_context->stroke();
  
  m_cairo_context->move_to(0, double(height)/2);
  m_cairo_context->line_to(0, -double(height)/2);
  m_cairo_context->stroke();

  m_cairo_context->set_source_rgba(0,0,0,.04);
  for (int x = 20; x < width/2; x += 20)
  {
    m_cairo_context->move_to(x, double(height)/2);
    m_cairo_context->line_to(x, -double(height)/2);
    m_cairo_context->stroke();

    m_cairo_context->move_to(-x, double(height)/2);
    m_cairo_context->line_to(-x, -double(height)/2);
    m_cairo_context->stroke();
  }

  for (int y = 20; y < height/2; y += 20)
  {
    m_cairo_context->move_to(double(width)/2, y);
    m_cairo_context->line_to(-double(width)/2, y);
    m_cairo_context->stroke();

    m_cairo_context->move_to(double(width)/2, -y);
    m_cairo_context->line_to(-double(width)/2, -y);
    m_cairo_context->stroke();
  }

  try {
    using namespace chaiscript;
    chaiscript::ChaiScript chai;
    chai.add(fun(&fabs), "abs");
    chai.add(fun(&DrawArea::draw_circle), "draw_circle");
    chai.add(chaiscript::var(this), "drawarea");
    chai(script);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
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

  // Child widgets
  Gtk::VBox m_box0;
  Gtk::VBox m_box1;
  Gtk::HBox m_box2;

  Gtk::ScrolledWindow m_scrolledwindow;
  Gtk::TextView m_entry;
  Gtk::Button m_button1;
  Gtk::Statusbar m_sb;

  DrawArea m_area;
};


ChaiDraw::ChaiDraw()
  : m_box0(/*homogeneous*/false, /*spacing*/5), m_box1(false, 5), m_box2(false, 5), 
    m_scrolledwindow(),
    m_entry(),
    m_button1("Go"), 
    m_sb(), 
    m_area()
{
  // box2
  m_button1.signal_clicked().connect(sigc::mem_fun(*this, &ChaiDraw::on_button1_clicked));

  m_entry.set_size_request(300, 100);
  m_scrolledwindow.add(m_entry);
  m_box2.pack_start(m_scrolledwindow, Gtk::PACK_EXPAND_WIDGET, 5);
  m_box2.pack_start(m_button1, Gtk::PACK_SHRINK, 5);
  
  // box1
  m_area.set_size_request(300, 300);
  m_box1.pack_start(m_area, Gtk::PACK_EXPAND_WIDGET, 5);
  m_box1.pack_start(m_box2, Gtk::PACK_SHRINK, 5);
    
  // box0
  m_box0.pack_start(m_box1, Gtk::PACK_EXPAND_WIDGET, 5);
  m_box0.pack_start(m_sb, Gtk::PACK_SHRINK, 5);
 
  set_border_width(10);
  add(m_box0);

  show_all();
}


ChaiDraw::~ChaiDraw()
{
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

