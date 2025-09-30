#define TB_IMPL
#include "termbox2.h"

#include <erl_nif.h>
#include <stdint.h>
#include <string.h>

static ERL_NIF_TERM atom_ok;
static ERL_NIF_TERM atom_error;
static ERL_NIF_TERM atom_timeout;
static ERL_NIF_TERM atom_key;
static ERL_NIF_TERM atom_mouse;
static ERL_NIF_TERM atom_resize;
static ERL_NIF_TERM atom_type;
static ERL_NIF_TERM atom_mod;
static ERL_NIF_TERM atom_key_k;
static ERL_NIF_TERM atom_ch;
static ERL_NIF_TERM atom_x;
static ERL_NIF_TERM atom_y;
static ERL_NIF_TERM atom_w;
static ERL_NIF_TERM atom_h;
static ERL_NIF_TERM atom_fg;
static ERL_NIF_TERM atom_bg;
static ERL_NIF_TERM atom_ech;
static ERL_NIF_TERM atom_nech;
static ERL_NIF_TERM atom_cech;
static ERL_NIF_TERM atom_cells;
static ERL_NIF_TERM atom_width_key;
static ERL_NIF_TERM atom_height_key;
static ERL_NIF_TERM atom_len;
static ERL_NIF_TERM atom_codepoint;
static ERL_NIF_TERM atom_string;
static ERL_NIF_TERM atom_ttyfd;
static ERL_NIF_TERM atom_resizefd;
static ERL_NIF_TERM atom_nil;
static ERL_NIF_TERM atom_true;
static ERL_NIF_TERM atom_false;

static ERL_NIF_TERM make_error(ErlNifEnv *env, int code) {
  return enif_make_tuple2(env, atom_error, enif_make_int(env, code));
}

static ERL_NIF_TERM make_ok_value(ErlNifEnv *env, ERL_NIF_TERM value) {
  return enif_make_tuple2(env, atom_ok, value);
}

static ERL_NIF_TERM ok_or_err(ErlNifEnv *env, int status) {
  return (status < 0) ? make_error(env, status) : atom_ok;
}

static ERL_NIF_TERM ok_or_err_with_value(ErlNifEnv *env, int status, ERL_NIF_TERM value) {
  return (status < 0) ? make_error(env, status) : make_ok_value(env, value);
}

static int term_to_c_string(ErlNifEnv *env, ERL_NIF_TERM term, char **out) {
  ErlNifBinary bin;
  if (!enif_inspect_iolist_as_binary(env, term, &bin)) {
    return 0;
  }
  char *buf = enif_alloc(bin.size + 1);
  if (buf == NULL) {
    return 0;
  }
  memcpy(buf, bin.data, bin.size);
  buf[bin.size] = '\0';
  *out = buf;
  return 1;
}

static int term_to_uintattr(ErlNifEnv *env, ERL_NIF_TERM term, uintattr_t *out) {
  unsigned long tmp;
  if (!enif_get_ulong(env, term, &tmp)) {
    unsigned int small;
    if (!enif_get_uint(env, term, &small)) {
      return 0;
    }
    tmp = small;
  }
  *out = (uintattr_t)tmp;
  return 1;
}

static ERL_NIF_TERM make_binary_string(ErlNifEnv *env, const char *str) {
  size_t len = strlen(str);
  ERL_NIF_TERM bin;
  unsigned char *data = enif_make_new_binary(env, len, &bin);
  memcpy(data, str, len);
  return bin;
}

static ERL_NIF_TERM make_cell(ErlNifEnv *env, const struct tb_cell *cell) {
  ERL_NIF_TERM map = enif_make_new_map(env);
  enif_make_map_put(env, map, atom_ch, enif_make_uint(env, cell->ch), &map);
  enif_make_map_put(env, map, atom_fg, enif_make_uint64(env, (unsigned long long)cell->fg), &map);
  enif_make_map_put(env, map, atom_bg, enif_make_uint64(env, (unsigned long long)cell->bg), &map);
#ifdef TB_OPT_EGC
  if (cell->nech > 0 && cell->ech != NULL) {
    size_t count = cell->nech;
    if (count == 0) {
      enif_make_map_put(env, map, atom_ech, enif_make_list(env, 0), &map);
      enif_make_map_put(env, map, atom_nech, enif_make_uint64(env, 0), &map);
      enif_make_map_put(env, map, atom_cech, enif_make_uint64(env, (unsigned long long)cell->cech), &map);
    } else {
      ERL_NIF_TERM *array = enif_alloc(sizeof(ERL_NIF_TERM) * count);
      if (array != NULL) {
        for (size_t i = 0; i < count; i++) {
          array[i] = enif_make_uint(env, cell->ech[i]);
        }
        ERL_NIF_TERM list = enif_make_list_from_array(env, array, count);
        enif_make_map_put(env, map, atom_ech, list, &map);
        enif_make_map_put(env, map, atom_nech, enif_make_uint64(env, (unsigned long long)cell->nech), &map);
        enif_make_map_put(env, map, atom_cech, enif_make_uint64(env, (unsigned long long)cell->cech), &map);
        enif_free(array);
      }
    }
  }
#endif
  return map;
}

