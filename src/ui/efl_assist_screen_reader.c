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
#include <vconf.h>
#include <tts.h>

static tts_h tts = NULL;

static void _tts_shutdown(void)
{
   int ret = 0;
   if (tts)
     {
        /* check current state */
        tts_state_e state;
        tts_get_state(tts, &state);
        if (state == TTS_STATE_PLAYING || state == TTS_STATE_PAUSED)
          {
             ret = tts_stop(tts);
             if (TTS_ERROR_NONE != ret)
               {
                  fprintf(stderr, "Fail to stop handle : result(%d)", ret);
                  return;
               }
          }

        /* it is possible to try to shutdown before the state is ready,
           because tts_prepare(); works Asynchronously. see elm_modapi_init(): */
        if (state == TTS_STATE_READY)
          {
             ret = tts_unprepare(tts);
             if (TTS_ERROR_NONE != ret)
               {
                  fprintf(stderr, "Fail to unprepare handle : result(%d)", ret);
                  return;
               }

             ret = tts_unset_state_changed_cb(tts);
             if (TTS_ERROR_NONE != ret)
               {
                  fprintf(stderr, "Fail to set callback : result(%d)", ret);
                  return;
               }
          }

        ret = tts_destroy(tts);
        if (TTS_ERROR_NONE != ret)
          {
             fprintf(stderr, "Fail to destroy handle : result(%d)", ret);
             return;
          }

        if (tts) tts = NULL;
     }
}

void _tts_state_changed_cb(tts_h tts, tts_state_e previous, tts_state_e current, void* data)
{
   int ret = 0;
   int u_id = 0;

   if (TTS_STATE_CREATED == previous && TTS_STATE_READY == current)
     {
        ret = tts_add_text(tts, ASSIST_STR_UNAVAILABLE_TEXT, NULL, TTS_VOICE_TYPE_AUTO,
                           TTS_SPEED_AUTO, &u_id);
        if (TTS_ERROR_NONE != ret)
          {
             fprintf(stderr, "Fail to add kept text : ret(%d)\n", ret);
          }

        ret = tts_play(tts);
        if (TTS_ERROR_NONE != ret)
          {
             fprintf(stderr, "Fail to play TTS : ret(%d)\n", ret);
          }
     }
}

static void _tts_init(void)
{
   int ret = 0;

   ret = tts_create(&tts);
   if (TTS_ERROR_NONE != ret)
     {
        fprintf(stderr, "Fail to get handle : result(%d)", ret);
        return;
     }

   ret = tts_set_state_changed_cb(tts, _tts_state_changed_cb, NULL);
   if (TTS_ERROR_NONE != ret)
     {
        fprintf(stderr, "Fail to set callback : result(%d)", ret);
        return;
     }

   ret = tts_set_mode(tts, TTS_MODE_SCREEN_READER);
   if (TTS_ERROR_NONE != ret)
     {
        fprintf(stderr, "Fail to set mode : result(%d)", ret);
        return;
     }

   ret = tts_prepare(tts);
   if (TTS_ERROR_NONE != ret)
     {
        fprintf(stderr, "Fail to prepare handle : result(%d)", ret);
        return;
     }
}

static void _timeout_cb(void *data, Evas_Object *obj, void *event_info)
{

   _tts_shutdown();
   evas_object_del(obj);
}

static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   _tts_shutdown();
   evas_object_del(obj);
}

static Eina_Bool _init_tts_cb(void *data)
{
   _tts_init();
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_is_screen_reader_enable(void)
{
   int tts_val;

   if (vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &tts_val) != 0)
     return EINA_FALSE;

   if (!tts_val) return EINA_FALSE;
   else return EINA_TRUE;
}

static Eina_Bool
_unsupported_popup_show(Evas_Object *win)
{
   Evas_Object *popup;
   int tts_val;

   if (vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &tts_val) != 0)
     return EINA_FALSE;

   if (!tts_val) return EINA_FALSE;

   ecore_idler_add(_init_tts_cb, NULL);

   popup = elm_popup_add(win);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_domain_translatable_text_set(popup, "efl-assist", ASSIST_STR_UNAVAILABLE_TEXT);
   elm_popup_timeout_set(popup, 12.0);
   evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
   ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _block_clicked_cb, NULL);

   evas_object_show(popup);
   return EINA_TRUE;
}

static char *_access_info_prepend_cb(void *data, Evas_Object *obj)
{
   char *ret;
   char name[256];
   Eina_Strbuf *buf;
   void *app_data;
   Elm_Access_Info_Cb func;
   Elm_Access_Info_Type type = (Elm_Access_Info_Type)data;

   buf = eina_strbuf_new();

   sprintf(name, "ea_origin_access_info_%d", type);
   ret = (char *)evas_object_data_get(obj, name);
   sprintf(name, "ea_access_info_data_%d", type);
   app_data = evas_object_data_get(obj, name);
   sprintf(name, "ea_access_info_func_%d", type);
   func = evas_object_data_get(obj, name);

   printf(" func : %p\n", func);
   eina_strbuf_append(buf, func(app_data, obj));
   if (ret)
     {
        int length = eina_strbuf_length_get(buf);
        const char *pre_txt = eina_strbuf_string_get(buf);
        char end = pre_txt[length-1];
        if (length > 0 && end != '?' && end != '!' && end != '.')
          eina_strbuf_append(buf, ", ");
        else
          eina_strbuf_append(buf, " ");
        eina_strbuf_append(buf, ret);
     }
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return ret;
}

