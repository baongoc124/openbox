#include "frame.h"
#include "openbox.h"
#include "screen.h"
#include "framerender.h"
#include "render/theme.h"

static void framerender_label(Frame *self, Appearance *a);
static void framerender_icon(Frame *self, Appearance *a);
static void framerender_max(Frame *self, Appearance *a);
static void framerender_iconify(Frame *self, Appearance *a);
static void framerender_desk(Frame *self, Appearance *a);
static void framerender_shade(Frame *self, Appearance *a);
static void framerender_close(Frame *self, Appearance *a);

void framerender_frame(Frame *self)
{
    if (client_focused(self->client)) {
        XSetWindowBorder(ob_display, self->plate,
                         theme_cb_focused_color->pixel);
    } else {
        XSetWindowBorder(ob_display, self->plate,
                         theme_cb_unfocused_color->pixel);
    }

    if (self->client->decorations & Decor_Titlebar) {
        Appearance *t, *l, *m, *n, *i, *d, *s, *c;

        t = (client_focused(self->client) ?
             self->a_focused_title : self->a_unfocused_title);
        l = (client_focused(self->client) ?
             self->a_focused_label : self->a_unfocused_label);
        m = (client_focused(self->client) ?
             (self->client->max_vert || self->client->max_horz ?
              theme_a_focused_pressed_set_max :
              (self->max_press ?
               theme_a_focused_pressed_max : theme_a_focused_unpressed_max)) :
             (self->client->max_vert || self->client->max_horz ?
              theme_a_unfocused_pressed_set_max :
              (self->max_press ?
               theme_a_unfocused_pressed_max :
               theme_a_unfocused_unpressed_max)));
        n = self->a_icon;
        i = (client_focused(self->client) ?
             (self->iconify_press ?
              theme_a_focused_pressed_iconify :
              theme_a_focused_unpressed_iconify) :
             (self->iconify_press ?
              theme_a_unfocused_pressed_iconify :
              theme_a_unfocused_unpressed_iconify));
        d = (client_focused(self->client) ?
             (self->client->desktop == DESKTOP_ALL ?
              theme_a_focused_pressed_set_desk :
              (self->desk_press ?
               theme_a_focused_pressed_desk :
               theme_a_focused_unpressed_desk)) :
             (self->client->desktop == DESKTOP_ALL ?
              theme_a_unfocused_pressed_set_desk :
              (self->desk_press ?
               theme_a_unfocused_pressed_desk :
               theme_a_unfocused_unpressed_desk)));
        s = (client_focused(self->client) ?
             (self->client->shaded ?
              theme_a_focused_pressed_set_shade :
              (self->shade_press ?
               theme_a_focused_pressed_shade :
               theme_a_focused_unpressed_shade)) :
             (self->client->shaded ?
              theme_a_unfocused_pressed_set_shade :
              (self->shade_press ?
               theme_a_unfocused_pressed_shade :
               theme_a_unfocused_unpressed_shade)));
        c = (client_focused(self->client) ?
             (self->close_press ?
              theme_a_focused_pressed_close :
              theme_a_focused_unpressed_close) :
             (self->close_press ?
              theme_a_unfocused_pressed_close :
              theme_a_unfocused_unpressed_close));

        paint(self->title, t);

        /* set parents for any parent relative guys */
        l->surface.data.planar.parent = t;
        l->surface.data.planar.parentx = self->label_x;
        l->surface.data.planar.parenty = theme_bevel;

        m->surface.data.planar.parent = t;
        m->surface.data.planar.parentx = self->max_x;
        m->surface.data.planar.parenty = theme_bevel + 1;

        n->surface.data.planar.parent = t;
        n->surface.data.planar.parentx = self->icon_x;
        n->surface.data.planar.parenty = theme_bevel + 1;

        i->surface.data.planar.parent = t;
        i->surface.data.planar.parentx = self->iconify_x;
        i->surface.data.planar.parenty = theme_bevel + 1;

        d->surface.data.planar.parent = t;
        d->surface.data.planar.parentx = self->desk_x;
        d->surface.data.planar.parenty = theme_bevel + 1;

        s->surface.data.planar.parent = t;
        s->surface.data.planar.parentx = self->shade_x;
        s->surface.data.planar.parenty = theme_bevel + 1;

        c->surface.data.planar.parent = t;
        c->surface.data.planar.parentx = self->close_x;
        c->surface.data.planar.parenty = theme_bevel + 1;

        framerender_label(self, l);
        framerender_max(self, m);
        framerender_icon(self, n);
        framerender_iconify(self, i);
        framerender_desk(self, d);
        framerender_shade(self, s);
        framerender_close(self, c);
    }

    if (self->client->decorations & Decor_Handle) {
        Appearance *h, *g;

        h = (client_focused(self->client) ?
             self->a_focused_handle : self->a_unfocused_handle);
        g = (client_focused(self->client) ?
             theme_a_focused_grip : theme_a_unfocused_grip);

        if (g->surface.data.planar.grad == Background_ParentRelative) {
            g->surface.data.planar.parent = h;
            paint(self->handle, h);
        } else
            paint(self->handle, h);

        g->surface.data.planar.parentx = 0;
        g->surface.data.planar.parenty = 0;

        paint(self->lgrip, g);

        g->surface.data.planar.parentx = self->width - theme_grip_width;
        g->surface.data.planar.parenty = 0;

        paint(self->rgrip, g);
    }
}