static ERL_NIF_TERM make_event(ErlNifEnv *env, const struct tb_event *e) {
  ERL_NIF_TERM map = enif_make_new_map(env);
  ERL_NIF_TERM type_atom =
      (e->type == TB_EVENT_KEY)   ? atom_key   :
      (e->type == TB_EVENT_MOUSE) ? atom_mouse :
      (e->type == TB_EVENT_RESIZE)? atom_resize: enif_make_atom(env, "unknown");
  enif_make_map_put(env, map, atom_type, type_atom, &map);

  switch (e->type) {
    case TB_EVENT_KEY:
      enif_make_map_put(env, map, atom_mod, enif_make_uint(env, e->mod), &map);
      enif_make_map_put(env, map, atom_key_k, enif_make_uint(env, e->key), &map);
      enif_make_map_put(env, map, atom_ch, enif_make_uint(env, e->ch), &map);
      break;
    case TB_EVENT_MOUSE:
      enif_make_map_put(env, map, atom_mod, enif_make_uint(env, e->mod), &map);
      enif_make_map_put(env, map, atom_x, enif_make_int(env, e->x), &map);
      enif_make_map_put(env, map, atom_y, enif_make_int(env, e->y), &map);
      enif_make_map_put(env, map, atom_key_k, enif_make_uint(env, e->key), &map);
      break;
    case TB_EVENT_RESIZE:
      enif_make_map_put(env, map, atom_w, enif_make_int(env, e->w), &map);
      enif_make_map_put(env, map, atom_h, enif_make_int(env, e->h), &map);
      break;
    default:
      break;
  }

  return map;
}

static ERL_NIF_TERM nif_tb_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return ok_or_err(env, tb_init());
}

static ERL_NIF_TERM nif_init_file(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  char *path = NULL;
  if (!term_to_c_string(env, argv[0], &path)) {
    return enif_make_badarg(env);
  }
  int rv = tb_init_file(path);
  enif_free(path);
  return ok_or_err(env, rv);
}

static ERL_NIF_TERM nif_init_fd(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int fd;
  if (!enif_get_int(env, argv[0], &fd)) {
    return enif_make_badarg(env);
  }
  return ok_or_err(env, tb_init_fd(fd));
}

static ERL_NIF_TERM nif_init_rwfd(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int rfd, wfd;
  if (!enif_get_int(env, argv[0], &rfd) || !enif_get_int(env, argv[1], &wfd)) {
    return enif_make_badarg(env);
  }
  return ok_or_err(env, tb_init_rwfd(rfd, wfd));
}

static ERL_NIF_TERM nif_shutdown(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return ok_or_err(env, tb_shutdown());
}

static ERL_NIF_TERM nif_width(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return enif_make_int(env, tb_width());
}

static ERL_NIF_TERM nif_height(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return enif_make_int(env, tb_height());
}

static ERL_NIF_TERM nif_clear(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return ok_or_err(env, tb_clear());
}

static ERL_NIF_TERM nif_set_clear_attrs(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  uintattr_t fg, bg;
  if (!term_to_uintattr(env, argv[0], &fg) || !term_to_uintattr(env, argv[1], &bg)) {
    return enif_make_badarg(env);
  }
  return ok_or_err(env, tb_set_clear_attrs(fg, bg));
}

static ERL_NIF_TERM nif_present(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return ok_or_err(env, tb_present());
}

static ERL_NIF_TERM nif_invalidate(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return ok_or_err(env, tb_invalidate());
}

