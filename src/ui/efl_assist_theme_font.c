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

typedef struct _Ea_Theme_Font_Config {
   const char *font;
   int size;
} Ea_Theme_Font_Config;

typedef struct _Ea_Theme_Font {
   const char *id;
   const char *type;
   const char *style;
   int size;
   Eina_Bool giant;
} Ea_Theme_Font;

typedef struct _Ea_Theme_Object_Font {
   Evas_Object *obj;
   Elm_Object_Item *item;
   const char *code;
   const char *new_code;
} Ea_Theme_Object_Font;

typedef struct _Ea_Theme_App_Font {
   Evas_Object *obj;
   Ea_Theme_Font_Table *table;
} Ea_Theme_App_Font;

// system font table
static const char _data_path[] = PREFIX"/share/themes";
static const char _font_table[] = "FontInfoTable.xml";

static const char _config[] = "font.cfg";
static const char _encoding_key[] = "config";

// data
static Eina_List *_app_fonts;
static Eina_List *_object_fonts;

static void
_font_config_desc_init(Eet_Data_Descriptor **edd)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Ea_Theme_Font_Config);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   *edd = eet_data_descriptor_file_new(&eddc);
   if (!(*edd)) return;

   EET_DATA_DESCRIPTOR_ADD_BASIC(*edd, Ea_Theme_Font_Config, "font", font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(*edd, Ea_Theme_Font_Config, "size", size, EET_T_INT);
}

static Eina_Bool
_font_config_set(Ea_Theme_Font_Config *config)
{
   Eet_Data_Descriptor *edd;
   Eet_File *ef;
   char buf[BUF_SIZE], temp[BUF_SIZE];
   int ret;
   char *path;

   if (!config) return EINA_FALSE;

   path = _theme_user_config_path_get();
   if (!path) return EINA_FALSE;

   ret = ecore_file_mkpath(path);
   if (!ret)
     {
        free(path);
        return EINA_FALSE;
     }

   // write config file for input colors
   snprintf(buf, BUF_SIZE, "%s/%s", path, _config);
   snprintf(temp, BUF_SIZE, "%s/%s.tmp", path, _config);
   free(path);

   ef = eet_open(temp, EET_FILE_MODE_WRITE);
   if (!ef)
     {
        ERR("cannot open font config temp file (%s)", temp);
        return EINA_FALSE;
     }

   _font_config_desc_init(&edd);
   if (!edd)
     {
        eet_close(ef);
        return EINA_FALSE;
     }

   ret = eet_data_write(ef, edd, _encoding_key, config, 1);
   if (!ret)
     {
        ERR("cannot write font config file (%s)", temp);
        eet_data_descriptor_free(edd);
        eet_close(ef);
        return EINA_FALSE;
     }

   eet_data_descriptor_free(edd);
   ret = eet_close(ef);
   if (ret)
     {
        ERR("cannot close font config temp file (%s)", temp);
        return EINA_FALSE;
     }

   ret = ecore_file_mv(temp, buf);
   if (!ret)
     {
        ERR("cannot move font config file (%s -> %s)", temp, buf);
        return EINA_FALSE;
     }

   ecore_file_unlink(temp);

   return EINA_TRUE;
}

static Ea_Theme_Font_Config *
_font_config_get(void)
{
   Ea_Theme_Font_Config *config;
   Eet_Data_Descriptor *edd = NULL;
   Eet_File *ef;
   char buf[BUF_SIZE];
   char *path;

   path = _theme_user_config_path_get();
   if (!path) return NULL;

   snprintf(buf, BUF_SIZE, "%s/%s", path, _config);
   free(path);

   // read config file for input colors
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (!ef)
     {
        path = _theme_system_config_path_get();
        if (!path) return NULL;

        snprintf(buf, BUF_SIZE, "%s/%s", path, _config);
        ef = eet_open(buf, EET_FILE_MODE_READ);
        free(path);
     }
   INFO("read font config file from (%s)", buf);

   if (!ef)
     {
        ERR("cannot open font config file (%s)", buf);
        return NULL;
     }

   _font_config_desc_init(&edd);
   if (!edd)
     {
        eet_close(ef);
        return NULL;
     }

   config = eet_data_read(ef, edd, _encoding_key);
   if (!config)
     {
        ERR("cannot read font config data", buf);
        eet_data_descriptor_free(edd);
        eet_close(ef);
        return NULL;
     }

   eet_data_descriptor_free(edd);
   eet_close(ef);

   return config;
}

