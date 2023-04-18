/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_7_guis.c written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 */
/* thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_pd.h"
#include "g_canvas.h"

#include "g_all_guis.h"
#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* --------------- tgl     gui-toggle ------------------------- */

t_widgetbehavior toggle_widgetbehavior;
static t_class *toggle_class;

/* widget helper functions */

void toggle_draw_update(t_toggle *x, t_glist *glist)
{
    if(glist_isvisible(glist))
    {
        t_canvas *canvas = glist_getcanvas(glist);

        sys_vgui(".x%lx.c itemconfigure %lxX1 -fill #%06x\n", canvas, x,
                 (x->x_on != 0.0) ? x->x_gui.x_fcol : x->x_gui.x_bcol);
        sys_vgui(".x%lx.c itemconfigure %lxX2 -fill #%06x\n", canvas, x,
                 (x->x_on != 0.0) ? x->x_gui.x_fcol : x->x_gui.x_bcol);
    }
}

void toggle_draw_new(t_toggle *x, t_glist *glist)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = IEM_GUI_IOHEIGHT * IEMGUI_ZOOM(x);
    int crossw = 1, w = x->x_gui.x_w / IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);
    if(w >= 30)
        crossw = 2;
    if(w >= 60)
        crossw = 3;
    crossw *= IEMGUI_ZOOM(x);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -width %d -fill #%06x -tags %lxBASE\n",
             canvas, xpos, ypos,
             xpos + x->x_gui.x_w, ypos + x->x_gui.x_h,
             IEMGUI_ZOOM(x),
             x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create line %d %d %d %d -width %d -fill #%06x -tags %lxX1\n",
             canvas,
             xpos + crossw + IEMGUI_ZOOM(x), ypos + crossw + IEMGUI_ZOOM(x),
             xpos + x->x_gui.x_w - crossw - IEMGUI_ZOOM(x), ypos + x->x_gui.x_h - crossw - IEMGUI_ZOOM(x),
             crossw, (x->x_on != 0.0) ? x->x_gui.x_fcol : x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create line %d %d %d %d -width %d -fill #%06x -tags %lxX2\n",
             canvas,
             xpos + crossw + IEMGUI_ZOOM(x), ypos + x->x_gui.x_h - crossw - IEMGUI_ZOOM(x),
             xpos + x->x_gui.x_w - crossw - IEMGUI_ZOOM(x), ypos + crossw + IEMGUI_ZOOM(x),
             crossw, (x->x_on != 0.0) ? x->x_gui.x_fcol : x->x_gui.x_bcol, x);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxOUT%d outlet]\n",
             canvas,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h,
             x, 0);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh,
             x, 0);
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font {{%s} -%d %s} -fill #%06x -tags [list %lxLABEL label text]\n",
             canvas, xpos + x->x_gui.x_ldx * IEMGUI_ZOOM(x),
             ypos + x->x_gui.x_ldy * IEMGUI_ZOOM(x),
             (strcmp(x->x_gui.x_lab->s_name, "empty") ? x->x_gui.x_lab->s_name : ""),
             x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x), sys_fontweight,
             x->x_gui.x_lcol, x);
}

void toggle_draw_move(t_toggle *x, t_glist *glist)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = IEM_GUI_IOHEIGHT * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);
    int crossw = 1, w = x->x_gui.x_w / IEMGUI_ZOOM(x);
    if(w >= 30)
        crossw = 2;
    if(w >= 60)
        crossw = 3;
    crossw *= IEMGUI_ZOOM(x);

    sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + x->x_gui.x_w, ypos + x->x_gui.x_h);
    sys_vgui(".x%lx.c itemconfigure %lxX1 -width %d\n", canvas, x, crossw);
    sys_vgui(".x%lx.c coords %lxX1 %d %d %d %d\n",
             canvas, x,
             xpos + crossw + IEMGUI_ZOOM(x), ypos + crossw + IEMGUI_ZOOM(x),
             xpos + x->x_gui.x_w - crossw, ypos + x->x_gui.x_h - crossw);
    sys_vgui(".x%lx.c itemconfigure %lxX2 -width %d\n", canvas, x, crossw);
    sys_vgui(".x%lx.c coords %lxX2 %d %d %d %d\n",
             canvas, x,
             xpos + crossw + IEMGUI_ZOOM(x), ypos + x->x_gui.x_h - crossw - IEMGUI_ZOOM(x),
             xpos + x->x_gui.x_w - crossw, ypos + crossw);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh);
    sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x,
             xpos + x->x_gui.x_ldx * IEMGUI_ZOOM(x),
             ypos + x->x_gui.x_ldy * IEMGUI_ZOOM(x));
}