static ERL_NIF_TERM nif_set_cursor(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int cx, cy;
  if (!enif_get_int(env, argv[0], &cx) || !enif_get_int(env, argv[1], &cy)) {
    return enif_make_badarg(env);
  }
  return ok_or_err(env, tb_set_cursor(cx, cy));
}

static ERL_NIF_TERM nif_hide_cursor(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return ok_or_err(env, tb_hide_cursor());
}

static ERL_NIF_TERM nif_set_cell(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  unsigned int ch;
  uintattr_t fg, bg;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !enif_get_uint(env, argv[2], &ch) ||
      !term_to_uintattr(env, argv[3], &fg) ||
      !term_to_uintattr(env, argv[4], &bg)) {
    return enif_make_badarg(env);
  }
  return ok_or_err(env, tb_set_cell(x, y, (uint32_t)ch, fg, bg));
}

static ERL_NIF_TERM nif_set_cell_ex(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y)) {
    return enif_make_badarg(env);
  }
  ERL_NIF_TERM list = argv[2];
  unsigned int len;
  if (!enif_get_list_length(env, list, &len)) {
    return enif_make_badarg(env);
  }
  uint32_t *codepoints = NULL;
  if (len > 0) {
    codepoints = enif_alloc(sizeof(uint32_t) * len);
    if (codepoints == NULL) {
      return enif_make_badarg(env);
    }
    ERL_NIF_TERM head, tail = list;
    for (unsigned int i = 0; i < len; i++) {
      if (!enif_get_list_cell(env, tail, &head, &tail)) {
        enif_free(codepoints);
        return enif_make_badarg(env);
      }
      unsigned int cp;
      if (!enif_get_uint(env, head, &cp)) {
        enif_free(codepoints);
        return enif_make_badarg(env);
      }
      codepoints[i] = cp;
    }
  }
  uintattr_t fg, bg;
  if (!term_to_uintattr(env, argv[3], &fg) || !term_to_uintattr(env, argv[4], &bg)) {
    if (codepoints) enif_free(codepoints);
    return enif_make_badarg(env);
  }
  int rv = tb_set_cell_ex(x, y, codepoints, len, fg, bg);
  if (codepoints) enif_free(codepoints);
  return ok_or_err(env, rv);
}

static ERL_NIF_TERM nif_extend_cell(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  unsigned int ch;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !enif_get_uint(env, argv[2], &ch)) {
    return enif_make_badarg(env);
  }
  return ok_or_err(env, tb_extend_cell(x, y, (uint32_t)ch));
}

static ERL_NIF_TERM nif_get_cell(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y, back;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !enif_get_int(env, argv[2], &back)) {
    return enif_make_badarg(env);
  }
  struct tb_cell *cell = NULL;
  int rv = tb_get_cell(x, y, back, &cell);
  if (rv < 0) {
    return make_error(env, rv);
  }
  if (cell == NULL) {
    return make_error(env, TB_ERR);
  }
  return make_ok_value(env, make_cell(env, cell));
}

static ERL_NIF_TERM nif_set_input_mode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int mode;
  if (!enif_get_int(env, argv[0], &mode)) {
    return enif_make_badarg(env);
  }
  int rv = tb_set_input_mode(mode);
  if (rv < 0) {
    return make_error(env, rv);
  }
  return make_ok_value(env, enif_make_int(env, rv));
}

static ERL_NIF_TERM nif_set_output_mode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int mode;
  if (!enif_get_int(env, argv[0], &mode)) {
    return enif_make_badarg(env);
  }
  int rv = tb_set_output_mode(mode);
  if (rv < 0) {
    return make_error(env, rv);
  }
  return make_ok_value(env, enif_make_int(env, rv));
}

static ERL_NIF_TERM nif_peek_event(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int timeout_ms;
  if (!enif_get_int(env, argv[0], &timeout_ms)) {
    return enif_make_badarg(env);
  }
  struct tb_event ev;
  int rv = tb_peek_event(&ev, timeout_ms);
  if (rv == 0) {
    return atom_timeout;
  }
  if (rv < 0) {
    return make_error(env, rv);
  }
  return make_ok_value(env, make_event(env, &ev));
}

static ERL_NIF_TERM nif_poll_event(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  struct tb_event ev;
  int rv = tb_poll_event(&ev);
  if (rv < 0) {
    return make_error(env, rv);
  }
  return make_ok_value(env, make_event(env, &ev));
}

