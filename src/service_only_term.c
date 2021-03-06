#include <config.h>
#include <signal.h>
#include <stdio.h>
#include "lisp.h"
#include "blockinput.h"
#include "service_only_term.h"

#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>

#include "coding.h"
#include "frame.h"
#include "fontset.h"
#include "termhooks.h"
#include "termopts.h"
#include "termchar.h"
#include "buffer.h"
#include "window.h"
#include "keyboard.h"
#include "menu.h"


struct so_display_info one_so_display_info;
struct so_display_info *x_display_list;

int
x_display_pixel_height (struct so_display_info *dpyinfo)
{
  return 0;
}

int
x_display_pixel_width (struct so_display_info *dpyinfo)
{
  return 0;
}

Pixmap so_create_pixmap(Display *display,
                        void * d,
                        char *data, unsigned int width,
                        unsigned int height,
                        unsigned long fg,
                        unsigned long bg, unsigned int depth) {
  return 0;
}

unsigned long
so_get_pixel(XImagePtr ximage, int x, int y) {
  return 0;
}

void
so_put_pixel(XImagePtr ximage, int x, int y, unsigned long pixel) {
}

void
so_free_pixmap(XImagePtr ximage) {
}

void
x_set_offset (struct frame *f, register int xoff, register int yoff,
	      int change_gravity) {
}

void
x_focus_frame (struct frame *f, bool noactivate) {
}

Lisp_Object
x_new_font (struct frame *f, Lisp_Object font_object, int fontset) {
  struct font *font = XFONT_OBJECT (font_object);
  int unit, font_ascent, font_descent;

  if (fontset < 0)
    fontset = fontset_from_font (font_object);

  FRAME_FONTSET (f) = fontset;
  if (FRAME_FONT (f) == font)
    /* This font is already set in frame F.  There's nothing more to
       do.  */
    return font_object;

  FRAME_FONT (f) = font;
  FRAME_BASELINE_OFFSET (f) = font->baseline_offset;
  FRAME_COLUMN_WIDTH (f) = unit = font->average_width;
  get_font_ascent_descent (font, &font_ascent, &font_descent);
  FRAME_LINE_HEIGHT (f) = font_ascent + font_descent;

  /* Compute number of scrollbar columns.  */
  unit = FRAME_COLUMN_WIDTH (f);
  if (FRAME_CONFIG_SCROLL_BAR_WIDTH (f) > 0)
    FRAME_CONFIG_SCROLL_BAR_COLS (f)
      = (FRAME_CONFIG_SCROLL_BAR_WIDTH (f) + unit - 1) / unit;
  else
    {
      FRAME_CONFIG_SCROLL_BAR_COLS (f) = (14 + unit - 1) / unit;
      FRAME_CONFIG_SCROLL_BAR_WIDTH (f) =
	FRAME_CONFIG_SCROLL_BAR_COLS (f) * unit;
    }

  /* Now make the frame display the given font.  */
  if (FRAME_X_WINDOW (f) != 0)
    {
      /* Don't change the size of a tip frame; there's no point in
	 doing it because it's done in Fx_show_tip, and it leads to
	 problems because the tip frame has no widget.  */
      if (!FRAME_TOOLTIP_P (f))
	adjust_frame_size (f, FRAME_COLS (f) * FRAME_COLUMN_WIDTH (f),
			   FRAME_LINES (f) * FRAME_LINE_HEIGHT (f), 3,
			   false, Qfont);
    }

  return font_object;
}

void
x_set_scroll_bar_default_width (struct frame *f) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

void
x_set_scroll_bar_default_height (struct frame *f) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

char *
x_get_string_resource (XrmDatabase rdb, const char *name, const char *class) {
  fprintf(stderr, "%s %d n:%s c:%s\n", __FUNCTION__, __LINE__, name, class);
  return 0;
}

void x_clear_under_internal_border (struct frame *f) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

void x_implicitly_set_name (struct frame * f, Lisp_Object arg, Lisp_Object oldval) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

bool x_bitmap_icon (struct frame * f, Lisp_Object arg) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
  return false;
}

char *x_get_keysym_name (int key) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
  return 0;
}

void
x_query_colors (struct frame *f, XColor *colors, int ncolors) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

int
so_defined_color (struct frame *f, const char *color, XColor *color_def,
                  bool alloc_p) {
  return 0;
}