static void
_font_config_free(Ea_Theme_Font_Config *config)
{
   if (!config) return;

   eina_stringshare_del(config->font);
   free(config);
}

static void
_initial_font_rule_set(Ea_Theme_Font *font)
{
   font->id = NULL;
   font->type = NULL;
   font->style = NULL;
   font->size = 0;
   font->giant = EINA_FALSE;
}

static Eina_Bool
_font_attribute_parse_cb(void *data,
                         const char *key,
                         const char *value)
{
   Ea_Theme_Font *font = (Ea_Theme_Font *)data;

   if (!font) return EINA_FALSE;

   if (!strcmp(key, "id"))
     font->id = eina_stringshare_add(value);
   else if (!strcmp(key, "typeface"))
     font->type = eina_stringshare_add(value);
   else if (!strcmp(key, "style"))
     font->style = eina_stringshare_add(value);
   else if (!strcmp(key, "size"))
     font->size = atoi(value);
   else if (!strcmp(key, "giant") && !strcmp(value, "true"))
     font->giant = EINA_TRUE;

   return EINA_TRUE;
}

static Eina_Bool
_font_token_parse_cb(void *data,
                    Eina_Simple_XML_Type type,
                    const char *content,
                    unsigned offset EINA_UNUSED,
                    unsigned length)
{
   Eina_List **plist = (Eina_List **)data;

   if (!plist) return EINA_FALSE;

   if (type == EINA_SIMPLE_XML_OPEN_EMPTY) /* <FontInfo> */
     {
        if (!strncmp("FontInfo ", content, strlen("FontInfo ")))
          {
             static Ea_Theme_Font *font;
             const char *buf;

             font = malloc(sizeof(Ea_Theme_Font));
             if (!font) return EINA_FALSE;

             _initial_font_rule_set(font);

             buf = eina_simple_xml_tag_attributes_find(content, length);
             eina_simple_xml_attributes_parse(buf,
                                              length - (buf - content),
                                              _font_attribute_parse_cb,
                                              font);

             *plist = eina_list_append(*plist, font);
          }
     }

   return EINA_TRUE;
}

static void
_font_overlays_set(Eina_List *list, Ea_Theme_Font_Config *fconfig, Eina_Bool config, Evas_Object *edje)
{
   Ea_Theme_Font *font;
   char buf[BUF_SIZE], type[BUF_SIZE], style[BUF_SIZE];
   Eina_List *l;
   int new_size;

   EINA_LIST_FOREACH(list, l, font)
     {
        if (!font->id) continue;

        memset(type, 0, BUF_SIZE);
        if (font->type)
          strcpy(type, font->type);
        else
          strcpy(type, fconfig->font);

        memset(style, 0, BUF_SIZE);
        if (font->style && !strcmp(font->style, "M"))
          strcpy(style, "Bold");
        else if (font->style && !strcmp(font->style, "B"))
          strcpy(style, "Bold");
        else if (font->style && !strcmp(font->style, "L"))
          strcpy(style, "Light");
        else if (font->style && !strcmp(font->style, "T"))
          strcpy(style, "Thin");
        else if (font->style && !strcmp(font->style, "ExB"))
          strcpy(style, "ExtraBold");
        else
          strcpy(style, "Regular");

        snprintf(buf, BUF_SIZE, "%s:style=%s", type, style);
        if (font->giant) new_size = (font->size * -1 * fconfig->size) / 100;
        else new_size = font->size;

        if (config)
          elm_config_font_overlay_set(font->id, buf, new_size);
        else
          {
             if (edje)
               edje_object_text_class_set(edje, font->id, buf, new_size);
             else
               edje_text_class_set(font->id, buf, new_size);
          }
     }
}