void toggle_draw_erase(t_toggle* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxX1\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxX2\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void toggle_draw_config(t_toggle* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c itemconfigure %lxLABEL -font {{%s} -%d %s} -fill #%06x -text {%s} \n",
             canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize * IEMGUI_ZOOM(x), sys_fontweight,
             (x->x_gui.x_fsf.x_selected ? IEM_GUI_COLOR_SELECTED : x->x_gui.x_lcol),
             (strcmp(x->x_gui.x_lab->s_name, "empty") ? x->x_gui.x_lab->s_name : ""));
    sys_vgui(".x%lx.c itemconfigure %lxBASE -fill #%06x\n", canvas, x,
             x->x_gui.x_bcol);
    sys_vgui(".x%lx.c itemconfigure %lxX1 -fill #%06x\n", canvas, x,
             x->x_on ? x->x_gui.x_fcol : x->x_gui.x_bcol);
    sys_vgui(".x%lx.c itemconfigure %lxX2 -fill #%06x\n", canvas, x,
             x->x_on ? x->x_gui.x_fcol : x->x_gui.x_bcol);
}

void toggle_draw_io(t_toggle* x, t_glist* glist, int old_snd_rcv_flags)
{
    int xpos = text_xpix(&x->x_gui.x_obj, glist);
    int ypos = text_ypix(&x->x_gui.x_obj, glist);
    int iow = IOWIDTH * IEMGUI_ZOOM(x), ioh = IEM_GUI_IOHEIGHT * IEMGUI_ZOOM(x);
    t_canvas *canvas = glist_getcanvas(glist);

    if((old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && !x->x_gui.x_fsf.x_snd_able) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lxOUT%d\n",
             canvas,
             xpos, ypos + x->x_gui.x_h + IEMGUI_ZOOM(x) - ioh,
             xpos + iow, ypos + x->x_gui.x_h,
             x, 0);
        /* keep above outlet */
        sys_vgui(".x%lx.c raise %lxLABEL %lxOUT%d\n", canvas, x, x, 0);
    }
    if(!(old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    if((old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && !x->x_gui.x_fsf.x_rcv_able) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lxIN%d\n",
             canvas,
             xpos, ypos,
             xpos + iow, ypos - IEMGUI_ZOOM(x) + ioh,
             x, 0);
        /* keep above inlet */
        sys_vgui(".x%lx.c raise %lxLABEL %lxIN%d\n", canvas, x, x, 0);
    }
    if(!(old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void toggle_draw_select(t_toggle* x, t_glist* glist)
{
    t_canvas *canvas = glist_getcanvas(glist);

    if(x->x_gui.x_fsf.x_selected)
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%06x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%06x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
    }
    else
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%06x\n", canvas, x, IEM_GUI_COLOR_NORMAL);
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%06x\n", canvas, x, x->x_gui.x_lcol);
    }
}

void toggle_draw(t_toggle *x, t_glist *glist, int mode)
{
    if(mode == IEM_GUI_DRAW_MODE_UPDATE)
        toggle_draw_update(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_MOVE)
        toggle_draw_move(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_NEW)
        toggle_draw_new(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_SELECT)
        toggle_draw_select(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_ERASE)
        toggle_draw_erase(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_CONFIG)
        toggle_draw_config(x, glist);
    else if(mode >= IEM_GUI_DRAW_MODE_IO)
        toggle_draw_io(x, glist, mode - IEM_GUI_DRAW_MODE_IO);
}

/* ------------------------ tgl widgetbehaviour----------------------------- */

static void toggle_getrect(t_gobj *z, t_glist *glist,
                           int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_toggle *x = (t_toggle *)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void toggle_save(t_gobj *z, t_binbuf *b)
{
    t_toggle *x = (t_toggle *)z;
    t_symbol *bflcol[3];
    t_symbol *srl[3];

    iemgui_save(&x->x_gui, srl, bflcol);
    binbuf_addv(b, "ssiisiisssiiiisssff", gensym("#X"), gensym("obj"),
                (int)x->x_gui.x_obj.te_xpix,
                (int)x->x_gui.x_obj.te_ypix,
                gensym("tgl"), x->x_gui.x_w/IEMGUI_ZOOM(x),
                iem_symargstoint(&x->x_gui.x_isa),
                srl[0], srl[1], srl[2],
                x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[1], bflcol[2], x->x_on, x->x_nonzero);
    binbuf_addv(b, ";");
}

static void toggle_properties(t_gobj *z, t_glist *owner)
{
    t_toggle *x = (t_toggle *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_properties(&x->x_gui, srl);
    sprintf(buf, "pdtk_iemgui_dialog %%s |tgl| \
            ----------dimensions(pix):----------- %d %d size: 0 0 empty \
            -----------non-zero-value:----------- %g value: 0.0 empty %g \
            -1 lin log %d %d empty %d \
            %s %s \
            %s %d %d \
            %d %d \
            #%06x #%06x #%06x\n",
            x->x_gui.x_w/IEMGUI_ZOOM(x), IEM_GUI_MINSIZE,
            x->x_nonzero, 1.0,/*non_zero-schedule*/
            x->x_gui.x_isa.x_loadinit, -1, -1,/*no multi*/
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fsf.x_font_style, x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol, 0xffffff & x->x_gui.x_lcol);
    gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}

static void toggle_bang(t_toggle *x)
{
    x->x_on = (x->x_on == 0.0) ? x->x_nonzero : 0.0;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    outlet_float(x->x_gui.x_obj.ob_outlet, x->x_on);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, x->x_on);
}

static void toggle_dialog(t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *srl[3];
    int a = (int)atom_getfloatarg(0, argc, argv);
    t_float nonzero = (t_float)atom_getfloatarg(2, argc, argv);
    int sr_flags;

    if(nonzero == 0.0)
        nonzero = 1.0;
    x->x_nonzero = nonzero;
    if(x->x_on != 0.0)
        x->x_on = x->x_nonzero;
    sr_flags = iemgui_dialog(&x->x_gui, srl, argc, argv);
    x->x_gui.x_w = iemgui_clip_size(a) * IEMGUI_ZOOM(x);
    x->x_gui.x_h = x->x_gui.x_w;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_CONFIG);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_IO + sr_flags);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
    canvas_fixlinesfor(x->x_gui.x_glist, (t_text*)x);
}

