#include "wayland.h"

#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>

#include "utils.h"

bool WlQuit = false;

static wl_display *display;
static wl_registry *registry;

static wl_seat *seat = NULL;
static wl_shm *shm = NULL;
static wl_compositor *compositor = NULL;
static xdg_wm_base *shell = NULL;

static wl_surface *surface;
static xdg_surface *shell_surface;
static xdg_toplevel *toplevel;
static wl_pointer *pointer;

static void null_func() {}

static void global_add(void *data, struct wl_registry *wl_registry,
                       uint32_t name, const char *interface, uint32_t version) {
  if (strcmp(wl_compositor_interface.name, interface) == 0) {
    compositor = (wl_compositor *)wl_registry_bind(registry, name,
                                                   &wl_compositor_interface, 4);
  } else if (strcmp(xdg_wm_base_interface.name, interface) == 0) {
    shell = (xdg_wm_base *)wl_registry_bind(registry, name,
                                            &xdg_wm_base_interface, 1);
  } else if (strcmp(wl_shm_interface.name, interface) == 0) {
    shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
  } else if (strcmp(wl_seat_interface.name, interface) == 0) {
    seat = (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface,
                                       version);
  }
}

static wl_registry_listener registry_listener = {
    global_add,
    (void (*)(void *data, wl_registry *wl_registry, uint32_t name))null_func};

static void wm_base_listener_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                                  uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static xdg_wm_base_listener wm_base_listener = {wm_base_listener_ping};

static void pointer_listener_button(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, uint32_t time, uint32_t button,
                             uint32_t state) {
  printf("pointer_listener_button: %u %u %u %u\n", serial, time, button, state);
  if (button == BTN_LEFT) {
    xdg_toplevel_move(toplevel, seat, serial);
  } else if (button == BTN_RIGHT && state == WL_POINTER_BUTTON_STATE_RELEASED) {
    WlQuit = true;
  }
}
static void pointer_listener_axis(void *data, struct wl_pointer *wl_pointer,
                           uint32_t time, uint32_t axis, wl_fixed_t value) {
  printf("pointer_listener_axis: %u, %u, %d\n", time, axis, value);
}
static wl_pointer_listener pointer_listener = {
    (void (*)(void *data, wl_pointer *wl_pointer, uint32_t serial,
              wl_surface *surface, wl_fixed_t surface_x,
              wl_fixed_t surface_y))null_func,
    (void (*)(void *data, wl_pointer *wl_pointer, uint32_t serial,
              wl_surface *surface))null_func,
    (void (*)(void *data, wl_pointer *wl_pointer, uint32_t time,
              wl_fixed_t surface_x, wl_fixed_t surface_y))null_func,
    pointer_listener_button,
    pointer_listener_axis,
    (void (*)(void *data, wl_pointer *wl_pointer))null_func,
    (void (*)(void *data, wl_pointer *wl_pointer,
              uint32_t axis_source))null_func,
    (void (*)(void *data, wl_pointer *wl_pointer, uint32_t time,
              uint32_t axis))null_func,
    (void (*)(void *data, wl_pointer *wl_pointer, uint32_t axis,
              int32_t discrete))null_func,
};

static int buffer_fd = -1;
static void *buffer_mem = NULL;
static wl_buffer *buffer;
static const int buffer_size = buffer_width * buffer_height * 4;
static const int buffer_stride = buffer_width * 4;
static const int buffer_format = WL_SHM_FORMAT_XRGB8888;

static void attach_buffer() {
  char file[512];
  snprintf(file, sizeof(file), "/tmp/%d-wayland-XXXXXX", (int)getpid());
  buffer_fd = DIE_LT_0(mkostemp(file, O_CLOEXEC));
  unlink(file);
  DIE_NE_0(ftruncate(buffer_fd, buffer_size));
  buffer_mem = DIE_NULL(mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, buffer_fd, 0));
  memset(buffer_mem, 255, buffer_size);

  wl_shm_pool *pool = DIE_NULL(wl_shm_create_pool(shm, buffer_fd, buffer_size));
  buffer = DIE_NULL(wl_shm_pool_create_buffer(pool, 0, buffer_width, buffer_height, buffer_stride, buffer_format));
  wl_shm_pool_destroy(pool);

  wl_surface_commit(surface);
  DIE_LT_0(wl_display_dispatch(display));

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_commit(surface);
}
static void clear_buffer() {
  wl_buffer_destroy(buffer);
  close(buffer_fd);
  munmap(buffer_mem, buffer_size);
}

void WlInitWindow() {
  display = DIE_NULL(wl_display_connect(NULL));
  registry = DIE_NULL(wl_display_get_registry(display));
  DIE_NE_0(wl_registry_add_listener(registry, &registry_listener, NULL));
  DIE_LT_0(wl_display_roundtrip(display));

  DIE_NULL(shm);
  DIE_NULL(compositor);
  DIE_NULL(shell);
  DIE_NULL(seat);

  DIE_NE_0(xdg_wm_base_add_listener(shell, &wm_base_listener, NULL));

  surface = DIE_NULL(wl_compositor_create_surface(compositor));
  shell_surface = DIE_NULL(xdg_wm_base_get_xdg_surface(shell, surface));
  toplevel = DIE_NULL(xdg_surface_get_toplevel(shell_surface));
  pointer = DIE_NULL(wl_seat_get_pointer(seat));
  DIE_NE_0(wl_pointer_add_listener(pointer, &pointer_listener, NULL));

  attach_buffer();
}

void WlTermWindow() {
  clear_buffer();

  wl_pointer_destroy(pointer);
  xdg_toplevel_destroy(toplevel);
  xdg_surface_destroy(shell_surface);
  wl_surface_destroy(surface);

  wl_seat_destroy(seat);
  xdg_wm_base_destroy(shell);
  wl_compositor_destroy(compositor);
  wl_shm_destroy(shm);

  wl_registry_destroy(registry);
  wl_display_disconnect(display);
}

void WlDispatchWindow() {
  DIE_LT_0(wl_display_dispatch(display));
}