static void
_on_object_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ea_Theme_Object_Font *of = (Ea_Theme_Object_Font *)data;

   _object_fonts = eina_list_remove(_object_fonts, of);
   if (of->code) eina_stringshare_del(of->code);
   if (of->new_code) eina_stringshare_del(of->new_code);
   free(of);
}

void
_theme_fonts_update(void *data)
{
   Ea_Theme_App_Font *af;
   Ea_Theme_Object_Font *of;
   Evas_Object *edje;
   char *font;
   int size;
   Eina_List *l;

   DBG("[font update]");

   // application font table reload
   EINA_LIST_FOREACH(_app_fonts, l, af)
     {
        if (af->obj)
          ea_theme_object_fonts_set(af->obj, af->table);
        else
          ea_theme_fonts_set(af->table);
     }

   // object fonts reload
   EINA_LIST_FOREACH(_object_fonts, l, of)
     {
        if (of->obj) // ea_theme_object_font_replace
          edje = elm_layout_edje_get(of->obj);
        else if (of->item) // ea_theme_object_item_font_replace
          edje = elm_object_item_edje_get(of->item);
        else
          edje = NULL;

        if (ea_theme_object_font_get(edje, of->new_code, &font, &size))
          {
             edje_object_text_class_set(edje, of->code, font, size);
             free(font);
          }
     }
}

EAPI Ea_Theme_Font_Table *
ea_theme_font_table_new(const char *file)
{
   Eina_List *table;

   if (!file) return NULL;

   table = _theme_xml_table_get(file, _font_token_parse_cb);
   INFO("font table (%x) from (%s) is created", table, file);

   return table;
}

EAPI void
ea_theme_font_table_free(Ea_Theme_Font_Table *table)
{
   Ea_Theme_Font *font;
   Ea_Theme_App_Font *af;
   Eina_List *l;

   if (!table) return;

   INFO("color table (%x) is freed", table);

   // remove from data
   EINA_LIST_REVERSE_FOREACH(_app_fonts, l, af)
     {
        if (af->table == table)
          return;
     }
   if (af)
     {
        _app_fonts = eina_list_remove_list(_app_fonts, l);
        free(af);
     }

   EINA_LIST_FREE(table, font)
     {
        eina_stringshare_del(font->id);
        eina_stringshare_del(font->type);
        eina_stringshare_del(font->style);
        free(font);
     }
}

EAPI Eina_Bool
ea_theme_fonts_set(Ea_Theme_Font_Table *table)
{
   Ea_Theme_App_Font *af;
   Ea_Theme_Font_Config *config;
   Eina_List *l;

   if (!table) return EINA_FALSE;

   config = _font_config_get();
   if (!config)
     {
        ERR("cannot get font config file");
        return EINA_FALSE;
     }

   _font_overlays_set(table, config, EINA_FALSE, NULL);
   _font_config_free(config);

   // add to data
   EINA_LIST_FOREACH(_app_fonts, l, af)
     {
        if (af->table == table)
          break;
     }
   if (!af)
     {
        af = calloc(1, sizeof(Ea_Theme_App_Font));
        if (af)
          {
             af->table = table;
             _app_fonts = eina_list_append(_app_fonts, af);
          }
     }

   _theme_font_monitor_add();

   return EINA_TRUE;
}

EAPI Eina_Bool
ea_theme_fonts_unset(Ea_Theme_Font_Table *table)
{
   Ea_Theme_Font *font;
   Eina_List *l;

   if (!table) return EINA_FALSE;

   EINA_LIST_FOREACH(table, l, font)
      edje_text_class_del(font->id);

   return EINA_TRUE;
}

EAPI Eina_Bool
ea_theme_font_get(const char *code, char **font, int *size)
{
   Eina_Strbuf *str;
   char *temp;

   if (!code) return EINA_FALSE;

   if (!edje_text_class_get(code, &temp, size))
     return EINA_FALSE;

   if (!temp) return EINA_FALSE;

   str = eina_strbuf_new();
   if (!str)
     {
        *font = NULL;
        *size = 0;
        free(temp);
        return EINA_FALSE;
     }

   eina_strbuf_append_escaped(str, temp);
   free(temp);

   *font = strdup(eina_strbuf_string_get(str));
   eina_strbuf_free(str);
   if (!(*font))
     {
        *size = 0;
        return EINA_FALSE;
     }


   return EINA_TRUE;
}

