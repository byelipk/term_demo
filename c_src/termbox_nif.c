// termbox2 wants feature-test macros defined before including any system headers.
// We provided them via CFLAGS, but we also include the header FIRST as suggested.
#define TB_IMPL              // compile the single-header library into this NIF
#include "termbox2.h"        // vendored at c_src/vendor/termbox2.h

#include <erl_nif.h>
#include <stdint.h>

static ERL_NIF_TERM atom_ok, atom_error, atom_timeout;
static ERL_NIF_TERM atom_key, atom_mouse, atom_resize;
static ERL_NIF_TERM atom_type, atom_mod, atom_key_k, atom_ch, atom_x, atom_y, atom_w, atom_h;

static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
  atom_ok      = enif_make_atom(env, "ok");
  atom_error   = enif_make_atom(env, "error");
  atom_timeout = enif_make_atom(env, "timeout");
  atom_key     = enif_make_atom(env, "key");
  atom_mouse   = enif_make_atom(env, "mouse");
  atom_resize  = enif_make_atom(env, "resize");
  atom_type    = enif_make_atom(env, "type");
  atom_mod     = enif_make_atom(env, "mod");
  atom_key_k   = enif_make_atom(env, "key");
  atom_ch      = enif_make_atom(env, "ch");
  atom_x       = enif_make_atom(env, "x");
  atom_y       = enif_make_atom(env, "y");
  atom_w       = enif_make_atom(env, "w");
  atom_h       = enif_make_atom(env, "h");
  return 0;
}

static ERL_NIF_TERM ok_or_err(ErlNifEnv* env, int r) {
  return (r < 0) ? enif_make_tuple2(env, atom_error, enif_make_int(env, r)) : atom_ok;
}

static ERL_NIF_TERM nif_tb_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int r = tb_init();
  return ok_or_err(env, r);
}

static ERL_NIF_TERM nif_shutdown(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int r = tb_shutdown();
  return ok_or_err(env, r);
}

static ERL_NIF_TERM nif_clear(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  return ok_or_err(env, tb_clear());
}

static ERL_NIF_TERM nif_present(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  return ok_or_err(env, tb_present());
}

static ERL_NIF_TERM nif_width(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  return enif_make_int(env, tb_width());
}

static ERL_NIF_TERM nif_height(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  return enif_make_int(env, tb_height());
}

// set_cell(x, y, ch, fg, bg) -> :ok | {:error, code}
static ERL_NIF_TERM nif_set_cell(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int x, y;
  unsigned int ch;
  unsigned int fg, bg; // termbox2 uses uintattr_t for attrs; map from uint in BEAM
  if (!enif_get_int(env, argv[0], &x))  return enif_make_badarg(env);
  if (!enif_get_int(env, argv[1], &y))  return enif_make_badarg(env);
  if (!enif_get_uint(env, argv[2], &ch)) return enif_make_badarg(env);
  if (!enif_get_uint(env, argv[3], &fg)) return enif_make_badarg(env);
  if (!enif_get_uint(env, argv[4], &bg)) return enif_make_badarg(env);

  int r = tb_set_cell(x, y, (uint32_t)ch, (uintattr_t)fg, (uintattr_t)bg);
  return ok_or_err(env, r);
}

static ERL_NIF_TERM make_event(ErlNifEnv* env, struct tb_event* e) {
  ERL_NIF_TERM map = enif_make_new_map(env);
  ERL_NIF_TERM type_atom =
    (e->type == TB_EVENT_KEY)    ? atom_key :
    (e->type == TB_EVENT_MOUSE)  ? atom_mouse :
    (e->type == TB_EVENT_RESIZE) ? atom_resize :
                                   enif_make_atom(env, "unknown");
  enif_make_map_put(env, map, atom_type, type_atom, &map);

  switch (e->type) {
    case TB_EVENT_KEY:
      enif_make_map_put(env, map, atom_mod,   enif_make_uint(env, e->mod), &map);
      enif_make_map_put(env, map, atom_key_k, enif_make_uint(env, e->key), &map);
      enif_make_map_put(env, map, atom_ch,    enif_make_uint(env, e->ch),  &map);
      break;
    case TB_EVENT_MOUSE:
      enif_make_map_put(env, map, atom_mod,   enif_make_uint(env, e->mod), &map);
      enif_make_map_put(env, map, atom_x,     enif_make_int(env, e->x),    &map);
      enif_make_map_put(env, map, atom_y,     enif_make_int(env, e->y),    &map);
      enif_make_map_put(env, map, atom_key_k, enif_make_uint(env, e->key), &map);
      break;
    case TB_EVENT_RESIZE:
      enif_make_map_put(env, map, atom_w,     enif_make_int(env, e->w),    &map);
      enif_make_map_put(env, map, atom_h,     enif_make_int(env, e->h),    &map);
      break;
    default: break;
  }
  return map;
}

static ERL_NIF_TERM nif_peek_event(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int timeout_ms;
  if (!enif_get_int(env, argv[0], &timeout_ms)) return enif_make_badarg(env);
  struct tb_event ev;
  int r = tb_peek_event(&ev, timeout_ms);
  if (r == 0)  return atom_timeout;
  if (r <  0)  return enif_make_tuple2(env, atom_error, enif_make_int(env, r));
  return enif_make_tuple2(env, atom_ok, make_event(env, &ev));
}

static ERL_NIF_TERM nif_poll_event(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  struct tb_event ev;
  int r = tb_poll_event(&ev);
  if (r < 0) return enif_make_tuple2(env, atom_error, enif_make_int(env, r));
  return enif_make_tuple2(env, atom_ok, make_event(env, &ev));
}

static ErlNifFunc nif_funcs[] = {
  {"init",       0, nif_tb_init},
  {"shutdown",   0, nif_shutdown},
  {"clear",      0, nif_clear},
  {"present",    0, nif_present},
  {"width",      0, nif_width},
  {"height",     0, nif_height},
  {"set_cell",   5, nif_set_cell},
  {"peek_event", 1, nif_peek_event, ERL_NIF_DIRTY_JOB_IO_BOUND},
  {"poll_event", 0, nif_poll_event, ERL_NIF_DIRTY_JOB_IO_BOUND}
};

ERL_NIF_INIT(Elixir.TermDemo, nif_funcs, load, NULL, NULL, NULL)

