// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009 Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <SDL/SDL.h>

#include <iostream>
#include <boost/thread.hpp>
#include <math.h>
#include <chaiscript/chaiscript.hpp>

void print_help() {
  std::cout << "ChaiScript evaluator.  To evaluate and expression, type it and press <enter>." << std::endl;
  std::cout << "Additionally, you can inspect the runtime system using:" << std::endl;
  std::cout << "  dump_system() - outputs all functions registered to the system" << std::endl;
  std::cout << "  dump_object(x) - dumps information about the given symbol" << std::endl;
}

class Window
{
  private:
    SDL_Surface *screen;

  public:
    Window(int width, int height)
      : screen(SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE))
    {
      if (!screen)
      {
        throw std::runtime_error("Unable to create SDL Surface");
      }
    }

    SDL_Surface *get_screen()
    {
      return screen;
    }

    void draw_pixel(int x, int y, double R, double G, double B)
    {
      int transy = (screen->h - y) - screen->h/2;
      int transx = x + (screen->w / 2);

      if (transx > screen->w || transy > screen->h 
          || transx < 0 || transy < 0) 
      {
        return;
      }


      uint32_t color = SDL_MapRGB(screen->format, uint8_t(255 * R), uint8_t(255 * G), uint8_t(255 * B));

      if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
          return;
        }
      }

      uint32_t *bufp;
      bufp = (uint32_t *)screen->pixels + transy*screen->pitch/4 + transx;
      *bufp = color;

      if ( SDL_MUSTLOCK(screen) ) {
        SDL_UnlockSurface(screen);
      }
      SDL_UpdateRect(screen, transx, transy, 1, 1);
    }
};

void parseinput(chaiscript::ChaiScript &chai)
{
  std::cout << "eval> ";

  std::string input;
  std::getline(std::cin, input);
  while (input != "quit") 
  {
    chaiscript::Boxed_Value val;

    if (input == "help") {
      print_help();
    } else {
      try {
        //First, we evaluate it
        val = chai.eval(input);

        //Then, we try to print the result of the evaluation to the user
        if (val.get_type_info().m_bare_type_info && *(val.get_type_info().m_bare_type_info) != typeid(void)) {
          try {
            chaiscript::dispatch(chai.get_eval_engine().get_function("print"), chaiscript::Param_List_Builder() << val);
          } catch (...) {
            //If we can't, do nothing
          }
        }
      } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
      }
    }

    std::cout << "eval> ";
    std::getline(std::cin, input);
  }
}

void handle_events(Window &w)
{
  bool cont = true;

  while (cont)
  {
    SDL_Event e;

    if (!SDL_WaitEvent(&e))
    {
      cont = false;
    }

    switch (e.type)
    {
      case SDL_ACTIVEEVENT:
        break;
      case SDL_KEYDOWN:
        break;
      case SDL_KEYUP:
        break;
      case SDL_MOUSEMOTION:
        break;
      case SDL_JOYAXISMOTION:
        break;
      case SDL_JOYBALLMOTION:
        break;
      case SDL_JOYHATMOTION:
        break;
      case SDL_JOYBUTTONUP:
        break;
      case SDL_JOYBUTTONDOWN:
        break;
      case SDL_VIDEORESIZE:
        break;
      case SDL_VIDEOEXPOSE:
        SDL_UpdateRect(w.get_screen(), 0, 0, w.get_screen()->w, w.get_screen()->h);
        break;
      case SDL_QUIT:
        cont = false;
        break;
      case SDL_USEREVENT:
        break;
      case SDL_SYSWMEVENT:
        break;
    }
  }
}


int main(int argc, char *argv[]) 
{
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }

  atexit(SDL_Quit);

  chaiscript::ChaiScript chai;

  Window w(200, 200);
  chai.add(chaiscript::user_type<Window>(),"Window");
//  chai.add(chaiscript::constructor<Window (int, int)>(), "Window");
  chai.add(chaiscript::fun(&Window::draw_pixel), "draw_pixel");
  chai.add(chaiscript::var(&w), "window");
  chai.add(chaiscript::fun(&round), "round");
  chai.add(chaiscript::fun(&fabs), "abs");
  chai.add(chaiscript::fun(&abs), "abs");

  if (argc < 2) 
  {
    boost::thread t(boost::bind(&handle_events, boost::ref(w)));
    parseinput(chai);
    t.join();
  } else {
    for (int i = 1; i < argc; ++i) 
    {
      std::string filename(argv[i]);
      try {
        chaiscript::Boxed_Value val = chai.eval_file(argv[i]);
      } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
      }
    }
  }
}

