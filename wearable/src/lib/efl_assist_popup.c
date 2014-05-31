#include "efl_assist.h"
#include "efl_assist_private.h"

#include <Ecore_X.h>
#define BASE_WIDTH      (720 * elm_config_scale_get())
#define BASE_HEIGHT     (1280 * elm_config_scale_get())

static const char ELM_WIDGET_SMART_NAME[] = "elm_widget";
static const char WIN_SMART_NAME[] = "elm_win";
static const char ELM_CONFORMANT_SMART_NAME[] = "elm_conformant";

typedef struct _Center_Popup_Data Center_Popup_Data;
struct _Center_Popup_Data
{
   Evas_Object *parent;
   Evas_Object *parent_win;
   Evas_Object *block_events;
   Evas_Object *popup_win;
   Evas_Object *popup;
   Evas_Object *conform;
   Ecore_Job   *job;
};

typedef struct _Menu_Popup_Data Menu_Popup_Data;
struct _Menu_Popup_Data
{
   Evas_Object *parent;
   Evas_Object *popup_win;
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

static void _win_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _move_ctxpopup(data);
}

static void _win_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
   _move_ctxpopup(data);
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

   evas_object_show(mp_data->popup_win);
}

static void _ctxpopup_hide_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;

   evas_object_hide(mp_data->popup_win);
}

static void _ctxpopup_parent_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Menu_Popup_Data *mp_data = data;
   if (!mp_data) return;
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

   evas_object_event_callback_del_full(mp_data->popup_win, EVAS_CALLBACK_RESIZE, _win_resize_cb, obj);
   evas_object_smart_callback_del_full(mp_data->popup_win, "wm,rotation,changed", _win_rotate_cb, obj);
   evas_object_smart_callback_del_full(obj, "dismissed", _ctxpopup_dismissed_cb, mp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL, _ctxpopup_del_cb, mp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_SHOW, _ctxpopup_show_cb, mp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_HIDE, _ctxpopup_hide_cb, mp_data);
   evas_object_event_callback_del_full(mp_data->parent, EVAS_CALLBACK_DEL, _ctxpopup_parent_del_cb, mp_data);
   evas_object_smart_callback_del_full(mp_data->popup_win, "focus,out", _parent_win_focus_out_cb, obj);

   evas_object_del(mp_data->popup_win);
   free(mp_data);
}

static void _center_popup_parent_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Object *object = data;
   evas_object_del(object);
}

static void
_block_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   evas_object_smart_callback_call(data, "block,clicked", NULL);
}

static void _popup_win_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Object *popup = data;
   elm_win_resize_object_del(obj, popup);
}

static void _center_popup_resize_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;

   Evas_Coord ox, oy, ow, oh;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);

   evas_object_resize(cp_data->popup_win, ow, oh);
   evas_output_viewport_set(evas_object_evas_get(obj), ox, oy, ow, oh);
   evas_output_size_set(evas_object_evas_get(obj), ow, oh);
}

static void _center_popup_job_cb(void *data)
{
   Center_Popup_Data *cp_data = data;
   cp_data->job = NULL;

   evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_RESIZE, _center_popup_resize_cb, cp_data);

   elm_object_part_content_set(cp_data->conform, "elm.swallow.dim", cp_data->block_events);
   evas_object_show(cp_data->popup_win);
}

static void _center_popup_show_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;

   if (cp_data->job)
     {
        ecore_job_del(cp_data->job);
        cp_data->job = NULL;
     }
   cp_data->job = ecore_job_add(_center_popup_job_cb, cp_data);
}

static void _center_popup_hide_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;
   elm_object_part_content_unset(cp_data->conform, "elm.swallow.dim");
   evas_object_hide(cp_data->popup_win);
}

static void _center_popup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Center_Popup_Data *cp_data = data;
   if (!cp_data) return;

   elm_layout_signal_callback_del(cp_data->block_events, "elm,action,click", "elm", _block_area_clicked_cb);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL, _center_popup_del_cb, cp_data);
   evas_object_event_callback_del_full(cp_data->parent, EVAS_CALLBACK_DEL, _center_popup_parent_del_cb, obj);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_SHOW, _center_popup_show_cb, cp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_HIDE, _center_popup_hide_cb, cp_data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE, _center_popup_resize_cb, cp_data);

   if (cp_data->job)
     {
        ecore_job_del(cp_data->job);
        cp_data->job = NULL;
     }
   elm_object_part_content_unset(cp_data->conform, "elm.swallow.dim");
   evas_object_del(cp_data->block_events);
   evas_object_del(cp_data->popup_win);
   free(cp_data);
}

static Evas_Object *
_obj_tree_find(Evas_Object *obj, const char *name)
{
   Evas_Object *ret = NULL;
   Eina_List *children;
   const char *type = evas_object_type_get(obj);
   if ((type) && !strcmp(type, name))
     return obj;

   if (evas_object_smart_type_check_ptr(obj, ELM_WIDGET_SMART_NAME))
     {
        children = evas_object_smart_members_get(obj);
        EINA_LIST_FREE(children, obj)
          {
             ret = _obj_tree_find(obj, name);
             if (ret)
               {
                  eina_list_free(children);
                  return ret;
               }
          }
     }
   return NULL;
}

