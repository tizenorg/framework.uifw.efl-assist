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

typedef struct _Ea_Theme_Event_Callback
{
   Ea_Theme_Callback_Type type;
   void (*func)(void *data);
   void *data;
} Ea_Theme_Event_Callback;

static const char _config_path[] = PREFIX"/share/elementary/config";
static const char _home_path[] = "/opt/home/app/.elementary/config";
static const char _color_config[] = "color.cfg";
static const char _font_config[] = "font.cfg";

// internal data
static Eina_Bool _changeable;
static Ecore_File_Monitor *_color_monitor, *_font_monitor;
static Ecore_Event_Handler *_event_handler;
static Eina_List *_callbacks;

// internal function
static void _on_evas_del(void *data, Evas *e, void *event_info);

static void
_color_config_changed(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
   if (event ==  ECORE_FILE_EVENT_MODIFIED)
     {
        Ea_Theme_Event_Callback *callback;
        Eina_List *l;

        DBG("color config file is modified");

        EINA_LIST_FOREACH(_callbacks, l, callback)
          {
             if(callback->type == EA_THEME_CALLBACK_TYPE_COLOR)
               callback->func(callback->data);
          }
     }
}

static Eina_Bool
_elm_config_changed(void *data, int ev_type, void *ev)
{
   Ea_Theme_Event_Callback *callback;
   Eina_List *l;

   EINA_LIST_FOREACH(_callbacks, l, callback)
     {
        if(callback->type == EA_THEME_CALLBACK_TYPE_FONT)
          callback->func(callback->data);
     }

   ecore_event_handler_del(_event_handler);
   _event_handler = NULL;

   return ECORE_CALLBACK_PASS_ON;
}

static void
_font_config_changed(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
   if (event ==  ECORE_FILE_EVENT_MODIFIED)
     {
        DBG("font config file is modified");

        if (_event_handler)
          {
             ecore_event_handler_del(_event_handler);
             _event_handler = NULL;
          }

        _event_handler = ecore_event_handler_add(
           ELM_EVENT_CONFIG_ALL_CHANGED, _elm_config_changed, NULL);
     }
}

static void
_theme_changeable_ui_data_next_set(Eina_Bool enabled)
{
   Eina_List *eelist, *last, *prev;
   Ecore_Evas *ee;

   eelist = ecore_evas_ecore_evas_list_get();
   if (!eelist) return;

   last = eina_list_last(eelist);
   prev = eina_list_prev(last);
   ee = eina_list_data_get(prev);

   if (ee)
     {
        Evas *evas;

        if (enabled)
          ecore_evas_data_set(ee, "changeable_ui", (void *)1);
        else
          ecore_evas_data_set(ee, "changeable_ui", (void *)0);

        evas = ecore_evas_get(ee);
        evas_event_callback_add(evas, EVAS_CALLBACK_DEL, _on_evas_del, ee);
        INFO("changeable state [%d] is set to ecore_evas [%x]", enabled, ee);
     }
   eina_list_free(eelist);
}

static void
_on_evas_del(void *data, Evas *e, void *event_info)
{
   Ecore_Evas *ee = (Ecore_Evas *)data;
   void *cdata;

   cdata = ecore_evas_data_get(ee, "changeable_ui");
   if (!cdata) return;

   INFO("ecore_evas [%x] will be deleted", ee);

   if ((int)cdata == 1)
     _theme_changeable_ui_data_next_set(EINA_TRUE);
   else if ((int)cdata == 0)
     _theme_changeable_ui_data_next_set(EINA_FALSE);
}

Eina_List *
_theme_xml_table_get(const char *file, Eina_Simple_XML_Cb func)
{
   FILE *fp;
   Eina_List *table = NULL;

   fp = fopen(file, "rb");
   if (fp)
     {
        long sz;

        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);

        if (sz > 0)
          {
             char *buf;

             fseek(fp, 0, SEEK_SET);
             buf = malloc(sz);

             if (buf)
               {
                  if(fread(buf, 1, sz, fp))
                    eina_simple_xml_parse(buf, sz, EINA_TRUE,
                                          func, &table);
                  free(buf);
               }
          }
        fclose(fp);
     }

   return table;
}
char *
_theme_system_config_path_get(void)
{
   char *path;

   path = malloc(BUF_SIZE);
   snprintf(path, BUF_SIZE, "%s/%s", _config_path, elm_config_profile_get());

   return path;
}

