/*
 * Copyright 2014 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "efl_assist.h"
#include "efl_assist_private.h"

#include <Ecore_X.h>
#define BASE_WIDTH      (720 * elm_config_scale_get())
#define BASE_HEIGHT     (1280 * elm_config_scale_get())
#define PADDING_FOR_COPY_PASTE_AREA_HEIGHT (150 * elm_config_scale_get())
#define PADDING_FOR_COPY_PASTE_AREA_WIDTH (300 * elm_config_scale_get())

static const char ELM_WIDGET_SMART_NAME[] = "elm_widget";
static const char WIN_SMART_NAME[] = "elm_win";
static const char EA_MENU_DATA[] = "_ea_menu_data";

typedef struct _Center_Popup_Data Center_Popup_Data;
struct _Center_Popup_Data
{
   Evas_Object *parent;
   Evas_Object *parent_win;
   Evas_Object *dim_layout;
   Evas_Object *event_rect;
   Evas_Object *popup_win;
   Evas_Object *popup;
   Ecore_Job   *job, *rot_job, *entry_job;
   Ecore_Idler *idler, *entry_idler;
   Evas_Coord   mouse_pos_x, mouse_pos_y;
   Eina_Bool    mouse_on_hold;
   Eina_Bool    is_entry;
};

typedef struct _Menu_Popup_Data Menu_Popup_Data;
struct _Menu_Popup_Data
{
   Evas_Object *parent;
   Evas_Object *popup_win;
   Evas_Object *events_blocker;
   Evas_Object *ctxpopup;
};

static void _ctxpopup_detach(Evas_Object *obj, void *data);

static void _move_ctxpopup(Evas_Object *ctx)
{
   Evas_Object *win;
   Evas_Coord w, h;
   int pos = -1;

   win = elm_ctxpopup_hover_parent_get(ctx);

   elm_win_screen_size_get(win, NULL, NULL, &w, &h);
   pos = elm_win_rotation_get(win);

   switch (pos)
     {
      case 0:
      case 180:
         evas_object_move(ctx, 0, h);
         break;
      case 90:
         evas_object_move(ctx, 0, w);
         break;
      case 270:
         evas_object_move(ctx, h, w);
         break;
     }
}

static void _resize_events_blocker(Menu_Popup_Data *mp_data)
{
   Evas_Coord px, py, pw, ph;

   if (!mp_data) return;

   evas_object_geometry_get(mp_data->parent, &px, &py, &pw, &ph);
   evas_object_move(mp_data->events_blocker, px, py);
   evas_object_resize(mp_data->events_blocker, pw, ph);
}

static void _win_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   _resize_events_blocker(mp_data);
   _move_ctxpopup(mp_data->ctxpopup);
}

static void _win_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   _resize_events_blocker(mp_data);
   _move_ctxpopup(mp_data->ctxpopup);
}

static void _ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
   _ctxpopup_detach(obj, data);
}

static void _ctxpopup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   _ctxpopup_detach(obj, data);
}

static void _ctxpopup_show_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   evas_object_show(mp_data->events_blocker);
   evas_object_show(mp_data->popup_win);
}

static void _ctxpopup_hide_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   evas_object_hide(mp_data->events_blocker);
   evas_object_hide(mp_data->popup_win);
}

static void _ctxpopup_parent_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   evas_object_del(mp_data->events_blocker);
   evas_object_del(mp_data->popup_win);
}

static void _parent_win_focus_out_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_ctxpopup_dismiss(data);
}

static void _ctxpopup_detach(Evas_Object *obj, void *data)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   evas_object_data_set(obj, EA_MENU_DATA, NULL);

   evas_object_event_callback_del_full(mp_data->popup_win, EVAS_CALLBACK_RESIZE, _win_resize_cb, mp_data);
   evas_object_smart_callback_del_full(mp_data->popup_win, "wm,rotation,changed", _win_rotate_cb, mp_data);
   evas_object_smart_callback_del_full(obj, "dismissed", _ctxpopup_dismissed_cb, mp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL, _ctxpopup_del_cb, mp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_SHOW, _ctxpopup_show_cb, mp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_HIDE, _ctxpopup_hide_cb, mp_data);
   evas_object_event_callback_del_full(mp_data->parent, EVAS_CALLBACK_DEL, _ctxpopup_parent_del_cb, mp_data);
   evas_object_smart_callback_del_full(mp_data->popup_win, "focus,out", _parent_win_focus_out_cb, mp_data->ctxpopup);

   evas_object_del(mp_data->events_blocker);
   evas_object_del(mp_data->popup_win);
   free(mp_data);
}

static void _center_popup_parent_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Object *object = data ;
   if (object) evas_object_del(object);
}

static void
_block_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Center_Popup_Data *cp_data = data;
   if (!cp_data) return;

   if (!cp_data->mouse_on_hold)
     evas_object_smart_callback_call(cp_data->popup, "block,clicked", NULL);
}

static void _popup_win_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;
   if (!cp_data->is_entry)
     elm_win_resize_object_del(obj, cp_data->popup);
}

static void _entry_job_cb(void *data)
{
   Center_Popup_Data *cp_data = data;
   Evas_Coord ow, oh;
   int pos = -1;
   cp_data->entry_job = NULL;
   pos = elm_win_rotation_get(cp_data->parent_win);

   evas_smart_objects_calculate(evas_object_evas_get(cp_data->popup_win));
   evas_object_geometry_get(cp_data->popup, NULL, NULL, &ow, &oh);
   if (pos == 0 || pos == 180)
     evas_object_resize(cp_data->popup_win, BASE_WIDTH, oh + PADDING_FOR_COPY_PASTE_AREA_HEIGHT);
   else
     evas_object_resize(cp_data->popup_win, ow + PADDING_FOR_COPY_PASTE_AREA_WIDTH, oh + PADDING_FOR_COPY_PASTE_AREA_HEIGHT);
}

static void _ea_dummy_cb(void *data, Evas_Object *obj, void *event_info)
{

}


static void _center_popup_dim_layout_resize(void *data)
{
   Center_Popup_Data *cp_data = data;
   Evas_Coord ox, oy, ow, oh;

   if (!cp_data->dim_layout) return;

   evas_object_geometry_get(cp_data->parent_win, &ox, &oy, &ow, &oh);

   evas_object_move(cp_data->dim_layout, ox, oy);
   evas_object_resize(cp_data->dim_layout, ow, oh);

   if (!cp_data->event_rect)
     {
        cp_data->event_rect = evas_object_rectangle_add(evas_object_evas_get(cp_data->parent_win));
        evas_object_color_set(cp_data->event_rect, 0, 0, 0, 0);
        evas_object_move(cp_data->event_rect, ox, oy);
        evas_object_resize(cp_data->event_rect, ow, oh);
        evas_object_repeat_events_set(cp_data->event_rect, EINA_FALSE);
        ea_object_event_callback_add(cp_data->event_rect, EA_CALLBACK_BACK, _ea_dummy_cb, NULL);
        ea_object_event_callback_add(cp_data->event_rect, EA_CALLBACK_MORE, _ea_dummy_cb, NULL);
     }
}

static void _center_popup_rotate_job_cb(void *data)
{
   Center_Popup_Data *cp_data = data;
   cp_data->rot_job = NULL;

   _center_popup_dim_layout_resize(cp_data);
}

static void _center_popup_parent_win_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;

   if (cp_data->rot_job)
     {
        ecore_job_del(cp_data->rot_job);
        cp_data->rot_job = NULL;
     }
   cp_data->rot_job = ecore_job_add(_center_popup_rotate_job_cb, cp_data);

   if (cp_data->is_entry)
     {
        if (cp_data->entry_job)
          {
             ecore_job_del(cp_data->entry_job);
             cp_data->entry_job = NULL;
          }
        cp_data->entry_job = ecore_job_add(_entry_job_cb, cp_data);
     }
}

static void _center_popup_resize_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;

   Evas_Coord ox, oy, ow, oh;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   if (!cp_data->is_entry)
     {
        evas_object_resize(cp_data->popup_win, ow, oh);
        evas_output_viewport_set(evas_object_evas_get(obj), 0, 0, ow, oh);
        evas_output_size_set(evas_object_evas_get(obj), ow, oh);
     }
}

static Eina_Bool _entry_idler_cb(void *data)
{
   Center_Popup_Data *cp_data = data;
   cp_data->entry_idler = NULL;
   evas_object_show(cp_data->popup_win);
   return ECORE_CALLBACK_CANCEL;
}

static void _center_popup_job_cb(void *data)
{
   Center_Popup_Data *cp_data = data;
   cp_data->job = NULL;
   if (cp_data->is_entry)
     {
        evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_RESIZE, _center_popup_resize_cb, cp_data);
        if (cp_data->entry_job)
          {
             ecore_job_del(cp_data->entry_job);
             cp_data->entry_job = NULL;
          }
        cp_data->entry_job = ecore_job_add(_entry_job_cb, cp_data);

        if (cp_data->entry_idler)
          {
             ecore_idler_del(cp_data->entry_idler);
             cp_data->entry_idler = NULL;
          }
        cp_data->entry_idler = ecore_idler_add(_entry_idler_cb, cp_data);
     }
   else
     {
        evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_RESIZE, _center_popup_resize_cb, cp_data);
        evas_object_show(cp_data->popup_win);
     }
}

static Eina_Bool
_center_popup_idler_cb(void *data)
{
   Center_Popup_Data *cp_data = data;
   cp_data->idler = NULL;

   evas_object_show(cp_data->dim_layout);
   return ECORE_CALLBACK_CANCEL;
}

static void _center_popup_show_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;

   if (!cp_data->is_entry)
     elm_win_resize_object_add(cp_data->popup_win, cp_data->popup);
   if (cp_data->dim_layout)
     {
        _center_popup_dim_layout_resize(cp_data);
        if (cp_data->idler)
          {
             ecore_idler_del(cp_data->idler);
             cp_data->idler = NULL;
          }
        cp_data->idler = ecore_idler_add(_center_popup_idler_cb, cp_data);
     }
   if (cp_data->event_rect)
     evas_object_show(cp_data->event_rect);

   if (cp_data->job)
     {
        ecore_job_del(cp_data->job);
        cp_data->job = NULL;
     }
   cp_data->job = ecore_job_add(_center_popup_job_cb, cp_data);
}

static void _center_popup_parent_resize_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
    Center_Popup_Data *cp_data = data;
    if (cp_data->dim_layout)
      _center_popup_dim_layout_resize(cp_data);
}

static void _center_popup_hide_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;

   if (cp_data->dim_layout)
     evas_object_hide(cp_data->dim_layout);
   if (cp_data->event_rect)
      evas_object_hide(cp_data->event_rect);
   evas_object_hide(cp_data->popup_win);
}

static void
_dim_layout_mouse_move_cb(void *data,
                            Evas *e,
                            Evas_Object *obj,
                            void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Center_Popup_Data *cp_data = data;
   Evas_Coord half_finger_size;
   Evas_Coord move_x, move_y;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!cp_data) return;

   half_finger_size = elm_config_finger_size_get() / 2;
   move_x = abs(cp_data->mouse_pos_x - ev->cur.canvas.x);
   move_y = abs(cp_data->mouse_pos_y - ev->cur.canvas.y);

   if ((move_x > half_finger_size) || (move_y > half_finger_size))
     cp_data->mouse_on_hold = EINA_TRUE;
}

static void
_dim_layout_mouse_down_cb(void *data,
                            Evas *e,
                            Evas_Object *obj,
                            void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Center_Popup_Data *cp_data = data;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!cp_data) return;

   cp_data->mouse_pos_x = ev->canvas.x;
   cp_data->mouse_pos_y = ev->canvas.y;
   cp_data->mouse_on_hold = EINA_FALSE;
}

static void _center_popup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;
   if (!cp_data) return;

   if (cp_data->dim_layout)
     {
        elm_layout_signal_callback_del(cp_data->dim_layout, "elm,action,click", "elm", _block_area_clicked_cb);

        evas_object_event_callback_del(cp_data->dim_layout, EVAS_CALLBACK_MOUSE_MOVE,
                                       _dim_layout_mouse_move_cb);
        evas_object_event_callback_del(cp_data->dim_layout, EVAS_CALLBACK_MOUSE_DOWN,
                                       _dim_layout_mouse_down_cb);
        ea_object_event_callback_del(cp_data->event_rect, EA_CALLBACK_BACK, _ea_dummy_cb);
        ea_object_event_callback_del(cp_data->event_rect, EA_CALLBACK_MORE, _ea_dummy_cb);
        evas_object_del(cp_data->dim_layout);
        cp_data->dim_layout = NULL;
     }
   if (cp_data->event_rect)
     {
     evas_object_del(cp_data->event_rect);
        cp_data->event_rect = NULL;
     }

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL, _center_popup_del_cb, cp_data);
   evas_object_event_callback_del_full(cp_data->parent_win, EVAS_CALLBACK_DEL, _center_popup_parent_del_cb, obj);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_SHOW, _center_popup_show_cb, cp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_HIDE, _center_popup_hide_cb, cp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE, _center_popup_resize_cb, cp_data);

   evas_object_smart_callback_del_full(cp_data->parent_win, "wm,rotation,changed", _center_popup_parent_win_rotate_cb, cp_data);
   evas_object_event_callback_del_full(cp_data->parent_win, EVAS_CALLBACK_RESIZE, _center_popup_parent_resize_cb, cp_data);

   if (cp_data->job)
     {
        ecore_job_del(cp_data->job);
        cp_data->job = NULL;
     }
   if (cp_data->rot_job)
     {
        ecore_job_del(cp_data->rot_job);
        cp_data->rot_job = NULL;
     }
   if (cp_data->idler)
     {
        ecore_idler_del(cp_data->idler);
        cp_data->idler = NULL;
     }
   if (cp_data->is_entry)
     {
        if (cp_data->entry_idler)
          {
             ecore_idler_del(cp_data->entry_idler);
             cp_data->entry_idler = NULL;
          }
        if (cp_data->entry_job)
          {
             ecore_job_del(cp_data->entry_job);
             cp_data->entry_job = NULL;
          }
     }
   evas_object_del(cp_data->popup_win);
   free(cp_data);
}

static void
_center_popup_entry_cb(void *data, Evas_Object *obj, const char *emission, const char *source )
{
   Center_Popup_Data *cp_data = data;
   if (!cp_data) return;
   cp_data->is_entry = EINA_TRUE;
}

EAPI Evas_Object *
ea_center_popup_win_get(Evas_Object *popup)
{
   return evas_object_top_get(evas_object_evas_get(popup));
}

EAPI Evas_Object *
ea_center_popup_add(Evas_Object *parent)
{
   Center_Popup_Data *cp_data;

   cp_data = calloc(1, sizeof(Center_Popup_Data));
   cp_data->parent_win = elm_object_top_widget_get(parent);

   // Create popup window
   cp_data->popup_win = elm_win_add(cp_data->parent_win, "Center Popup", ELM_WIN_DIALOG_BASIC);
   if (!cp_data->popup_win) goto error;
   elm_win_alpha_set(cp_data->popup_win, EINA_TRUE);
   elm_win_title_set(cp_data->popup_win, "Center Popup");
   elm_win_modal_set(cp_data->popup_win, EINA_TRUE);
   if (elm_win_wm_rotation_supported_get(cp_data->popup_win))
     {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(cp_data->popup_win, (const int*)(&rots), 4);
     }

   cp_data->popup = elm_popup_add(cp_data->popup_win);
   if (!cp_data->popup) goto error;

   // for object color class setting ///
   Evas_Object *edje;

   edje = evas_object_data_get(parent, "color_overlay");
   if (edje) evas_object_data_set(cp_data->popup, "color_overlay", edje);

   edje = evas_object_data_get(parent, "font_overlay");
   if (edje) evas_object_data_set(cp_data->popup, "font_overlay", edje);
   /////////////////////////////////////

   elm_object_style_set(cp_data->popup, "center_popup");

   elm_popup_allow_events_set(cp_data->popup, EINA_TRUE);
   evas_object_size_hint_weight_set(cp_data->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   // Create Dimming object
   cp_data->dim_layout = elm_layout_add(cp_data->parent_win);
   if (!cp_data->dim_layout) goto error;
   evas_object_layer_set(cp_data->dim_layout, 50);
   elm_layout_theme_set(cp_data->dim_layout, "notify", "block_events", "center_popup");
   evas_object_size_hint_weight_set(cp_data->dim_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cp_data->dim_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_repeat_events_set(cp_data->dim_layout, EINA_FALSE);

   elm_layout_signal_callback_add(cp_data->dim_layout, "elm,action,click", "elm", _block_area_clicked_cb, cp_data);
   evas_object_event_callback_add(cp_data->dim_layout, EVAS_CALLBACK_MOUSE_MOVE,
                                  _dim_layout_mouse_move_cb, cp_data);
   evas_object_event_callback_add(cp_data->dim_layout, EVAS_CALLBACK_MOUSE_DOWN,
                                  _dim_layout_mouse_down_cb, cp_data);

   evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_SHOW, _center_popup_show_cb, cp_data);
   evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_HIDE, _center_popup_hide_cb, cp_data);
   evas_object_event_callback_priority_add(cp_data->popup, EVAS_CALLBACK_DEL, EVAS_CALLBACK_PRIORITY_BEFORE, _center_popup_del_cb, cp_data);
   evas_object_event_callback_add(cp_data->parent_win, EVAS_CALLBACK_DEL, _center_popup_parent_del_cb, cp_data->popup);
   evas_object_smart_callback_add(cp_data->parent_win, "wm,rotation,changed", _center_popup_parent_win_rotate_cb, cp_data);
   evas_object_event_callback_add(cp_data->popup_win, EVAS_CALLBACK_DEL, _popup_win_del_cb, cp_data);
   elm_object_signal_callback_add(cp_data->popup, "elm,action,center_popup,entry", "", _center_popup_entry_cb, cp_data);
   evas_object_event_callback_add(cp_data->parent_win, EVAS_CALLBACK_RESIZE, _center_popup_parent_resize_cb, cp_data);
   cp_data->is_entry = EINA_FALSE;
   return cp_data->popup;

error:
   if (cp_data->popup_win) evas_object_del(cp_data->popup_win);
   free(cp_data);
   return NULL;
}

EAPI Evas_Object *
ea_menu_popup_win_get(Evas_Object *obj)
{
   if (!obj) return NULL;
   return evas_object_top_get(evas_object_evas_get(obj));
}

EAPI Evas_Object *
ea_menu_popup_add(Evas_Object *parent)
{
   Evas_Object *parent_win;
   Evas_Coord px, py, pw, ph;
   Menu_Popup_Data *mp_data;

   mp_data = calloc(1, sizeof(Menu_Popup_Data));
   mp_data->parent = parent;

   parent_win = evas_object_top_get(evas_object_evas_get(parent));

   // Create Menu popup window
   mp_data->popup_win = elm_win_add(parent_win, "Menu Popup", ELM_WIN_POPUP_MENU);
   if (!mp_data->popup_win) goto error;
   ecore_x_icccm_transient_for_set(elm_win_xwindow_get(mp_data->popup_win), elm_win_xwindow_get(parent_win));
   elm_win_alpha_set(mp_data->popup_win, EINA_TRUE);
   elm_win_role_set(mp_data->popup_win, "no-effect");
   elm_win_title_set(mp_data->popup_win, "Menu Popup");

   if (elm_win_wm_rotation_supported_get(mp_data->popup_win))
     {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(mp_data->popup_win, (const int*)(&rots), 4);
     }

   // Create Menu popup
   mp_data->ctxpopup = elm_ctxpopup_add(mp_data->popup_win);
   if (!mp_data->ctxpopup) goto error;

   // Create Rectangle to block events
   mp_data->events_blocker = evas_object_rectangle_add(evas_object_evas_get(mp_data->parent));
   evas_object_color_set(mp_data->events_blocker, 0, 0, 0, 0);
   evas_object_geometry_get(mp_data->parent, &px, &py, &pw, &ph);
   evas_object_repeat_events_set(mp_data->events_blocker, EINA_FALSE);
   evas_object_move(mp_data->events_blocker, px, py);
   evas_object_resize(mp_data->events_blocker, pw, ph);

   // for object color class setting ///
   Evas_Object *edje;

   edje = evas_object_data_get(parent, "color_overlay");
   if (edje) evas_object_data_set(mp_data->ctxpopup, "color_overlay", edje);

   edje = evas_object_data_get(parent, "font_overlay");
   if (edje) evas_object_data_set(mp_data->ctxpopup, "font_overlay", edje);
   /////////////////////////////////////

   elm_object_style_set(mp_data->ctxpopup, "more/default");
   elm_ctxpopup_auto_hide_disabled_set(mp_data->ctxpopup, EINA_TRUE);
   evas_object_data_set(mp_data->ctxpopup, EA_MENU_DATA, mp_data);

   ea_object_event_callback_add(mp_data->ctxpopup, EA_CALLBACK_MORE, ea_ctxpopup_back_cb, NULL);
   ea_object_event_callback_add(mp_data->ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
   evas_object_smart_callback_add(mp_data->ctxpopup, "dismissed", _ctxpopup_dismissed_cb, mp_data);
   evas_object_event_callback_add(mp_data->ctxpopup, EVAS_CALLBACK_DEL, _ctxpopup_del_cb, mp_data);
   evas_object_event_callback_add(mp_data->ctxpopup, EVAS_CALLBACK_SHOW, _ctxpopup_show_cb, mp_data);
   evas_object_event_callback_add(mp_data->ctxpopup, EVAS_CALLBACK_HIDE, _ctxpopup_hide_cb, mp_data);

   evas_object_event_callback_add(mp_data->popup_win, EVAS_CALLBACK_RESIZE, _win_resize_cb, mp_data);
   evas_object_smart_callback_add(mp_data->popup_win, "wm,rotation,changed", _win_rotate_cb, mp_data);
   evas_object_event_callback_add(mp_data->parent, EVAS_CALLBACK_DEL, _ctxpopup_parent_del_cb, mp_data);

   evas_object_smart_callback_add(mp_data->popup_win, "focus,out", _parent_win_focus_out_cb, mp_data->ctxpopup);

   return mp_data->ctxpopup;

error:
   if (mp_data->popup_win) evas_object_del(mp_data->popup_win);
   free(mp_data);
   return NULL;
}

EAPI void
ea_menu_popup_move(Evas_Object *obj)
{
   Menu_Popup_Data *mp_data;
   if (!obj) return;

   mp_data = evas_object_data_get(obj, EA_MENU_DATA);
   if (!mp_data) return;
   if (!(mp_data->ctxpopup || mp_data->events_blocker)) return;

   _resize_events_blocker(mp_data);
   _move_ctxpopup(mp_data->ctxpopup);
}
