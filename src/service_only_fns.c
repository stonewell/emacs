#include <config.h>

#include <c-ctype.h>
#include <stdio.h>

#include "lisp.h"
#include "service_only_term.h"
#include "frame.h"
#include "window.h"
#include "buffer.h"
#include "keyboard.h"
#include "blockinput.h"
#include "coding.h"

/**
 * x_hide_tip:
 *
 * Hide currently visible tooltip and cancel its timer.
 *
 * This will try to make tooltip_frame invisible (if DELETE is false)
 * or delete tooltip_frame (if DELETE is true).
 *
 * Return Qt if the tooltip was either deleted or made invisible, Qnil
 * otherwise.
 */
static Lisp_Object
x_hide_tip (bool delete)
{
  return Qnil;
}

DEFUN ("x-hide-tip", Fx_hide_tip, Sx_hide_tip, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  return x_hide_tip (!tooltip_reuse_hidden_frame);
}

DEFUN ("xw-display-color-p", Fxw_display_color_p, Sxw_display_color_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  if ((dpyinfo->n_planes * dpyinfo->n_cbits) <= 2)
    return Qnil;

  return Qt;
}

DEFUN ("x-display-grayscale-p", Fx_display_grayscale_p,
       Sx_display_grayscale_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  if ((dpyinfo->n_planes * dpyinfo->n_cbits) <= 1)
    return Qnil;

  return Qt;
}

DEFUN ("x-display-pixel-width", Fx_display_pixel_width,
       Sx_display_pixel_width, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  return make_fixnum (x_display_pixel_width (dpyinfo));
}

DEFUN ("x-display-pixel-height", Fx_display_pixel_height,
       Sx_display_pixel_height, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  return make_fixnum (x_display_pixel_height (dpyinfo));
}

DEFUN ("x-display-planes", Fx_display_planes, Sx_display_planes,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  return make_fixnum (dpyinfo->n_planes * dpyinfo->n_cbits);
}

DEFUN ("x-display-color-cells", Fx_display_color_cells, Sx_display_color_cells,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);
  int cap;

  /* Don't use NCOLORS: it returns incorrect results under remote
   * desktop.  We force 24+ bit depths to 24-bit, both to prevent an
   * overflow and because probably is more meaningful on Windows
   * anyway.  */

  cap = 1 << min (dpyinfo->n_planes * dpyinfo->n_cbits, 24);
  return make_fixnum (cap);
}

DEFUN ("x-server-max-request-size", Fx_server_max_request_size,
       Sx_server_max_request_size,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  return make_fixnum (1);
}

GC
x_create_gc (struct frame *f,
	     unsigned long mask,
	     XGCValues *xgcv)
{
  GC gc = xmalloc (sizeof *gc);
  *gc = *xgcv;
  return gc;
}

void
x_free_gc (struct frame *f, GC gc)
{
  xfree (gc);
}

struct so_display_info *
check_x_display_info (Lisp_Object object)
{
  if (NILP (object))
    {
      struct frame *sf = XFRAME (selected_frame);

      if (FRAME_SO_P (sf) && FRAME_LIVE_P (sf))
	return FRAME_DISPLAY_INFO (sf);
      else {
  fprintf(stderr, "%d\n", __LINE__);
	return &one_so_display_info;
      }
    }
  else if (TERMINALP (object))
    {
      struct terminal *t = decode_live_terminal (object);

      if (t->type != output_service_only)
	error ("Terminal %d is not a service only display", t->id);

      return t->display_info.so;
    }
  else if (STRINGP (object)) {
  fprintf(stderr, "%d\n", __LINE__);
    return x_display_info_for_name (object);
  }
  else
    {
      struct frame *f;

      CHECK_LIVE_FRAME (object);
      f = XFRAME (object);
      if (! FRAME_SO_P (f))
	error ("Non-ServiceOnly frame used");
      return FRAME_DISPLAY_INFO (f);
    }
}