static ERL_NIF_TERM nif_get_fds(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  int ttyfd = -1;
  int resizefd = -1;
  int rv = tb_get_fds(&ttyfd, &resizefd);
  if (rv < 0) {
    return make_error(env, rv);
  }
  ERL_NIF_TERM map = enif_make_new_map(env);
  enif_make_map_put(env, map, atom_ttyfd, enif_make_int(env, ttyfd), &map);
  enif_make_map_put(env, map, atom_resizefd, enif_make_int(env, resizefd), &map);
  return make_ok_value(env, map);
}

static ERL_NIF_TERM nif_print(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  uintattr_t fg, bg;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !term_to_uintattr(env, argv[2], &fg) || !term_to_uintattr(env, argv[3], &bg)) {
    return enif_make_badarg(env);
  }
  char *text = NULL;
  if (!term_to_c_string(env, argv[4], &text)) {
    return enif_make_badarg(env);
  }
  int rv = tb_print(x, y, fg, bg, text);
  enif_free(text);
  return ok_or_err(env, rv);
}

static ERL_NIF_TERM nif_print_ex(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  uintattr_t fg, bg;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !term_to_uintattr(env, argv[2], &fg) || !term_to_uintattr(env, argv[3], &bg)) {
    return enif_make_badarg(env);
  }
  char *text = NULL;
  if (!term_to_c_string(env, argv[4], &text)) {
    return enif_make_badarg(env);
  }
  size_t out_w = 0;
  int rv = tb_print_ex(x, y, fg, bg, &out_w, text);
  enif_free(text);
  return ok_or_err_with_value(env, rv, enif_make_uint64(env, (unsigned long long)out_w));
}

static ERL_NIF_TERM nif_printf(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  uintattr_t fg, bg;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !term_to_uintattr(env, argv[2], &fg) || !term_to_uintattr(env, argv[3], &bg)) {
    return enif_make_badarg(env);
  }
  char *fmt = NULL;
  if (!term_to_c_string(env, argv[4], &fmt)) {
    return enif_make_badarg(env);
  }
  int rv = tb_printf(x, y, fg, bg, fmt);
  enif_free(fmt);
  return ok_or_err(env, rv);
}

static ERL_NIF_TERM nif_printf_ex(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int x, y;
  uintattr_t fg, bg;
  if (!enif_get_int(env, argv[0], &x) || !enif_get_int(env, argv[1], &y) ||
      !term_to_uintattr(env, argv[2], &fg) || !term_to_uintattr(env, argv[3], &bg)) {
    return enif_make_badarg(env);
  }
  char *fmt = NULL;
  if (!term_to_c_string(env, argv[4], &fmt)) {
    return enif_make_badarg(env);
  }
  size_t out_w = 0;
  int rv = tb_printf_ex(x, y, fg, bg, &out_w, fmt);
  enif_free(fmt);
  return ok_or_err_with_value(env, rv, enif_make_uint64(env, (unsigned long long)out_w));
}

static ERL_NIF_TERM nif_send(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  ErlNifBinary bin;
  if (!enif_inspect_iolist_as_binary(env, argv[0], &bin)) {
    return enif_make_badarg(env);
  }
  int rv = tb_send((const char *)bin.data, bin.size);
  return ok_or_err(env, rv);
}

static ERL_NIF_TERM nif_sendf(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  char *fmt = NULL;
  if (!term_to_c_string(env, argv[0], &fmt)) {
    return enif_make_badarg(env);
  }
  int rv = tb_sendf(fmt);
  enif_free(fmt);
  return ok_or_err(env, rv);
}

static ERL_NIF_TERM nif_set_func(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int fn_type;
  if (!enif_get_int(env, argv[0], &fn_type)) {
    return enif_make_badarg(env);
  }
  if (enif_is_identical(argv[1], atom_nil)) {
    return ok_or_err(env, tb_set_func(fn_type, NULL));
  }
  return enif_make_badarg(env);
}

static ERL_NIF_TERM nif_utf8_char_length(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  unsigned int byte;
  if (!enif_get_uint(env, argv[0], &byte) || byte > 255) {
    return enif_make_badarg(env);
  }
  return enif_make_int(env, tb_utf8_char_length((char)byte));
}