EAPI Eina_Bool
ea_theme_object_fonts_set(Evas_Object *obj, Ea_Theme_Font_Table *table)
{
   Ea_Theme_App_Font *af;
   Ea_Theme_Font_Config *config;
   Evas_Object *edje;
   Eina_List *l;

   if (!obj) return EINA_FALSE;
   if (!table) return EINA_FALSE;

   edje = elm_layout_edje_get(obj);
   if (!edje) return EINA_FALSE;

   DBG("table: %x, obj: %x (%s)",
       table, obj, evas_object_type_get(obj));

   config = _font_config_get();
   if (!config)
     {
        ERR("cannot get font config file");
        return EINA_FALSE;
     }

   _font_overlays_set(table, config, EINA_FALSE, edje);
   evas_object_data_set(obj, "font_overlay", edje);
   evas_object_data_set(edje, "font_overlay", edje);
   _font_config_free(config);

   // apply to children
   elm_config_font_overlay_apply();

   // add to data
   EINA_LIST_FOREACH(_app_fonts, l, af)
     {
        if (af->obj == obj && af->table == table)
          break;
     }
   if (!af)
     {
        af = calloc(1, sizeof(Ea_Theme_App_Font));
        if (af)
          {
             af->obj = obj;
             af->table = table;
             _app_fonts = eina_list_append(_app_fonts, af);
          }
     }

   _theme_font_monitor_add();

   return EINA_TRUE;
}