char *
_theme_user_config_path_get(void)
{
   char *path;

   path = malloc(BUF_SIZE);
   snprintf(path, BUF_SIZE, "%s/%s", _home_path, elm_config_profile_get());

   return path;
}

void
_theme_changeable_ui_data_set(Eina_Bool enabled)
{
   Eina_List *eelist, *last;
   Ecore_Evas *ee;

   eelist = ecore_evas_ecore_evas_list_get();
   if (!eelist) return;

   last = eina_list_last(eelist);
   ee = eina_list_data_get(last);

   if (ee)
     {
        Evas *evas;

        if (enabled)
          ecore_evas_data_set(ee, "changeable_ui", (void *)1);
        else
          ecore_evas_data_set(ee, "changeable_ui", (void *)0);

        evas = ecore_evas_get(ee);
        evas_event_callback_add(evas, EVAS_CALLBACK_DEL, _on_evas_del, ee);
        INFO("changeable state [%d] is set to ecore_evas [%x]", enabled, ee);
     }
   eina_list_free(eelist);
}

void
_theme_color_monitor_add(void)
{
   char buf[BUF_SIZE];

   if (_color_monitor) return;

   snprintf(buf, BUF_SIZE, "%s/%s/%s", _home_path, elm_config_profile_get(), _color_config);
   _color_monitor = ecore_file_monitor_add(buf, _color_config_changed, NULL);

   if (_color_monitor)
     {
        ea_theme_event_callback_add(EA_THEME_CALLBACK_TYPE_COLOR,
                                    _theme_colors_update, NULL);
        DBG("color config file (%s) monitor is added!!", buf);
     }
}

void
_theme_font_monitor_add(void)
{
   char buf[BUF_SIZE];

   if (_font_monitor) return;

   snprintf(buf, BUF_SIZE, "%s/%s/%s", _home_path, elm_config_profile_get(), _font_config);
   _font_monitor = ecore_file_monitor_add(buf, _font_config_changed, NULL);

   if (_font_monitor)
     {
        ea_theme_event_callback_add(EA_THEME_CALLBACK_TYPE_FONT,
                                    _theme_fonts_update, NULL);
        DBG("font config file (%s) monitor is added!!", buf);
     }
}

void
ea_theme_changeable_ui_enabled_set(Eina_Bool enabled)
{
   if (_changeable == enabled) return;
   _changeable = enabled;

   DBG("changeable state is set to %d", _changeable);

   _theme_color_monitor_add();
   _theme_changeable_ui_data_set(enabled);

   // set colors for default style
   ea_theme_style_set(EA_THEME_STYLE_DEFAULT);
}

Eina_Bool
ea_theme_changeable_ui_enabled_get(void)
{
   return _changeable;
}

void
ea_theme_object_changeable_ui_enabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   Evas_Object *edje;
   void *data = NULL;

   edje = elm_layout_edje_get(obj);
   if (!edje) return;

   data = evas_object_data_get(obj, "changeable_ui");
   if ((data && enabled) || (!data && !enabled)) return;

   DBG("changeable state is set to %d on object %x (%s)",
       enabled, obj, evas_object_type_get(obj));

   _theme_color_monitor_add();

   if (enabled)
     evas_object_data_set(obj, "changeable_ui", (void *)1);
   else
     evas_object_data_set(obj, "changeable_ui", NULL);

   // set colors for default style
   ea_theme_object_style_set(obj, EA_THEME_STYLE_DEFAULT);
}

void
ea_theme_event_callback_add(Ea_Theme_Callback_Type type, Ea_Theme_Event_Cb func, void *data)
{
   Ea_Theme_Event_Callback *callback;

   callback = calloc(1, sizeof(Ea_Theme_Event_Callback));
   if (!callback) return;

   if (type == EA_THEME_CALLBACK_TYPE_COLOR)
     _theme_color_monitor_add();
   else if (type == EA_THEME_CALLBACK_TYPE_FONT)
     _theme_font_monitor_add();

   callback->type = type;
   callback->func = func;
   callback->data = data;

   _callbacks = eina_list_append(_callbacks, callback);
}

void *
ea_theme_event_callback_del(Ea_Theme_Callback_Type type, Ea_Theme_Event_Cb func)
{
   Ea_Theme_Event_Callback *callback;
   Eina_List *l;
   void *data;

   EINA_LIST_REVERSE_FOREACH(_callbacks, l, callback)
     {
        if ((callback->type == type) && (callback->func == func))
          break;
     }
   if (!callback) return NULL;

   data = callback->data;
   _callbacks = eina_list_remove_list(_callbacks, l);
   free(callback);

   return data;
}