static ERL_NIF_TERM nif_utf8_char_to_unicode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  ErlNifBinary bin;
  if (!enif_inspect_iolist_as_binary(env, argv[0], &bin)) {
    return enif_make_badarg(env);
  }
  char *buf = enif_alloc(bin.size + 1);
  if (buf == NULL) {
    return enif_make_badarg(env);
  }
  memcpy(buf, bin.data, bin.size);
  buf[bin.size] = '\0';
  uint32_t out = 0;
  int rv = tb_utf8_char_to_unicode(&out, buf);
  enif_free(buf);
  if (rv < 0) {
    return make_error(env, rv);
  }
  ERL_NIF_TERM map = enif_make_new_map(env);
  enif_make_map_put(env, map, atom_len, enif_make_int(env, rv), &map);
  if (rv > 0) {
    enif_make_map_put(env, map, atom_codepoint, enif_make_uint(env, out), &map);
  }
  return make_ok_value(env, map);
}

static ERL_NIF_TERM nif_utf8_unicode_to_char(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  unsigned int codepoint;
  if (!enif_get_uint(env, argv[0], &codepoint)) {
    return enif_make_badarg(env);
  }
  char buf[7];
  int len = tb_utf8_unicode_to_char(buf, (uint32_t)codepoint);
  ERL_NIF_TERM binary;
  unsigned char *dest = enif_make_new_binary(env, len, &binary);
  memcpy(dest, buf, len);
  ERL_NIF_TERM map = enif_make_new_map(env);
  enif_make_map_put(env, map, atom_len, enif_make_int(env, len), &map);
  enif_make_map_put(env, map, atom_string, binary, &map);
  return make_ok_value(env, map);
}

static ERL_NIF_TERM nif_last_errno(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return enif_make_int(env, tb_last_errno());
}

static ERL_NIF_TERM nif_strerror(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  int err;
  if (!enif_get_int(env, argv[0], &err)) {
    return enif_make_badarg(env);
  }
  const char *msg = tb_strerror(err);
  return make_ok_value(env, make_binary_string(env, msg));
}

static ERL_NIF_TERM nif_cell_buffer(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  struct tb_cell *cells = tb_cell_buffer();
  if (cells == NULL) {
    return make_error(env, TB_ERR_NOT_INIT);
  }
  int width = tb_width();
  int height = tb_height();
  if (width < 0 || height < 0) {
    return make_error(env, TB_ERR);
  }
  size_t count = (size_t)width * (size_t)height;
  ERL_NIF_TERM list;
  if (count == 0) {
    list = enif_make_list(env, 0);
  } else {
    ERL_NIF_TERM *array = enif_alloc(sizeof(ERL_NIF_TERM) * count);
    if (array == NULL) {
      return make_error(env, TB_ERR);
    }
    for (size_t i = 0; i < count; i++) {
      array[i] = make_cell(env, &cells[i]);
    }
    list = enif_make_list_from_array(env, array, count);
    enif_free(array);
  }
  ERL_NIF_TERM map = enif_make_new_map(env);
  enif_make_map_put(env, map, atom_width_key, enif_make_int(env, width), &map);
  enif_make_map_put(env, map, atom_height_key, enif_make_int(env, height), &map);
  enif_make_map_put(env, map, atom_cells, list, &map);
  return make_ok_value(env, map);
}

static ERL_NIF_TERM nif_has_truecolor(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return tb_has_truecolor() ? atom_true : atom_false;
}

static ERL_NIF_TERM nif_has_egc(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return tb_has_egc() ? atom_true : atom_false;
}

static ERL_NIF_TERM nif_attr_width(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  return enif_make_int(env, tb_attr_width());
}

static ERL_NIF_TERM nif_version(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc; (void)argv;
  const char *ver = tb_version();
  return make_binary_string(env, ver);
}

static ERL_NIF_TERM nif_iswprint(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  unsigned int ch_val;
  if (!enif_get_uint(env, argv[0], &ch_val)) {
    return enif_make_badarg(env);
  }
  return tb_iswprint((uint32_t)ch_val) ? atom_true : atom_false;
}

static ERL_NIF_TERM nif_wcwidth(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  (void)argc;
  unsigned int ch_val;
  if (!enif_get_uint(env, argv[0], &ch_val)) {
    return enif_make_badarg(env);
  }
  return enif_make_int(env, tb_wcwidth((uint32_t)ch_val));
}