static int so_initialized = 0;
static void
so_initialize (void) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static
void
so_initialize_display_info (Lisp_Object display_name)
{
  struct so_display_info *dpyinfo = &one_so_display_info;

  memset (dpyinfo, 0, sizeof (*dpyinfo));

  dpyinfo->name_list_element = Fcons (display_name, Qnil);
  if (STRINGP (Vsystem_name))
    {
      dpyinfo->so_id_name = xmalloc (SCHARS (Vinvocation_name)
                                     + SCHARS (Vsystem_name) + 2);
      sprintf (dpyinfo->so_id_name, "%s@%s",
               SDATA (Vinvocation_name), SDATA (Vsystem_name));
    }
  else
    dpyinfo->so_id_name = xlispstrdup (Vinvocation_name);

  /* Default Console mode values - overridden when running in GUI mode
     with values obtained from system metrics.  */
  dpyinfo->resx = 1;
  dpyinfo->resy = 1;
  dpyinfo->n_planes = 1;
  dpyinfo->n_cbits = 4;
  dpyinfo->n_fonts = 0;
  dpyinfo->smallest_font_height = 1;
  dpyinfo->smallest_char_width = 1;
  dpyinfo->vertical_scroll_bar_cursor = 0;
  dpyinfo->horizontal_scroll_bar_cursor = 0;
  /* TODO: dpyinfo->gray */

  reset_mouse_highlight (&dpyinfo->mouse_highlight);
}

/* Create an xrdb-style database of resources to supersede registry settings.
   The database is just a concatenation of C strings, finished by an additional
   \0.  The strings are submitted to some basic normalization, so

   [ *]option[ *]:[ *]value...

   becomes

   option:value...

   but any whitespace following value is not removed.  */

static char *
so_make_rdb (char *xrm_option)
{
  char *buffer = xmalloc (strlen (xrm_option) + 2);
  char *current = buffer;
  char ch;
  int in_option = 1;
  int before_value = 0;

  do {
    ch = *xrm_option++;

    if (ch == '\n')
      {
        *current++ = '\0';
        in_option = 1;
        before_value = 0;
      }
    else if (ch != ' ')
      {
        *current++ = ch;
        if (in_option && (ch == ':'))
          {
            in_option = 0;
            before_value = 1;
          }
        else if (before_value)
          {
            before_value = 0;
          }
      }
    else if (!(in_option || before_value))
      {
        *current++ = ch;
      }
  } while (ch);

  *current = '\0';

  return buffer;
}

static
void x_draw_glyph_string (struct glyph_string *s) {
  fprintf(stderr, "soterm draw:%.*ls\n", s->nchars, s->char2b);
}

static void
x_update_window_begin (struct window *w) {
  fprintf(stderr, "update-window-begin\n");
}

static void
x_update_window_end (struct window *w, bool cursor_on_p,
		     bool mouse_face_overwritten_p) {
  fprintf(stderr, "update-window-end\n");
}

static void
x_after_update_window_line (struct window *w, struct glyph_row *desired_row) {
  fprintf(stderr, "x_after_update_window_line\n");
}

static void
x_scroll_run (struct window *w, struct run *run) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
so_clear_frame_area (struct frame *f, int x, int y, int width, int height) {
  fprintf(stderr, "%s %d x=%d, y=%d, w=%d, h=%d\n", __FUNCTION__, __LINE__, x, y, width, height);
}

static void
so_draw_fringe_bitmap (struct window *w, struct glyph_row *row,
			struct draw_fringe_bitmap_params *p) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
so_define_fringe_bitmap (int which, unsigned short *bits, int h, int wd) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
so_destroy_fringe_bitmap (int which) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

extern frame_parm_handler so_frame_parm_handlers[];
static void
x_delete_terminal (struct terminal *terminal);

static struct redisplay_interface so_redisplay_interface =
  {
   so_frame_parm_handlers,
   x_produce_glyphs,
   x_write_glyphs,
   x_insert_glyphs,
   x_clear_end_of_line,
   x_scroll_run,
   x_after_update_window_line,
   x_update_window_begin,
   x_update_window_end,
   0, /* flush_display */
   x_clear_window_mouse_face,
   x_get_glyph_overhangs,
   x_fix_overlapping_area,
   so_draw_fringe_bitmap,
   so_define_fringe_bitmap,
   so_destroy_fringe_bitmap,
   0, //so_compute_glyph_string_overhangs,
   x_draw_glyph_string,
   0, //so_define_frame_cursor,
   so_clear_frame_area,
   0, //so_draw_window_cursor,
   0, //so_draw_vertical_window_border,
   0, //so_draw_window_divider,
   0, //so_shift_glyphs_for_insert,
   0, //so_show_hourglass,
   0, //so_hide_hourglass
  };

void
x_delete_display (struct so_display_info *dpyinfo)
{
  /* FIXME: the only display info apparently can't be deleted.  */
  /* free palette table */
  {
    struct so_palette_entry * plist;

    plist = dpyinfo->color_list;
    while (plist)
      {
        struct so_palette_entry * pentry = plist;
        plist = plist->next;
        xfree (pentry);
      }
    dpyinfo->color_list = NULL;
    if (dpyinfo->palette)
      xfree (dpyinfo->palette);
  }
}