EAPI Eina_Bool
ea_theme_object_font_replace(Evas_Object *obj, const char *code, const char *new_code)
{
   Evas_Object *edje;
   char *font;
   int size;

   if (!obj) return EINA_FALSE;
   if (!code) return EINA_FALSE;
   if (!new_code) return EINA_FALSE;

   edje = elm_layout_edje_get(obj);
   if (!edje) return EINA_FALSE;

   if (ea_theme_object_font_get(obj, new_code, &font, &size))
     {
        Ea_Theme_Object_Font *of;

        edje_object_text_class_set(edje, code, font, size);
        INFO("obj: %x (%s), font code replace: (%s -> %s), font: %s, size: %d",
            obj, evas_object_type_get(obj), code, new_code, font, size);
        free(font);

        of = calloc(1, sizeof(Ea_Theme_Object_Font));
        if (of)
          {
             of->obj = obj;
             of->code = eina_stringshare_add(code);
             of->new_code = eina_stringshare_add(new_code);
             evas_object_event_callback_add(edje, EVAS_CALLBACK_DEL,
                                            _on_object_del, of);

             _object_fonts = eina_list_append(_object_fonts, of);
             _theme_font_monitor_add();
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
ea_theme_object_item_font_replace(Elm_Object_Item *item, const char *code, const char *new_code)
{
   Evas_Object *edje;
   char *font;
   int size;

   if (!item) return EINA_FALSE;
   if (!code) return EINA_FALSE;
   if (!new_code) return EINA_FALSE;

   edje = elm_object_item_edje_get(item);
   if (!edje) return EINA_FALSE;

   if (ea_theme_object_font_get(edje, new_code, &font, &size))
     {
        Ea_Theme_Object_Font *of;

        edje_object_text_class_set(edje, code, font, size);
        INFO("item: %x, font code replace: (%s -> %s), font: %s, size: %d",
            item, code, new_code, font, size);
        free(font);

        of = calloc(1, sizeof(Ea_Theme_Object_Font));
        if (of)
          {
             of->item = item;
             of->code = eina_stringshare_add(code);
             of->new_code = eina_stringshare_add(new_code);
             evas_object_event_callback_add(edje, EVAS_CALLBACK_DEL,
                                            _on_object_del, of);

             _object_fonts = eina_list_append(_object_fonts, of);
             _theme_font_monitor_add();
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void
ea_theme_object_fonts_reload(void)
{
   Eina_List *l;
   Ea_Theme_Object_Font *of;
   Evas_Object *edje;
   char *font;
   int size;

   EINA_LIST_FOREACH(_object_fonts, l, of)
     {
        if (of->obj) // ea_theme_object_font_replace
          edje = elm_layout_edje_get(of->obj);
        else if (of->item) // ea_theme_object_item_font_replace
          edje = elm_object_item_edje_get(of->item);
        else
          edje = NULL;

        if (ea_theme_object_font_get(edje, of->new_code, &font, &size))
          {
             edje_object_text_class_set(edje, of->code, font, size);
             free(font);
          }
     }
}

EAPI Eina_Bool
ea_theme_object_font_get(Evas_Object *obj, const char *code, char **font, int *size)
{
   Evas_Object *edje = NULL;
   Eina_Strbuf *str;
   char *temp = NULL;

   if (!obj) return EINA_FALSE;
   if (!code) return EINA_FALSE;

   // get font from object level
   if (evas_object_type_get(obj) &&
       !strcmp(evas_object_type_get(obj), "edje"))
     edje = obj;
   else
     edje = elm_layout_edje_get(obj);
   if (edje)
     {
        if (!edje_object_text_class_get(edje, code, &temp, size))
          return EINA_FALSE;
     }

   // get font from parent level ex) elm_image (not layout object)
   if (!temp)
     {
        edje = evas_object_data_get(obj, "font_overlay");
        if (edje)
          {
             if (!edje_object_text_class_get(edje, code, &temp, size))
               return EINA_FALSE;
          }
     }

   // get font from global level
   if (!temp)
     {
        if (!ea_theme_font_get(code, &temp, size))
          return EINA_FALSE;
     }

   if (!temp) return EINA_FALSE;

   str = eina_strbuf_new();
   if (!str)
     {
        *font = NULL;
        *size = 0;
        free(temp);
        return EINA_FALSE;
     }

   eina_strbuf_append_escaped(str, temp);
   free(temp);

   *font = strdup(eina_strbuf_string_get(str));
   eina_strbuf_free(str);
   if (!(*font))
     {
        *size = 0;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI void
ea_theme_system_fonts_apply(void)
{
#if 0
   Eina_List *table;
   char buf[BUF_SIZE];

   snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _font_table);
   table = ea_theme_font_table_new(buf);
   if (!table) return;

   _font_overlays_set(table, EINA_TRUE, NULL);
   elm_config_all_flush();
   elm_config_save();

   ea_theme_font_table_free(table);
#endif
}

EAPI Eina_Bool
ea_theme_system_font_set(const char *font, int size)
{
   Ea_Theme_Font_Config *config;
   Eina_List *table;
   char buf[BUF_SIZE];
   Eina_Bool ret;

   DBG("font: %s, size: %d", font, size);

   config = calloc(1, sizeof(Ea_Theme_Font_Config));
   if (!config) return EINA_FALSE;

   config->font = eina_stringshare_add(font);
   config->size = size;

   // update system fonts
   snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _font_table);
   table = ea_theme_font_table_new(buf);
   if (!table)
     {
        ERR("cannot get winset font table");
        _font_config_free(config);
        return EINA_FALSE;
     }

   _font_overlays_set(table, config, EINA_TRUE, NULL);

   ret = _font_config_set(config);
   elm_config_all_flush();
   elm_config_save();

   _font_config_free(config);
   ea_theme_font_table_free(table);

   if (!ret)
     {
        ERR("cannot set font config file. Theme is not set!!");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
ea_theme_system_font_get(char **font, int *size)
{
   Ea_Theme_Font_Config *config;

   config = _font_config_get();
   if (!config) return EINA_FALSE;

   if (font)
     {
        *font = malloc(strlen(config->font) + 1);
        if (!(*font)) return EINA_FALSE;
        strcpy(*font, config->font);
     }
   if (size) *size = config->size;

   _font_config_free(config);

   return EINA_TRUE;
}
