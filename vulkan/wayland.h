#pragma once

#include <wayland-client.h>
#include "xdg-shell.h"

void WlInitWindow();
void WlTermWindow();
void WlDispatchWindow();

extern bool WlQuit;