static void framerender_label(Frame *self, Appearance *a)
{
    if (self->label_x < 0) return;


    /* set the texture's text! */
    a->texture[0].data.text.string = self->client->title;
    RECT_SET(a->texture[0].position, 0, 0,
             self->label_width, theme_label_height);

    paint(self->label, a);
}

static void framerender_icon(Frame *self, Appearance *a)
{
    if (self->icon_x < 0) return;

    if (self->client->nicons) {
        Icon *icon = client_icon(self->client,
                                 theme_button_size, theme_button_size);
        a->texture[0].type = RGBA;
        a->texture[0].data.rgba.width = icon->width;
        a->texture[0].data.rgba.height = icon->height;
        a->texture[0].data.rgba.data = icon->data;
        RECT_SET(self->a_icon->texture[0].position, 0, 0,
                 theme_button_size,theme_button_size);
    } else
        a->texture[0].type = NoTexture;

    paint(self->icon, a);
}

static void framerender_max(Frame *self, Appearance *a)
{
    if (self->max_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0,
             theme_button_size, theme_button_size);
    paint(self->max, a);
}

static void framerender_iconify(Frame *self, Appearance *a)
{
    if (self->iconify_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0,
             theme_button_size, theme_button_size);
    paint(self->iconify, a);
}

static void framerender_desk(Frame *self, Appearance *a)
{
    if (self->desk_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0,
             theme_button_size, theme_button_size);
    paint(self->desk, a);
}

static void framerender_shade(Frame *self, Appearance *a)
{
    if (self->shade_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0,
             theme_button_size, theme_button_size);
    paint(self->shade, a);
}

static void framerender_close(Frame *self, Appearance *a)
{
    if (self->close_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0,
             theme_button_size, theme_button_size);
    paint(self->close, a);
}

void framerender_popup_label(Window win, Size *sz, char *text)
{
    Appearance *a;

    a = theme_app_hilite_label;
    a->texture[0].data.text.string = text;
    RECT_SET(a->area, 0, 0, sz->width, sz->height);
    a->texture[0].position = a->area;

    XSetWindowBorderWidth(ob_display, win, theme_bwidth);
    XSetWindowBorder(ob_display, win, theme_b_color->pixel);

    paint(win, a);
}

void framerender_size_popup_label(char *text, Size *sz)
{
    Appearance *a;

    a = theme_app_hilite_label;
    a->texture[0].data.text.string = text;

    appearance_minsize(a, &sz->width, &sz->height);
    sz->width += theme_bevel * 2;
    sz->height += theme_bevel * 2;
}