static void toggle_click(t_toggle *x, t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{toggle_bang(x);}

static int toggle_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        toggle_click((t_toggle *)z, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift, 0, (t_floatarg)alt);
    return (1);
}

static void toggle_set(t_toggle *x, t_floatarg f)
{
    int old = (x->x_on != 0);
    x->x_on = f;
    if (f != 0.0 && pd_compatibilitylevel < 46)
        x->x_nonzero = f;
    if ((x->x_on != 0) != old)
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void toggle_float(t_toggle *x, t_floatarg f)
{
    toggle_set(x, f);
    if(x->x_gui.x_fsf.x_put_in2out)
    {
        outlet_float(x->x_gui.x_obj.ob_outlet, x->x_on);
        if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
            pd_float(x->x_gui.x_snd->s_thing, x->x_on);
    }
}

static void toggle_fout(t_toggle *x, t_floatarg f)
{
    toggle_set(x, f);
    outlet_float(x->x_gui.x_obj.ob_outlet, x->x_on);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, x->x_on);
}

static void toggle_loadbang(t_toggle *x, t_floatarg action)
{
    if (action == LB_LOAD && x->x_gui.x_isa.x_loadinit)
        toggle_fout(x, (t_float)x->x_on);
}

static void toggle_size(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_gui.x_w = iemgui_clip_size((int)atom_getfloatarg(0, ac, av)) * IEMGUI_ZOOM(x);
    x->x_gui.x_h = x->x_gui.x_w;
    iemgui_size((void *)x, &x->x_gui);
}