static void
x_clear_frame (struct frame *f) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
x_ins_del_lines (struct frame *f, int vpos, int n)
{
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
x_delete_glyphs (struct frame *f, register int n)
{
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
x_update_begin (struct frame *f) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static void
x_update_end (struct frame *f)
{
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

static struct terminal *
so_create_terminal (struct so_display_info *dpyinfo)
{
  struct terminal *terminal;

  terminal = create_terminal (output_service_only, &so_redisplay_interface);

  terminal->display_info.so = dpyinfo;
  dpyinfo->terminal = terminal;

  /* MSVC does not type K&R functions with no arguments correctly, and
     so we must explicitly cast them.  */
  terminal->clear_frame_hook = x_clear_frame;
  terminal->ins_del_lines_hook = x_ins_del_lines;
  terminal->delete_glyphs_hook = x_delete_glyphs;
  terminal->ring_bell_hook = 0; //so_ring_bell;
  terminal->toggle_invisible_pointer_hook = 0; //so_toggle_invisible_pointer;
  terminal->update_begin_hook = x_update_begin;
  terminal->update_end_hook = x_update_end;
  terminal->read_socket_hook = 0; //so_read_socket;
  terminal->frame_up_to_date_hook = 0; //so_frame_up_to_date;
  terminal->mouse_position_hook = 0; //so_mouse_position;
  terminal->frame_rehighlight_hook = 0; //so_frame_rehighlight;
  terminal->frame_raise_lower_hook = 0; //so_frame_raise_lower;
  terminal->fullscreen_hook = 0; //sofullscreen_hook;
  terminal->menu_show_hook = 0; //so_menu_show;
  terminal->popup_dialog_hook = 0; //so_popup_dialog;
  terminal->set_vertical_scroll_bar_hook = 0; //so_set_vertical_scroll_bar;
  terminal->set_horizontal_scroll_bar_hook = 0; //so_set_horizontal_scroll_bar;
  terminal->condemn_scroll_bars_hook = 0; //so_condemn_scroll_bars;
  terminal->redeem_scroll_bar_hook = 0; //so_redeem_scroll_bar;
  terminal->judge_scroll_bars_hook = 0; //so_judge_scroll_bars;
  terminal->delete_frame_hook = 0; //x_destroy_window;
  terminal->delete_terminal_hook = x_delete_terminal;
  /* Other hooks are NULL by default.  */

  /* We don't yet support separate terminals on service only gui, so don't try to share
     keyboards between virtual terminals that are on the same physical
     terminal like X does.  */
  terminal->kboard = allocate_kboard (QServiceOnlyGui);
  /* Don't let the initial kboard remain current longer than necessary.
     That would cause problems if a file loaded on startup tries to
     prompt in the mini-buffer.  */
  if (current_kboard == initial_kboard)
    current_kboard = terminal->kboard;
  terminal->kboard->reference_count++;

  return terminal;
}

static void
x_delete_terminal (struct terminal *terminal)
{
  struct so_display_info *dpyinfo = terminal->display_info.so;

  /* Protect against recursive calls.  delete_frame in
     delete_terminal calls us back when it deletes our last frame.  */
  if (!terminal->name)
    return;

  block_input ();

  x_delete_display (dpyinfo);
  unblock_input ();
}

struct so_display_info *
so_term_init (Lisp_Object display_name, char *xrm_option, char *resource_name)
{
  struct so_display_info *dpyinfo;
  struct terminal *terminal;

  block_input ();

  if (!so_initialized)
    {
      so_initialize ();
      so_initialized = 1;
    }

  so_initialize_display_info (display_name);

  dpyinfo = &one_so_display_info;
  terminal = so_create_terminal (dpyinfo);

  /* Set the name of the terminal. */
  terminal->name = xlispstrdup (display_name);

  dpyinfo->xrdb = xrm_option ? so_make_rdb (xrm_option) : NULL;

  /* Put this display on the chain.  */
  dpyinfo->next = x_display_list;
  x_display_list = dpyinfo;

  /* initialize palette with white and black */
  {
    XColor color;
    so_defined_color (0, "white", &color, 1);
    so_defined_color (0, "black", &color, 1);
  }

  unblock_input ();

  return dpyinfo;
}

void frame_set_mouse_pixel_position (struct frame *f, int pix_x, int pix_y) {
}

void x_iconify_frame (struct frame *f) {
  SET_FRAME_VISIBLE (f, 0);
  SET_FRAME_ICONIFIED (f, true);
}

void x_make_frame_invisible (struct frame *f) {
  SET_FRAME_VISIBLE (f, 0);
}

void x_make_frame_visible (struct frame *f) {
  SET_FRAME_VISIBLE (f, 1);
  SET_FRAME_ICONIFIED (f, false);
  SET_FRAME_GARBAGED (f);
}

void
x_set_window_size (struct frame *f, bool change_gravity,
		   int width, int height, bool pixelwise) {
  fprintf(stderr, "%s %d %d %d\n", __FUNCTION__, __LINE__, width, height);
}

void syms_of_soterm (void) {
  Fprovide (QServiceOnlyGui, Qnil);
}