DEFUN ("x-create-frame", Fx_create_frame, Sx_create_frame,
       1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
     (Lisp_Object parameters) {
  struct frame *f;
  Lisp_Object frame, tem;
  Lisp_Object name;
  bool minibuffer_only = false;
  long window_prompting = 0;
  ptrdiff_t count = SPECPDL_INDEX ();
  Lisp_Object display;
  struct so_display_info *dpyinfo = NULL;
  Lisp_Object parent, parent_frame;
  struct kboard *kb;
  int x_width = 0, x_height = 0;

  if (!FRAME_SO_P (SELECTED_FRAME ())
      && !FRAME_INITIAL_P (SELECTED_FRAME ()))
    error ("Cannot create a GUI frame in a -nw session");

  /* Make copy of frame parameters because the original is in pure
     storage now. */
  parameters = Fcopy_alist (parameters);

  /* Use this general default value to start with
     until we know if this frame has a specified name.  */
  Vx_resource_name = Vinvocation_name;

  display = x_get_arg (dpyinfo, parameters, Qterminal, 0, 0, RES_TYPE_NUMBER);
  if (EQ (display, Qunbound))
    display = x_get_arg (dpyinfo, parameters, Qdisplay, 0, 0, RES_TYPE_STRING);
  if (EQ (display, Qunbound))
    display = Qnil;
  dpyinfo = check_x_display_info (display);
  kb = dpyinfo->terminal->kboard;

  if (!dpyinfo->terminal->name)
    error ("Terminal is not live, can't create new frames on it");

  name = x_get_arg (dpyinfo, parameters, Qname, "name", "Name", RES_TYPE_STRING);
  if (!STRINGP (name)
      && ! EQ (name, Qunbound)
      && ! NILP (name))
    error ("Invalid frame name--not a string or nil");

  if (STRINGP (name))
    Vx_resource_name = name;

  /* See if parent window is specified.  */
  parent = x_get_arg (dpyinfo, parameters, Qparent_id, NULL, NULL,
		      RES_TYPE_NUMBER);
  if (EQ (parent, Qunbound))
    parent = Qnil;
  else if (!NILP (parent))
    CHECK_FIXNUM (parent);

  /* make_frame_without_minibuffer can run Lisp code and garbage collect.  */
  /* No need to protect DISPLAY because that's not used after passing
     it to make_frame_without_minibuffer.  */
  frame = Qnil;
  tem = x_get_arg (dpyinfo, parameters, Qminibuffer, "minibuffer", "Minibuffer",
		   RES_TYPE_SYMBOL);
  if (EQ (tem, Qnone) || NILP (tem))
    f = make_frame_without_minibuffer (Qnil, kb, display);
  else if (EQ (tem, Qonly))
    {
      f = make_minibuffer_frame ();
      minibuffer_only = true;
    }
  else if (WINDOWP (tem))
    f = make_frame_without_minibuffer (tem, kb, display);
  else
    f = make_frame (true);

  XSETFRAME (frame, f);

 /* Initialize `default-minibuffer-frame' in case this is the first
     frame on this terminal.  */
  if (FRAME_HAS_MINIBUF_P (f)
      && (!FRAMEP (KVAR (kb, Vdefault_minibuffer_frame))
	  || !FRAME_LIVE_P (XFRAME (KVAR (kb, Vdefault_minibuffer_frame)))))
    kset_default_minibuffer_frame (kb, frame);

  /* All remaining specified parameters, which have not been "used"
     by x_get_arg and friends, now go in the misc. alist of the frame.  */
  for (tem = parameters; CONSP (tem); tem = XCDR (tem))
    if (CONSP (XCAR (tem)) && !NILP (XCAR (XCAR (tem))))
      fset_param_alist (f, Fcons (XCAR (tem), f->param_alist));

  /* Make sure windows on this frame appear in calls to next-window
     and similar functions.  */
  Vwindow_list = Qnil;

  return unbind_to (count, frame);
}

frame_parm_handler so_frame_parm_handlers[] =
{
  x_set_autoraise,
  x_set_autolower,
  0, //x_set_background_color,
  0, //x_set_border_color,
  0, //x_set_border_width,
  0, //x_set_cursor_color,
  0, //x_set_cursor_type,
  0, //x_set_font,
  0, //x_set_foreground_color,
  0, //x_set_icon_name,
  0, //x_set_icon_type,
  0, //x_set_internal_border_width,
  x_set_right_divider_width,
  x_set_bottom_divider_width,
  0, //x_set_menu_bar_lines,
  0, //x_set_mouse_color,
  0, //x_explicitly_set_name,
  x_set_scroll_bar_width,
  x_set_scroll_bar_height,
  0, //x_set_title,
  x_set_unsplittable,
  x_set_vertical_scroll_bars,
  x_set_horizontal_scroll_bars,
  x_set_visibility,
  0, //x_set_tool_bar_lines,
  0, /* x_set_scroll_bar_foreground, */
  0, /* x_set_scroll_bar_background, */
  x_set_screen_gamma,
  x_set_line_spacing,
  x_set_left_fringe,
  x_set_right_fringe,
  0, /* x_set_wait_for_wm, */
  x_set_fullscreen,
  x_set_font_backend,
  x_set_alpha,
  0, /* x_set_sticky */
  0, /* x_set_tool_bar_position */
  0, /* x_set_inhibit_double_buffering */
  0, //x_set_undecorated,
  0, //x_set_parent_frame,
  0, //x_set_skip_taskbar,
  0, //x_set_no_focus_on_map,
  0, //x_set_no_accept_focus,
  0, //x_set_z_group,
  0, /* x_set_override_redirect */
  x_set_no_special_glyphs,
};

void syms_of_sofns (void) {
  defsubr (&Sx_create_frame);
}

struct so_display_info *x_display_info_for_name (Lisp_Object name) {
  struct so_display_info *dpyinfo;

  CHECK_STRING (name);

  fprintf(stderr, "%d\n", __LINE__);
  for (dpyinfo = &one_so_display_info; dpyinfo; dpyinfo = dpyinfo->next)
    if (!NILP (Fstring_equal (XCAR (dpyinfo->name_list_element), name)))
      return dpyinfo;

  /* Use this general default value to start with.  */
  Vx_resource_name = Vinvocation_name;

  validate_x_resource_name ();

  fprintf(stderr, "%d\n", __LINE__);
  dpyinfo = so_term_init (name, NULL, SSDATA (Vx_resource_name));

  if (dpyinfo == 0)
    error ("Cannot connect to server %s", SDATA (name));

  XSETFASTINT (Vwindow_system_version, 1);

  return dpyinfo;
}

DEFUN ("x-open-connection", Fx_open_connection, Sx_open_connection,
       1, 3, 0, doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display, Lisp_Object xrm_string, Lisp_Object must_succeed)
{
  char *xrm_option;
  struct so_display_info *dpyinfo;

  CHECK_STRING (display);

  /* Signal an error in order to encourage correct use from callers.
   * If we ever support multiple window systems in the same Emacs,
   * we'll need callers to be precise about what window system they
   * want.  */

  fprintf(stderr, "display:%s\n", SSDATA (display));

  if (strcmp (SSDATA (display), "w32") != 0)
    error ("The name of the display in this Emacs must be \"w32\"");

  /* If initialization has already been done, return now to avoid
     overwriting critical parts of one_so_display_info.  */
  if (window_system_available (NULL))
    return Qnil;

  if (! NILP (xrm_string))
    CHECK_STRING (xrm_string);

  /* /\* Allow color mapping to be defined externally; first look in user's */
  /*    HOME directory, then in Emacs etc dir for a file called rgb.txt. *\/ */
  /* { */
  /*   Lisp_Object color_file; */

  /*   color_file = build_string ("~/rgb.txt"); */

  /*   if (NILP (Ffile_readable_p (color_file))) */
  /*     color_file = */
  /*       Fexpand_file_name (build_string ("rgb.txt"), */
  /*       		   Fsymbol_value (intern ("data-directory"))); */

  /*   Vso_color_map = Fx_load_color_file (color_file); */
  /* } */
  /* if (NILP (Vso_color_map)) */
  /*   Vso_color_map = so_default_color_map (); */

  /* /\* Merge in system logical colors.  *\/ */
  /* add_system_logical_colors_to_map (&Vso_color_map); */

  if (! NILP (xrm_string))
    xrm_option = SSDATA (xrm_string);
  else
    xrm_option = NULL;

  /* Use this general default value to start with.  */
  /* First remove .exe suffix from invocation-name - it looks ugly. */
  {
    char basename[ 255 ], *str;

    lispstpcpy (basename, Vinvocation_name);
    str = strrchr (basename, '.');
    if (str) *str = 0;
    Vinvocation_name = build_string (basename);
  }
  Vx_resource_name = Vinvocation_name;

  validate_x_resource_name ();

  fprintf(stderr, "%d\n", __LINE__);

  /* This is what opens the connection and sets x_current_display.
     This also initializes many symbols, such as those used for input.  */
  dpyinfo = so_term_init (display, xrm_option, SSDATA (Vx_resource_name));

  if (dpyinfo == 0)
    {
      if (!NILP (must_succeed))
	fatal ("Cannot connect to server %s.\n",
	       SDATA (display));
      else
	error ("Cannot connect to server %s", SDATA (display));
    }

  XSETFASTINT (Vwindow_system_version, 1);
  return Qnil;
}

DEFUN ("x-close-connection", Fx_close_connection,
       Sx_close_connection, 1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  if (dpyinfo->reference_count > 0)
    error ("Display still has frames on it");

  block_input ();
  x_destroy_all_bitmaps (dpyinfo);

  x_delete_display (dpyinfo);
  unblock_input ();

  return Qnil;
}

DEFUN ("x-display-list", Fx_display_list, Sx_display_list, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  Lisp_Object result = Qnil;
  struct so_display_info *wdi;

  for (wdi = x_display_list; wdi; wdi = wdi->next)
    result = Fcons (XCAR (wdi->name_list_element), result);

  return result;
}

DEFUN ("x-synchronize", Fx_synchronize, Sx_synchronize, 1, 2, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object on, Lisp_Object display)
{
  return Qnil;
}