static void toggle_delta(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void toggle_pos(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void toggle_color(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_color((void *)x, &x->x_gui, s, ac, av);}

static void toggle_send(t_toggle *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void toggle_receive(t_toggle *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}

static void toggle_label(t_toggle *x, t_symbol *s)
{iemgui_label((void *)x, &x->x_gui, s);}

static void toggle_label_font(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_label_font((void *)x, &x->x_gui, s, ac, av);}

static void toggle_label_pos(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void toggle_init(t_toggle *x, t_floatarg f)
{
    x->x_gui.x_isa.x_loadinit = (f == 0.0) ? 0 : 1;
}

static void toggle_nonzero(t_toggle *x, t_floatarg f)
{
    if(f != 0.0)
        x->x_nonzero = f;
}

static void *toggle_new(t_symbol *s, int argc, t_atom *argv)
{
    t_toggle *x = (t_toggle *)pd_new(toggle_class);
    int a = IEM_GUI_DEFAULTSIZE, f = 0;
    int ldx = 17, ldy = 7;
    int fs = 10;
    t_float on = 0.0, nonzero = 1.0;
    char str[144];

    iem_inttosymargs(&x->x_gui.x_isa, 0);
    iem_inttofstyle(&x->x_gui.x_fsf, 0);

    x->x_gui.x_bcol = 0xFCFCFC;
    x->x_gui.x_fcol = 0x00;
    x->x_gui.x_lcol = 0x00;

    if(((argc == 13)||(argc == 14))&&IS_A_FLOAT(argv,0)
       &&IS_A_FLOAT(argv,1)
       &&(IS_A_SYMBOL(argv,2)||IS_A_FLOAT(argv,2))
       &&(IS_A_SYMBOL(argv,3)||IS_A_FLOAT(argv,3))
       &&(IS_A_SYMBOL(argv,4)||IS_A_FLOAT(argv,4))
       &&IS_A_FLOAT(argv,5)&&IS_A_FLOAT(argv,6)
       &&IS_A_FLOAT(argv,7)&&IS_A_FLOAT(argv,8)&&IS_A_FLOAT(argv,12))
    {
        a = (int)atom_getfloatarg(0, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, atom_getfloatarg(1, argc, argv));
        iemgui_new_getnames(&x->x_gui, 2, argv);
        ldx = (int)atom_getfloatarg(5, argc, argv);
        ldy = (int)atom_getfloatarg(6, argc, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, atom_getfloatarg(7, argc, argv));
        fs = (int)atom_getfloatarg(8, argc, argv);
        iemgui_all_loadcolors(&x->x_gui, argv+9, argv+10, argv+11);
        on = (t_float)atom_getfloatarg(12, argc, argv);
    }
    else iemgui_new_getnames(&x->x_gui, 2, 0);
    if((argc == 14)&&IS_A_FLOAT(argv,13))
        nonzero = (t_float)atom_getfloatarg(13, argc, argv);
    x->x_gui.x_draw = (t_iemfunptr)toggle_draw;

    x->x_gui.x_fsf.x_snd_able = 1;
    x->x_gui.x_fsf.x_rcv_able = 1;
    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.x_snd->s_name, "empty"))
        x->x_gui.x_fsf.x_snd_able = 0;
    if (!strcmp(x->x_gui.x_rcv->s_name, "empty"))
        x->x_gui.x_fsf.x_rcv_able = 0;
    if(x->x_gui.x_fsf.x_font_style == 1) strcpy(x->x_gui.x_font, "helvetica");
    else if(x->x_gui.x_fsf.x_font_style == 2) strcpy(x->x_gui.x_font, "times");
    else { x->x_gui.x_fsf.x_font_style = 0;
        strcpy(x->x_gui.x_font, sys_font); }
    x->x_nonzero = (nonzero != 0.0) ? nonzero : 1.0;
    if(x->x_gui.x_isa.x_loadinit)
        x->x_on = (on != 0.0) ? nonzero : 0.0;
    else
        x->x_on = 0.0;
    if (x->x_gui.x_fsf.x_rcv_able)
        pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.x_fontsize = fs;
    x->x_gui.x_w = iemgui_clip_size(a);
    x->x_gui.x_h = x->x_gui.x_w;
    iemgui_verify_snd_ne_rcv(&x->x_gui);
    iemgui_newzoom(&x->x_gui);
    outlet_new(&x->x_gui.x_obj, &s_float);
    return (x);
}

static void toggle_ff(t_toggle *x)
{
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    gfxstub_deleteforkey(x);
}

void g_toggle_setup(void)
{
    toggle_class = class_new(gensym("tgl"), (t_newmethod)toggle_new,
                             (t_method)toggle_ff, sizeof(t_toggle), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)toggle_new, gensym("toggle"), A_GIMME, 0);
    class_addbang(toggle_class, toggle_bang);
    class_addfloat(toggle_class, toggle_float);
    class_addmethod(toggle_class, (t_method)toggle_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_dialog, gensym("dialog"),
                    A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_loadbang,
        gensym("loadbang"), A_DEFFLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_set, gensym("set"), A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_size, gensym("size"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_delta, gensym("delta"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_pos, gensym("pos"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_color, gensym("color"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_send, gensym("send"), A_DEFSYM, 0);
    class_addmethod(toggle_class, (t_method)toggle_receive, gensym("receive"), A_DEFSYM, 0);
    class_addmethod(toggle_class, (t_method)toggle_label, gensym("label"), A_DEFSYM, 0);
    class_addmethod(toggle_class, (t_method)toggle_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_label_font, gensym("label_font"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_init, gensym("init"), A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_nonzero, gensym("nonzero"), A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)iemgui_zoom, gensym("zoom"),
        A_CANT, 0);
    toggle_widgetbehavior.w_getrectfn = toggle_getrect;
    toggle_widgetbehavior.w_displacefn = iemgui_displace;
    toggle_widgetbehavior.w_selectfn = iemgui_select;
    toggle_widgetbehavior.w_activatefn = NULL;
    toggle_widgetbehavior.w_deletefn = iemgui_delete;
    toggle_widgetbehavior.w_visfn = iemgui_vis;
    toggle_widgetbehavior.w_clickfn = toggle_newclick;
    class_setwidget(toggle_class, &toggle_widgetbehavior);
    class_sethelpsymbol(toggle_class, gensym("toggle"));
    class_setsavefn(toggle_class, toggle_save);
    class_setpropertiesfn(toggle_class, toggle_properties);
}