static int load(ErlNifEnv *env, void **priv, ERL_NIF_TERM info) {
  (void)priv; (void)info;
  atom_ok         = enif_make_atom(env, "ok");
  atom_error      = enif_make_atom(env, "error");
  atom_timeout    = enif_make_atom(env, "timeout");
  atom_key        = enif_make_atom(env, "key");
  atom_mouse      = enif_make_atom(env, "mouse");
  atom_resize     = enif_make_atom(env, "resize");
  atom_type       = enif_make_atom(env, "type");
  atom_mod        = enif_make_atom(env, "mod");
  atom_key_k      = enif_make_atom(env, "key");
  atom_ch         = enif_make_atom(env, "ch");
  atom_x          = enif_make_atom(env, "x");
  atom_y          = enif_make_atom(env, "y");
  atom_w          = enif_make_atom(env, "w");
  atom_h          = enif_make_atom(env, "h");
  atom_fg         = enif_make_atom(env, "fg");
  atom_bg         = enif_make_atom(env, "bg");
  atom_ech        = enif_make_atom(env, "ech");
  atom_nech       = enif_make_atom(env, "nech");
  atom_cech       = enif_make_atom(env, "cech");
  atom_cells      = enif_make_atom(env, "cells");
  atom_width_key  = enif_make_atom(env, "width");
  atom_height_key = enif_make_atom(env, "height");
  atom_len        = enif_make_atom(env, "len");
  atom_codepoint  = enif_make_atom(env, "codepoint");
  atom_string     = enif_make_atom(env, "string");
  atom_ttyfd      = enif_make_atom(env, "ttyfd");
  atom_resizefd   = enif_make_atom(env, "resizefd");
  atom_nil        = enif_make_atom(env, "nil");
  atom_true       = enif_make_atom(env, "true");
  atom_false      = enif_make_atom(env, "false");
  return 0;
}

static ErlNifFunc nif_funcs[] = {
  {"init",              0, nif_tb_init,           0},
  {"init_file",         1, nif_init_file,          0},
  {"init_fd",           1, nif_init_fd,            0},
  {"init_rwfd",         2, nif_init_rwfd,          0},
  {"shutdown",          0, nif_shutdown,           0},
  {"width",             0, nif_width,              0},
  {"height",            0, nif_height,             0},
  {"clear",             0, nif_clear,              0},
  {"set_clear_attrs",   2, nif_set_clear_attrs,    0},
  {"present",           0, nif_present,            0},
  {"invalidate",        0, nif_invalidate,         0},
  {"set_cursor",        2, nif_set_cursor,         0},
  {"hide_cursor",       0, nif_hide_cursor,        0},
  {"set_cell",          5, nif_set_cell,           0},
  {"set_cell_ex",       5, nif_set_cell_ex,        0},
  {"extend_cell",       3, nif_extend_cell,        0},
  {"get_cell",          3, nif_get_cell,           0},
  {"set_input_mode",    1, nif_set_input_mode,     0},
  {"set_output_mode",   1, nif_set_output_mode,    0},
  {"peek_event",        1, nif_peek_event,  ERL_NIF_DIRTY_JOB_IO_BOUND},
  {"poll_event",        0, nif_poll_event, ERL_NIF_DIRTY_JOB_IO_BOUND},
  {"get_fds",           0, nif_get_fds,            0},
  {"print",             5, nif_print,              0},
  {"print_ex",          5, nif_print_ex,           0},
  {"printf",            5, nif_printf,             0},
  {"printf_ex",         5, nif_printf_ex,          0},
  {"send",              1, nif_send,               0},
  {"sendf",             1, nif_sendf,              0},
  {"set_func",          2, nif_set_func,           0},
  {"utf8_char_length",  1, nif_utf8_char_length,   0},
  {"utf8_char_to_unicode", 1, nif_utf8_char_to_unicode, 0},
  {"utf8_unicode_to_char", 1, nif_utf8_unicode_to_char, 0},
  {"last_errno",        0, nif_last_errno,         0},
  {"strerror",          1, nif_strerror,           0},
  {"cell_buffer",       0, nif_cell_buffer, ERL_NIF_DIRTY_JOB_CPU_BOUND},
  {"has_truecolor",     0, nif_has_truecolor,      0},
  {"has_egc",           0, nif_has_egc,            0},
  {"attr_width",        0, nif_attr_width,         0},
  {"version",           0, nif_version,            0},
  {"iswprint",          1, nif_iswprint,           0},
  {"wcwidth",           1, nif_wcwidth,            0}
};

ERL_NIF_INIT(Elixir.Termbox2.Native, nif_funcs, load, NULL, NULL, NULL)
