
#include "wayland.h"
#include "xcb.h"

/*
  wayland-scanner public-code xdg-shell.xml xdg-shell.c
  wayland-scanner client-header xdg-shell.xml xdg-shell.h

  clear && \
    gcc main.cpp xdg-shell.c wayland.cpp utils.cpp xcb.cpp \
    -lwayland-client -lxcb \
    -lstdc++ && \
    ./a.out
*/

int main(void) {
  XcbInitWindow();

  while (!XcbQuit) {
    XcbDispatchWindow();
  }

  XcbTermWindow();
  return 0;
}