static char *_access_info_append_cb(void *data, Evas_Object *obj)
{
   char *ret;
   char name[256];
   Eina_Strbuf *buf;
   void *app_data;
   Elm_Access_Info_Cb func;
   Elm_Access_Info_Type type = (Elm_Access_Info_Type)data;

   buf = eina_strbuf_new();

   sprintf(name, "ea_origin_access_info_%d", type);
   ret = (char *)evas_object_data_get(obj, name);
   sprintf(name, "ea_access_info_data_%d", type);
   app_data = evas_object_data_get(obj, name);
   sprintf(name, "ea_access_info_func_%d", type);
   func = evas_object_data_get(obj, name);

   if (ret)
     {
        eina_strbuf_append(buf, ret);

        int length = eina_strbuf_length_get(buf);
        const char *pre_txt = eina_strbuf_string_get(buf);
        char end = pre_txt[length-1];
        if (length > 0 && end != '?' && end != '!' && end != '.')
          eina_strbuf_append(buf, ", ");
        else
          eina_strbuf_append(buf, " ");
     }
   eina_strbuf_append(buf, func(app_data, obj));
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return ret;
}

EAPI Eina_Bool
ea_screen_reader_support_set(Evas_Object *win, Eina_Bool support)
{
   Ecore_X_Window xwin;
   unsigned int val;

   if (!win) return EINA_FALSE;

   xwin = elm_win_xwindow_get(win);
   if (!xwin) return EINA_FALSE;

   if (support)
     {
        if (_is_screen_reader_enable()) elm_config_access_set(EINA_TRUE);

        val = 0;
        ecore_x_window_prop_card32_set
          (xwin, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL, &val, 1);
     }
   else
     {
        elm_config_access_set(EINA_FALSE);

        val = 2;
        ecore_x_window_prop_card32_set
          (xwin, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL, &val, 1);

        return _unsupported_popup_show(win);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
ea_screen_reader_support_get(Evas_Object *win)
{
   Ecore_X_Window xwin;
   int ret;
   unsigned int val;

   if (!win) return EINA_FALSE;

   xwin = elm_win_xwindow_get(win);
   if (!xwin) return EINA_FALSE;

   ret = ecore_x_window_prop_card32_get
      (xwin, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL, &val, 1);

   if ((ret >= 0) && (val == 2)) return EINA_FALSE;

   return EINA_TRUE;
}

EAPI Eina_Bool
ea_screen_reader_window_property_set(Evas_Object *win, Eina_Bool support)
{
   Ecore_X_Window xwin;
   unsigned int val;

   if (!win) return EINA_FALSE;

   xwin = elm_win_xwindow_get(win);
   if (!xwin) return EINA_FALSE;

   if (support)
     {
        if (_is_screen_reader_enable()) elm_config_access_set(EINA_TRUE);

        val = 0;
        ecore_x_window_prop_card32_set
          (xwin, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL, &val, 1);
     }
   else
     {
        elm_config_access_set(EINA_FALSE);

        val = 2;
        ecore_x_window_prop_card32_set
          (xwin, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL, &val, 1);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
ea_screen_reader_window_property_get(Evas_Object *win)
{
   Eina_Bool return_val = EINA_FALSE;
   if (!win) return return_val;

   return_val = ea_screen_reader_support_get(win);
   return return_val;
}

EAPI Eina_Bool
ea_screen_reader_unsupported_popup_show(Evas_Object *win)
{
   if (!win) return EINA_FALSE;

   return _unsupported_popup_show(win);
}

EAPI void
ea_screen_reader_access_info_prepend_cb_set(Evas_Object *obj, Elm_Access_Info_Type type, Elm_Access_Info_Cb func, void *data)
{
   char *info = NULL;
   char name[256];

   info = elm_access_info_get(obj, type);
   sprintf(name, "ea_origin_access_info_%d", type);
   evas_object_data_set(obj, name, info);
   sprintf(name, "ea_access_info_data_%d", type);
   evas_object_data_set(obj, name, data);
   sprintf(name, "ea_access_info_func_%d", type);
   evas_object_data_set(obj, name, func);
   elm_access_info_cb_set(obj, type, _access_info_prepend_cb, (void *)type);
}

EAPI void
ea_screen_reader_access_info_append_cb_set(Evas_Object *obj, Elm_Access_Info_Type type, Elm_Access_Info_Cb func, void *data)
{
   char *info = NULL;
   char name[256];

   info = elm_access_info_get(obj, type);
   sprintf(name, "ea_origin_access_info_%d", type);
   evas_object_data_set(obj, name, info);
   sprintf(name, "ea_access_info_data_%d", type);
   evas_object_data_set(obj, name, data);
   sprintf(name, "ea_access_info_func_%d", type);
   evas_object_data_set(obj, name, func);
   elm_access_info_cb_set(obj, type, _access_info_append_cb, (void *)type);
}
