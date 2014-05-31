#include "efl_assist.h"
#include "efl_assist_private.h"

const char *EA_EF_KEY_DATA = "_ea_ef_key_data";

typedef struct _Ea_Editfield_Data
{
   Eina_Bool clear_btn_disabled;
} Ea_Editfield_Data;

static void _editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   Ea_Editfield_Data *eed;

   eed = (Ea_Editfield_Data *)evas_object_data_get(obj, EA_EF_KEY_DATA);
   if (eed && !(eed->clear_btn_disabled)
       && elm_object_part_content_get(obj, "elm.swallow.clear"))
     {
        if (elm_object_focus_get(obj))
          {
             if (elm_entry_is_empty(obj))
               elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
             else
               elm_object_signal_emit(obj, "elm,state,clear,visible", "");
          }
     }
}

static void _editfield_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
   Ea_Editfield_Data *eed;

   eed = (Ea_Editfield_Data *)evas_object_data_get(obj, EA_EF_KEY_DATA);
   if (eed && !(eed->clear_btn_disabled)
       && elm_object_part_content_get(obj, "elm.swallow.clear"))
     {
        if (!elm_entry_is_empty(obj))
          elm_object_signal_emit(obj, "elm,state,clear,visible", "");
        else
          elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
     }
   elm_object_signal_emit(obj, "elm,state,focus,on", "");
}

static void _editfield_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
   Ea_Editfield_Data *eed;

   eed = (Ea_Editfield_Data *)evas_object_data_get(obj, EA_EF_KEY_DATA);
   if (eed && !(eed->clear_btn_disabled)
       && elm_object_part_content_get(obj, "elm.swallow.clear"))
     elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
   elm_object_signal_emit(obj, "elm,state,focus,off", "");
}

static void _eraser_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_entry_entry_set(data, "");
}

static void _editfield_searchbar_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   Ea_Editfield_Data *eed;

   eed = (Ea_Editfield_Data *)evas_object_data_get(obj, EA_EF_KEY_DATA);
   if (eed && !(eed->clear_btn_disabled)
       && elm_object_part_content_get(obj, "elm.swallow.clear"))
     {
        if (elm_entry_is_empty(obj))
          elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
        else
          elm_object_signal_emit(obj, "elm,state,clear,visible", "");
     }
}

static void _editfield_searchbar_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_object_signal_emit(obj, "elm,state,focus,on", "");
}

static void _editfield_searchbar_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_object_signal_emit(obj, "elm,state,focus,off", "");
}

EXPORT_API Evas_Object *
ea_editfield_add(Evas_Object *parent, Ea_Editfield_Type type)
{
   Evas_Object *entry, *button;
   Ea_Editfield_Data *eed;

   entry = elm_entry_add(parent);

   if (type == EA_EDITFIELD_SINGLELINE)
     {
        elm_object_style_set(entry, "editfield");
        elm_entry_single_line_set(entry, EINA_TRUE);
     }
   else if (type == EA_EDITFIELD_SCROLL_SINGLELINE)
     {
        elm_object_style_set(entry, "editfield");
        elm_entry_single_line_set(entry, EINA_TRUE);
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_object_signal_emit(entry, "elm,state,scroll,enabled", "");
     }
   else if (type == EA_EDITFIELD_SCROLL_SINGLELINE_PASSWORD)
     {
        elm_object_style_set(entry, "editfield");
        elm_entry_single_line_set(entry, EINA_TRUE);
        elm_entry_password_set(entry, EINA_TRUE);
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_object_signal_emit(entry, "elm,state,scroll,enabled", "");
     }
   else if (type == EA_EDITFIELD_SCROLL_MULTILINE)
     {
        elm_object_style_set(entry, "editfield");
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_object_signal_emit(entry, "elm,state,scroll,enabled", "");
     }
   else if (type == EA_EDITFIELD_SEARCHBAR)
     {
        elm_object_style_set(entry, "editfield/searchbar/default");
        elm_entry_single_line_set(entry, EINA_TRUE);
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_object_signal_emit(entry, "elm,state,scroll,enabled", "");
     }
   else
     {
        elm_object_style_set(entry, "editfield");
     }

   if (type == EA_EDITFIELD_SEARCHBAR)
     {
        button = elm_button_add(parent);
        elm_object_style_set(button, "search_clear");
        elm_object_focus_allow_set(button, EINA_FALSE);
        elm_object_part_content_set(entry, "elm.swallow.clear", button);
        evas_object_smart_callback_add(button, "clicked", _eraser_btn_clicked_cb, entry);

        evas_object_smart_callback_add(entry, "changed", _editfield_searchbar_changed_cb, NULL);
        evas_object_smart_callback_add(entry, "preedit,changed", _editfield_searchbar_changed_cb, NULL);
        evas_object_smart_callback_add(entry, "focused", _editfield_searchbar_focused_cb, NULL);
        evas_object_smart_callback_add(entry, "unfocused", _editfield_searchbar_unfocused_cb, NULL);
     }
   else
     {
        button = elm_button_add(parent);
        elm_object_style_set(button, "editfield_clear");
        elm_object_focus_allow_set(button, EINA_FALSE);
        elm_object_part_content_set(entry, "elm.swallow.clear", button);
        evas_object_smart_callback_add(button, "clicked", _eraser_btn_clicked_cb, entry);

        evas_object_smart_callback_add(entry, "changed", _editfield_changed_cb, NULL);
        evas_object_smart_callback_add(entry, "preedit,changed", _editfield_changed_cb, NULL);
        evas_object_smart_callback_add(entry, "focused", _editfield_focused_cb, NULL);
        evas_object_smart_callback_add(entry, "unfocused", _editfield_unfocused_cb, NULL);
     }

   eed = calloc(1, sizeof(Ea_Editfield_Data));
   eed->clear_btn_disabled = EINA_FALSE;
   evas_object_data_set(entry, EA_EF_KEY_DATA, eed);
   return entry;
}

EXPORT_API void
ea_editfield_clear_button_disabled_set(Evas_Object *obj, Eina_Bool disable)
{
   Ea_Editfield_Data *eed;

   eed = evas_object_data_get(obj, EA_EF_KEY_DATA);
   if (eed)
     {
        eed->clear_btn_disabled = !!disable;
        if (eed->clear_btn_disabled)
          elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
     }
}

EXPORT_API Eina_Bool
ea_editfield_clear_button_disabled_get(Evas_Object *obj)
{
   Ea_Editfield_Data *eed;

   if (!obj)
     return EINA_FALSE;
   eed = evas_object_data_get(obj, EA_EF_KEY_DATA);
   if (!eed)
     return EINA_FALSE;

   return eed->clear_btn_disabled;
}
