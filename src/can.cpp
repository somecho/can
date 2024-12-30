#include "App.hpp"

int main(int argc, char* argv[]) {
  Can::App app(argv[1]);
#ifndef PROFILE_STARTUP
  app.run();
#endif
}