static Evas_Object *
_elm_widget_name_find(Evas_Object *obj, const char *name)
{
   Evas_Object *ret = NULL;
   Eina_List *objs;
   if (!obj) return NULL;
   const char *type = evas_object_type_get(obj);
   if ((type) && !strcmp(type, name))
     return obj;
   objs = evas_objects_in_rectangle_get(evas_object_evas_get(obj),
                                        SHRT_MIN, SHRT_MIN, USHRT_MAX, USHRT_MAX, EINA_TRUE, EINA_TRUE);
   EINA_LIST_FREE(objs, obj)
     {
        ret = _obj_tree_find(obj, name);
        if (ret)
          {
             eina_list_free(objs);
             return ret;
          }
     }

   return NULL;
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
   cp_data->parent = parent;
   if (!strcmp(evas_object_type_get(parent), "elm_win"))
     cp_data->parent_win = parent;
   else
     {
        LOGE("The ea_center_popup_add's agument shoud be elm_win object");
        cp_data->parent_win = _elm_widget_name_find(parent, WIN_SMART_NAME);
     }

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

   cp_data->conform = _elm_widget_name_find(parent, ELM_CONFORMANT_SMART_NAME);

   // Create Dimming object
   cp_data->block_events = elm_layout_add(cp_data->parent_win);
   elm_layout_theme_set(cp_data->block_events, "notify", "block_events", "center_popup");
   evas_object_size_hint_weight_set(cp_data->block_events, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cp_data->block_events, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_repeat_events_set(cp_data->block_events, EINA_FALSE);

   cp_data->popup = elm_popup_add(cp_data->popup_win);
   if (!cp_data->popup) goto error;
   elm_object_style_set(cp_data->popup, "center_popup");

   elm_popup_allow_events_set(cp_data->popup, EINA_TRUE);
   evas_object_size_hint_weight_set(cp_data->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(cp_data->popup_win, cp_data->popup);

   elm_layout_signal_callback_add(cp_data->block_events, "elm,action,click", "elm", _block_area_clicked_cb, cp_data->popup);
   evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_SHOW, _center_popup_show_cb, cp_data);
   evas_object_event_callback_add(cp_data->popup, EVAS_CALLBACK_HIDE, _center_popup_hide_cb, cp_data);
   evas_object_event_callback_priority_add(cp_data->popup, EVAS_CALLBACK_DEL, EVAS_CALLBACK_PRIORITY_BEFORE, _center_popup_del_cb, cp_data);
   evas_object_event_callback_add(parent, EVAS_CALLBACK_DEL, _center_popup_parent_del_cb, cp_data->popup);
   evas_object_event_callback_add(cp_data->popup_win, EVAS_CALLBACK_DEL, _popup_win_del_cb, cp_data->popup);

   return cp_data->popup;

error:
   if (cp_data->block_events)
     {
        elm_object_part_content_unset(cp_data->conform, "elm.swallow.dim");
        evas_object_del(cp_data->block_events);
     }
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
   Evas_Object *ctxpopup = NULL;
   Evas_Object *parent_win;
   Menu_Popup_Data *mp_data;

   mp_data = calloc(1, sizeof(Menu_Popup_Data));
   mp_data->parent = parent;

   parent_win = evas_object_top_get(evas_object_evas_get(parent));

   // Create Menu popup window
   mp_data->popup_win = elm_win_add(parent_win, "Menu Popup", ELM_WIN_BASIC);
   if (!mp_data->popup_win) goto error;
   ecore_x_icccm_transient_for_set(elm_win_xwindow_get(mp_data->popup_win), elm_win_xwindow_get(parent_win));
   elm_win_alpha_set(mp_data->popup_win, EINA_TRUE);
   elm_win_role_set(mp_data->popup_win, "no-effect");

   if (elm_win_wm_rotation_supported_get(mp_data->popup_win))
     {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(mp_data->popup_win, (const int*)(&rots), 4);
     }

   // Create Menu popup
   ctxpopup = elm_ctxpopup_add(mp_data->popup_win);
   if (!ctxpopup) goto error;

   elm_object_style_set(ctxpopup, "more/default");
   elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

   ea_object_event_callback_add(ctxpopup, EA_CALLBACK_MORE, ea_ctxpopup_back_cb, NULL);
   ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _ctxpopup_dismissed_cb, mp_data);
   evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_DEL, _ctxpopup_del_cb, mp_data);
   evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_SHOW, _ctxpopup_show_cb, mp_data);
   evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_HIDE, _ctxpopup_hide_cb, mp_data);

   evas_object_event_callback_add(mp_data->popup_win, EVAS_CALLBACK_RESIZE, _win_resize_cb, ctxpopup);
   evas_object_smart_callback_add(mp_data->popup_win, "wm,rotation,changed", _win_rotate_cb, ctxpopup);
   evas_object_event_callback_add(mp_data->parent, EVAS_CALLBACK_DEL, _ctxpopup_parent_del_cb, mp_data);

   evas_object_smart_callback_add(mp_data->popup_win, "focus,out", _parent_win_focus_out_cb, ctxpopup);

   return ctxpopup;

error:
   if (mp_data->popup_win) evas_object_del(mp_data->popup_win);
   free(mp_data);
   return NULL;
}

EAPI void
ea_menu_popup_move(Evas_Object *obj)
{
   if (!obj) return;
   _move_ctxpopup(obj);
}

