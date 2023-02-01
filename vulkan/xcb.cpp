#include "xcb.h"

#include <xcb/xcb.h>

#include "utils.h"

bool XcbQuit = false;

static xcb_connection_t *connection;
static xcb_screen_t *screen;
static xcb_window_t window;

void XcbInitWindow() {
  int n;
  connection = DIE_NULL(xcb_connect(NULL, &n));
  DIE_NE_0(xcb_connection_has_error(connection));

  const xcb_setup_t *setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  while (n-- > 0) xcb_screen_next(&iter);
  screen = iter.data;

  window = xcb_generate_id(connection);

  uint32_t value_list[32];
  value_list[0] = screen->black_pixel;
  value_list[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0,
                    0, buffer_width, buffer_height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
                    XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, value_list);
  xcb_map_window(connection, window);
  DIE_LE_0(xcb_flush(connection));
}

void XcbTermWindow() {
  xcb_destroy_window(connection, window);
  xcb_disconnect(connection);
}

void XcbDispatchWindow() {
  xcb_generic_event_t *ev = xcb_wait_for_event(connection);
  if (ev == NULL) {
    XcbQuit = true;
    return;
  }
  printf("%d\n", ev->response_type);
  free(ev);
}
