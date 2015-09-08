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

typedef struct _Ea_Theme_Color_Config {
   int index;
   const char *style;
   Eina_List *list;
} Ea_Theme_Color_Config;

typedef struct _Ea_Theme_Color_Rule {
   const char *id;
   const char *input;
   int h;
   int s;
   int v;
   int a;
   int h_max;
   int h_min;
   Eina_Bool h_fixed;
   int s_max;
   int s_min;
   Eina_Bool s_fixed;
   int v_max;
   int v_min;
   Eina_Bool v_fixed;
} Ea_Theme_Color_Rule;

typedef struct _Ea_Theme_Color_Table_Item {
   Eina_List *dark;
   Eina_List *light;
} Ea_Theme_Color_Table_Item;

typedef struct _Ea_Theme_Input_Colors {
   int index;
   Eina_List *list;
} Ea_Theme_Input_Colors;

typedef struct _Ea_Theme_Object_Color {
   Evas_Object *obj;
   Elm_Object_Item *item;
   const char *code;
   const char *new_code;
} Ea_Theme_Object_Color;

typedef struct _Ea_Theme_App_Color {
   Evas_Object *obj;
   Ea_Theme_Color_Table *table;
   Ea_Theme_Style style;
} Ea_Theme_App_Color;

// system color table
static const char _data_path[] = PREFIX"/share/themes";
static const char _color_table_1[] = "ChangeableColorTable1.xml";
static const char _color_table_2[] = "ChangeableColorTable2.xml";
static const char _color_table_3[] = "ChangeableColorTable3.xml";
static const char _input_color_table[] = "InputColorTable.xml";

static const char _encoding_key[] = "config";
static const char _config[] = "color.cfg";

// data
static Ea_Theme_Style _cur_style;
static Eina_List *_app_colors;
static Eina_List *_object_colors;

static void
_color_config_desc_init(Eet_Data_Descriptor **edd,
                        Eet_Data_Descriptor **color_edd)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Ea_Theme_Color_Config);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   *edd = eet_data_descriptor_file_new(&eddc);
   if (!(*edd)) return;

   memset(&eddc, 0, sizeof(eddc));
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Ea_Theme_Color_hsv);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   *color_edd = eet_data_descriptor_file_new(&eddc);
   if (!(*color_edd))
     {
        eet_data_descriptor_free(*edd);
        return;
     }

   EET_DATA_DESCRIPTOR_ADD_BASIC(*color_edd, Ea_Theme_Color_hsv, "h", h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(*color_edd, Ea_Theme_Color_hsv, "s", s, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(*color_edd, Ea_Theme_Color_hsv, "v", v, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(*edd, Ea_Theme_Color_Config, "index", index, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(*edd, Ea_Theme_Color_Config, "style", style, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(*edd, Ea_Theme_Color_Config, "list", list, *color_edd);
}

static Eina_Bool
_color_config_set(Ea_Theme_Color_Config *config)
{
   Eet_Data_Descriptor *edd, *color_edd;
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
        ERR("cannot open color config temp file (%s)", temp);
        return EINA_FALSE;
     }

   _color_config_desc_init(&edd, &color_edd);
   if (!edd)
     {
        eet_close(ef);
        return EINA_FALSE;
     }

   ret = eet_data_write(ef, edd, _encoding_key, config, 1);
   if (!ret)
     {
        ERR("cannot write color config file (%s)", temp);
        eet_data_descriptor_free(color_edd);
        eet_data_descriptor_free(edd);
        eet_close(ef);
        return EINA_FALSE;
     }

   eet_data_descriptor_free(color_edd);
   eet_data_descriptor_free(edd);

   ret = eet_close(ef);
   if (ret)
     {
        ERR("cannot close color config temp file (%s)", temp);
        return EINA_FALSE;
     }

   ret = ecore_file_mv(temp, buf);
   if (!ret)
     {
        ERR("cannot move color config file (%s -> %s)", temp, buf);
        return EINA_FALSE;
     }

   ecore_file_unlink(temp);

   return EINA_TRUE;
}

static Ea_Theme_Color_Config *
_color_config_get(Eina_Bool is_user)
{
   Ea_Theme_Color_Config *config;
   Eet_Data_Descriptor *edd = NULL, *color_edd = NULL;
   Eet_File *ef = NULL;
   char buf[BUF_SIZE];
   char *path;

   if (is_user) // read user config
     {
        path = _theme_user_config_path_get();
        if (!path) return NULL;

        snprintf(buf, BUF_SIZE, "%s/%s", path, _config);
        ef = eet_open(buf, EET_FILE_MODE_READ);
        free(path);
     }
   if (!ef) // read system config
     {
        path = _theme_system_config_path_get();
        if (!path) return NULL;

        snprintf(buf, BUF_SIZE, "%s/%s", path, _config);
        ef = eet_open(buf, EET_FILE_MODE_READ);
        free(path);
     }
   INFO("read color config file from (%s)", buf);

   if (!ef)
     {
        ERR("cannot open color config file (%s)", buf);
        return NULL;
     }

   _color_config_desc_init(&edd, &color_edd);
   if (!edd)
     {
        eet_close(ef);
        return NULL;
     }

   config = eet_data_read(ef, edd, _encoding_key);
   if (!config)
     {
        ERR("cannot read color config data", buf);
        eet_data_descriptor_free(color_edd);
        eet_data_descriptor_free(edd);
        eet_close(ef);
        return NULL;
     }

   eet_data_descriptor_free(color_edd);
   eet_data_descriptor_free(edd);
   eet_close(ef);

   return config;
}

static void
_color_config_free(Ea_Theme_Color_Config *config)
{
   Ea_Theme_Color_hsv *color;

   if (!config) return;

   eina_stringshare_del(config->style);
   EINA_LIST_FREE(config->list, color)
      free(color);
   free(config);
}

static void
_initial_color_rule_set(Ea_Theme_Color_Rule *rule)
{
   rule->id = NULL;
   rule->input = NULL;
   rule->h = rule->s = rule->v = 0;
   rule->a = 100;
   rule->h_max = -1;
   rule->s_max = rule->v_max = 100;
   rule->h_min = -1;
   rule->s_min = rule->v_min = 0;
   rule->h_fixed = rule->s_fixed = rule->v_fixed = EINA_FALSE;
}

static Eina_Bool
_color_attribute_parse_cb(void *data,
                          const char *key,
                          const char *value)
{
   Ea_Theme_Color_Rule *rule = (Ea_Theme_Color_Rule *)data;

   if (!rule) return EINA_FALSE;

   if (!strcmp(key, "id"))
     rule->id = eina_stringshare_add(value);
   else if (!strcmp(key, "inputColor"))
     rule->input = eina_stringshare_add(value);
   else if (!strcmp(key, "hue"))
     rule->h = atoi(value);
   else if (!strcmp(key, "saturation"))
     rule->s = atoi(value);
   else if (!strcmp(key, "value"))
     rule->v = atoi(value);
   else if (!strcmp(key, "alpha"))
     rule->a = atoi(value);
   else if (!strcmp(key, "fixedHue") && !strcmp(value, "true"))
     rule->h_fixed = EINA_TRUE;
   else if (!strcmp(key, "fixedSaturation") && !strcmp(value, "true"))
     rule->s_fixed = EINA_TRUE;
   else if (!strcmp(key, "fixedValue") && !strcmp(value, "true"))
     rule->v_fixed = EINA_TRUE;
   else if (!strcmp(key, "maxHue"))
     rule->h_max = atoi(value);
   else if (!strcmp(key, "minHue"))
     rule->h_min = atoi(value);
   else if (!strcmp(key, "maxSaturation"))
     rule->s_max = atoi(value);
   else if (!strcmp(key, "minSaturation"))
     rule->s_min = atoi(value);
   else if (!strcmp(key, "maxValue"))
     rule->v_max = atoi(value);
   else if (!strcmp(key, "minValue"))
     rule->v_min = atoi(value);

   return EINA_TRUE;
}

static Eina_Bool
_color_token_parse_cb(void *data,
                    Eina_Simple_XML_Type type,
                    const char *content,
                    unsigned offset EINA_UNUSED,
                    unsigned length)
{
   Eina_List **clist = (Eina_List **)data;
   static Eina_List **plist;
   static Ea_Theme_Color_Table_Item *cur_item;

   if (!clist) return EINA_FALSE;

   if (type == EINA_SIMPLE_XML_OPEN)
     {
        if (!strncmp("ChangeableColorTables", content, strlen("ChangeableColorTables")))
          {
             return EINA_TRUE;
          }
        else if (!strncmp("ChangeableColorTable", content, strlen("ChangeableColorTable")))
          {
             Ea_Theme_Color_Table_Item *item;

             item = calloc(1, sizeof(Ea_Theme_Color_Table_Item));
             if (!item) return EINA_FALSE;

             *clist = eina_list_append(*clist, item);
             cur_item = item;
          }
        else if (!strncmp("Theme", content, strlen("Theme")))
          {
             char *token;

             if (!cur_item) return EINA_FALSE;

             token = malloc(length + 1);
             if (!token) return EINA_FALSE;

             memcpy(token, content, length);
             token[length] = '\0';

             if(strstr(token, "Dark"))
               plist = &(cur_item->dark);
             else if (strstr(token, "Light"))
               plist = &(cur_item->light);
             else
               plist = NULL;

             free(token);
          }
     }
   else if (type == EINA_SIMPLE_XML_OPEN_EMPTY) /* <ChangeableColorInfo> */
     {
        if (!strncmp("ChangeableColorInfo ", content, strlen("ChangeableColorInfo ")))
          {
             Ea_Theme_Color_Rule *rule;
             const char *buf;

             rule = malloc(sizeof(Ea_Theme_Color_Rule));
             if (!rule) return EINA_FALSE;

             _initial_color_rule_set(rule);

             buf = eina_simple_xml_tag_attributes_find(content, length);
             eina_simple_xml_attributes_parse(buf,
                                              length - (buf - content),
                                              _color_attribute_parse_cb,
                                              rule);

             *plist = eina_list_append(*plist, rule);
          }
     }
   else if (type == EINA_SIMPLE_XML_CLOSE)
     {
        if (!strncmp("ChangeableColorTable", content, strlen("ChangeableColorTable")))
          cur_item = NULL;

        else if (!strncmp("Theme", content, strlen("Theme")))
          plist = NULL;
     }

   return EINA_TRUE;
}

static Eina_Bool
_rule_color_get(Eina_List *input_colors,
                Ea_Theme_Color_Rule *rule,
                int *r, int *g, int *b, int *a)
{
   float h, s, v;
   Ea_Theme_Color_hsv *input = NULL;
   Ea_Theme_Color_hsv white = {0, 0, 98};
   Ea_Theme_Color_hsv black = {0, 0, 3};
   Ea_Theme_Color_hsv dummy = {0, 0, 0};

   if (!rule) return EINA_FALSE;
   if (!input_colors) return EINA_FALSE;

   if (rule->input)
     {
        if (!strcmp(rule->input, "1"))
          input = eina_list_data_get(input_colors);
        else if (!strcmp(rule->input, "2"))
          input = eina_list_nth(input_colors, 1);
        else if (!strcmp(rule->input, "3"))
          input = eina_list_nth(input_colors, 2);
        else if (!strcmp(rule->input, "4"))
          input = eina_list_nth(input_colors, 3);
        else if (!strcmp(rule->input, "5"))
          input = eina_list_nth(input_colors, 4);
        else if (!strcmp(rule->input, "6"))
          input = eina_list_nth(input_colors, 5);
        else if (!strcmp(rule->input, "K") || !strcmp(rule->input, "k"))
          input = &black;
        else if (!strcmp(rule->input, "W") || !strcmp(rule->input, "w"))
          input = &white;
     }

   if (!input) input = &dummy;

   // get component's color
   h = (rule->h_fixed == EINA_TRUE)? (float)rule->h : ((float)input->h + (float)rule->h);
   s = (rule->s_fixed == EINA_TRUE)? (float)rule->s : ((float)input->s + (float)rule->s);
   v = (rule->v_fixed == EINA_TRUE)? (float)rule->v : ((float)input->v + (float)rule->v);

   // compare min/max value
   h = ((rule->h_max > 0) && (h > rule->h_max))? ((float)rule->h_max) :
                          (((rule->h_min > 0) && (h < rule->h_min))? (float)rule->h_min: h);
   s = (s > rule->s_max)? ((float)rule->s_max) :
                          ((s < rule->s_min)? (float)rule->s_min: s);
   v = (v > rule->v_max)? ((float)rule->v_max) :
                          ((v < rule->v_min)? (float)rule->v_min: v);

   // check the range
   h = (h > 360.0)? (h - 360.0) : ((h < 0.0)? (h + 360.0) : h);
   s = (s > 100.0)? (100.0) : ((s < 0.0)? (0.0) : s);
   v = (v > 100.0)? (100.0) : ((v < 0.0)? (0.0) : v);

   s /= 100.0;
   v /= 100.0;
   evas_color_hsv_to_rgb(h, s, v, r, g, b);

   *a = (int)(255 * rule->a / 100);
   *a = (*a > 255)? (255) : ((*a < 0)? 0 : *a);

   return EINA_TRUE;
}

static void
_color_overlays_set(Eina_List *list, Eina_List *input_colors, Eina_Bool config, Evas_Object *edje)
{
   Eina_List *l;
   Ea_Theme_Color_Rule *rule;
   int r, g, b, a;

   if (!input_colors) return;

   EINA_LIST_FOREACH(list, l, rule)
     {
        if(_rule_color_get(input_colors, rule, &r, &g, &b, &a))
          {
             if (config)
               elm_config_color_overlay_set(rule->id,
                                             r, g, b, a,
                                             255, 255, 255, 255,
                                             255, 255, 255, 255);
             else
               {
                  if (edje)
                    edje_object_color_class_set(edje, rule->id,
                                                r, g, b, a,
                                                255, 255, 255, 255,
                                                255, 255, 255, 255);
                  else
                    edje_color_class_set(rule->id,
                                         r, g, b, a,
                                         255, 255, 255, 255,
                                         255, 255, 255, 255);
               }
          }
     }
}

static Eina_Bool
_input_color_attribute_parse_cb(void *data,
                                const char *key,
                                const char *value)
{
   if (!data) return EINA_FALSE;

   if (!strcmp(key, "index"))
     {
        Ea_Theme_Input_Colors *colors = (Ea_Theme_Input_Colors *)data;

        colors->index = atoi(value);
     }
   else
     {
        Ea_Theme_Color_hsv *color = (Ea_Theme_Color_hsv *)data;

        if (!strcmp(key, "hue"))
          color->h = atoi(value);
        else if (!strcmp(key, "saturation"))
          color->s = atoi(value);
        else if (!strcmp(key, "value"))
          color->v = atoi(value);
     }

   return EINA_TRUE;
}

static Eina_Bool
_input_color_token_parse_cb(void *data,
                    Eina_Simple_XML_Type type,
                    const char *content,
                    unsigned offset EINA_UNUSED,
                    unsigned length)
{
   Eina_List **clist = (Eina_List **)data;
   static Eina_List **plist;

   if (!clist) return EINA_FALSE;

   if (type == EINA_SIMPLE_XML_OPEN) /* <Theme> */
     {
        if (!strncmp("Theme ", content, strlen("Theme ")))
          {
             Ea_Theme_Input_Colors *colors;
             const char *buf;

             colors = calloc(1, sizeof(Ea_Theme_Input_Colors));
             if (!colors) return EINA_FALSE;

             buf = eina_simple_xml_tag_attributes_find(content, length);
             eina_simple_xml_attributes_parse(buf,
                                              length - (buf - content),
                                              _input_color_attribute_parse_cb,
                                              colors);


             *clist = eina_list_append(*clist, colors);
             plist = &(colors->list);

          }
     }
   else if (type == EINA_SIMPLE_XML_OPEN_EMPTY) /* <InputColorInfo> */
     {
        if (!strncmp("InputColorInfo ", content, strlen("InputColorInfo ")))
          {
             Ea_Theme_Color_hsv *color;
             const char *buf;

             color = malloc(sizeof(Ea_Theme_Color_hsv));
             if (!color) return EINA_FALSE;

             buf = eina_simple_xml_tag_attributes_find(content, length);
             eina_simple_xml_attributes_parse(buf,
                                              length - (buf - content),
                                              _input_color_attribute_parse_cb,
                                              color);

             *plist = eina_list_append(*plist, color);
          }
     }
   else if (type == EINA_SIMPLE_XML_CLOSE) /* </Theme> */
     plist = NULL;

   return EINA_TRUE;
}

static void
_color_convert_rgb_to_XYZ(int r, int g, int b, double *X, double *Y, double *Z)
{
   double var_r = r / 255.0f;
   double var_g = g / 255.0f;
   double var_b = b / 255.0f;

   if (var_r > 0.04045f)
     var_r = pow(((var_r + 0.055f) / 1.055f), 2.4f);
   else
     var_r = var_r / 12.92f;

   if (var_g > 0.04045f)
     var_g = pow(((var_g + 0.055f) / 1.055f), 2.4f);
   else
     var_g = var_g / 12.92f;

   if (var_b > 0.04045f)
     var_b = pow(((var_b + 0.055f) / 1.055f), 2.4f);
   else
     var_b = var_b / 12.92f;

   var_r *= 100;
   var_g *= 100;
   var_b *= 100;

   *X = var_r * 0.4124f + var_g * 0.3576f + var_b * 0.1805f;
   *Y = var_r * 0.2126f + var_g * 0.7152f + var_b * 0.0722f;
   *Z = var_r * 0.0193f + var_g * 0.1192f + var_b * 0.9505f;
}

static void
_color_convert_XYZ_to_Lab(double X, double Y, double Z, double *L, double *a, double *b)
{
   double ref_X = 95.047f;
   double ref_Y = 100.000f;
   double ref_Z = 108.883f;

   double var_X = X / ref_X;
   double var_Y = Y / ref_Y;
   double var_Z = Z / ref_Z;

   if (var_X > 0.008856f)
     var_X = pow(var_X, 1.0f/3.0f);
   else
     var_X = ((903.3f * var_X) + 16.0f) / 116.0f;

   if (var_Y > 0.008856f)
     var_Y = pow(var_Y, 1.0f/3.0f);
   else
	 var_Y = ((903.3f * var_Y) + 16.0f) / 116.0f;

   if (var_Z > 0.008856f)
     var_Z = pow(var_Z, 1.0f/3.0f);
   else
     var_Z = ((903.3f * var_Z) + 16.0f) / 116.0f;

   *L = ( 116 * var_Y ) - 16;
   *a = 500 * ( var_X - var_Y );
   *b = 200 * ( var_Y - var_Z );
}

static void
_on_object_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ea_Theme_Object_Color *oc = (Ea_Theme_Object_Color *)data;

   DBG("obj: %x (%s), color code: %s",
       oc->obj, evas_object_type_get(oc->obj), oc->code);
   _object_colors = eina_list_remove(_object_colors, oc);
   if (oc->code) eina_stringshare_del(oc->code);
   if (oc->new_code) eina_stringshare_del(oc->new_code);
   free(oc);
}

void
_theme_colors_update(void *data)
{
   Ea_Theme_App_Color *ac;
   Ea_Theme_Object_Color *oc;
   Evas_Object *edje;
   int r, g, b, a;
   int r2, g2, b2, a2;
   int r3, g3, b3, a3;
   Eina_List *l;

   DBG("[color update] changeable state: %d, winset current style: %d",
       ea_theme_changeable_ui_enabled_get(), _cur_style);

   // winset colors and application color tables reload
   if (ea_theme_changeable_ui_enabled_get())
     ea_theme_style_set(_cur_style);

   EINA_LIST_FOREACH(_app_colors, l, ac)
     {
        if (ac->obj)
          {
             void *data;
             data = evas_object_data_get(ac->obj, "changeable_ui");
             DBG("obj: %x (%s), changeable state: %d",
                 ac->obj, evas_object_type_get(ac->obj), (int)data);

             if (data && ac->table)
               ea_theme_object_colors_set(ac->obj, ac->table, ac->style);
             else if (data)
               ea_theme_object_style_set(ac->obj, ac->style);
          }
        else if (ea_theme_changeable_ui_enabled_get())
          ea_theme_colors_set(ac->table, ac->style);
     }

   // object colors reload
   EINA_LIST_FOREACH(_object_colors, l, oc)
     {
        if (oc->new_code)
          {
             if (oc->obj) // ea_theme_object_color_replace
               edje = elm_layout_edje_get(oc->obj);
             else if (oc->item) // ea_theme_object_item_color_replace
               edje = elm_object_item_edje_get(oc->item);
             else
               edje = NULL;

             if (ea_theme_object_color_get(edje, oc->new_code,
                                           &r, &g, &b, &a,
                                           &r2, &g2, &b2, &a2,
                                           &r3, &g3, &b3, &a3))
               {
                  if (oc->obj)
                    INFO("obj: %x (%s), color code replace: (%s -> %s), new color [%d, %d, %d, %d]",
                         oc->obj, evas_object_type_get(oc->obj), oc->code, oc->new_code, r, g, b, a);
                  if (oc->item)
                    INFO("item: %x, color code replace: (%s -> %s), new color [%d, %d, %d, %d]",
                         oc->item, oc->code, oc->new_code, r, g, b, a);
                  edje_object_color_class_set(edje, oc->code,
                                              r, g, b, a,
                                              r2, g2, b2, a2,
                                              r3, b3, g3, a3);
               }
          }
        else if (oc->obj) // ea_theme_object_color_set
          {
             if (ea_theme_object_color_get(oc->obj, oc->code,
                                           &r, &g, &b, &a,
                                           NULL, NULL, NULL, NULL,
                                           NULL, NULL, NULL, NULL))
               {
                  INFO("obj: %x (%s), color code: %s, color [%d, %d, %d, %d]",
                       oc->obj, evas_object_type_get(oc->obj), oc->code, r, g, b, a);
                  evas_color_argb_premul(a, &r, &g, &b);
                  evas_object_color_set(oc->obj, r, g, b, a);
               }
          }
     }
}

Ea_Theme_Color_Table *
ea_theme_color_table_new(const char *file)
{
   Ea_Theme_Color_Table *table;

   if (!file) return NULL;

   table = _theme_xml_table_get(file, _color_token_parse_cb);
   INFO("color table (%x) from (%s) is created", table, file);

   return table;
}

void
ea_theme_color_table_free(Ea_Theme_Color_Table *table)
{
   Ea_Theme_Color_Rule *rule;
   Ea_Theme_Color_Table_Item *item;
   Ea_Theme_App_Color *ac;
   Eina_List *l;

   if (!table) return;

   INFO("color table (%x) is freed", table);

   // remove from data
   EINA_LIST_REVERSE_FOREACH(_app_colors, l, ac)
     {
        if (ac->table == table)
          break;
     }
   if (ac)
     {
        _app_colors = eina_list_remove_list(_app_colors, l);
        free(ac);
     }

   EINA_LIST_FREE(table, item)
     {
        EINA_LIST_FREE(item->dark, rule)
          {
             eina_stringshare_del(rule->id);
             eina_stringshare_del(rule->input);
             free(rule);
          }
        EINA_LIST_FREE(item->light, rule)
          {
             eina_stringshare_del(rule->id);
             eina_stringshare_del(rule->input);
             free(rule);
          }
        free(item);
     }
}

Eina_Bool
ea_theme_colors_set(Ea_Theme_Color_Table *table, Ea_Theme_Style style)
{
   Eina_List *theme_list = NULL, *l;
   Ea_Theme_Color_Config *config;
   Ea_Theme_Color_Table_Item *item = NULL;
   Ea_Theme_App_Color *ac;

   if (!table) return EINA_FALSE;

   DBG("changeable state: %d,  color table (%x) is set with style (%d)",
       ea_theme_changeable_ui_enabled_get(), table, style);

   // get config file
   if (!ea_theme_changeable_ui_enabled_get())
     config = _color_config_get(EINA_FALSE);
   else
     config = _color_config_get(EINA_TRUE);
   if (!config)
     {
        ERR("cannot get color config file");
        return EINA_FALSE;
     }

   // read table
   if (config->index >= EA_THEME_COLOR_TABLE_3)
     item = eina_list_nth(table, 2);
   else if (config->index >= EA_THEME_COLOR_TABLE_2)
     item = eina_list_nth(table, 1);
   else
     item = eina_list_data_get(table);

   if (!item)
     item = eina_list_data_get(table);
   if (!item)
     {
        ERR("cannot get color table data");
        _color_config_free(config);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_DARK)
     theme_list = item->dark;
   else if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     {
        if (!strcmp(config->style, "Dark"))
          theme_list = item->dark;
        else if (!strcmp(config->style, "Light"))
          theme_list = item->light;
     }
   if (!theme_list)
     {
        ERR("cannot get color table data");
        _color_config_free(config);
        return EINA_FALSE;
     }

   _color_overlays_set(theme_list, config->list, EINA_FALSE, NULL);
   _color_config_free(config);

   // add to data
   EINA_LIST_FOREACH(_app_colors, l, ac)
     {
        if (ac->table == table && ac->style == style)
          break;
     }
   if (!ac)
     {
        ac = calloc(1, sizeof(Ea_Theme_App_Color));
        if (ac)
          {
             ac->table = table;
             ac->style = style;
             _app_colors = eina_list_append(_app_colors, ac);
          }
     }

   return EINA_TRUE;
}

Eina_Bool
ea_theme_colors_unset(Ea_Theme_Color_Table *table, Ea_Theme_Style style)
{
   Ea_Theme_Color_Config *config;
   Ea_Theme_Color_Table_Item *item = NULL;
   Eina_List *theme_list = NULL;
   Ea_Theme_Color_Rule *rule;
   Eina_List *l;

   if (!table) return EINA_FALSE;

   config = _color_config_get(EINA_TRUE);
   if (!config) return EINA_FALSE;

   if (config->index >= EA_THEME_COLOR_TABLE_3)
     item = eina_list_nth(table, 2);
   else if (config->index >= EA_THEME_COLOR_TABLE_2)
     item = eina_list_nth(table, 1);
   else
     item = eina_list_data_get(table);
   if (!item)
     {
        _color_config_free(config);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     theme_list = item->dark;
   if (!theme_list)
     {
        _color_config_free(config);
        return EINA_FALSE;
     }

   EINA_LIST_FOREACH(theme_list, l, rule)
      edje_color_class_del(rule->id);

   _color_config_free(config);

   return EINA_TRUE;
}

Eina_Bool
ea_theme_color_get(const char *code,
                   int *r, int *g, int *b, int *a,
                   int *r2, int *g2, int *b2, int *a2,
                   int *r3, int *g3, int *b3, int *a3)
{
   if (!code) return EINA_FALSE;

   if (edje_color_class_get(code,
                            r, g, b, a,
                            r2, g2, b2, a2,
                            r3, g3, b3, a3))
     return EINA_TRUE;

   return EINA_FALSE;
}

Eina_Bool
ea_theme_style_set(Ea_Theme_Style style)
{
   Ea_Theme_Color_Table *table;
   char buf[BUF_SIZE];
   Ea_Theme_Color_Config *config;
   Eina_List *theme_list = NULL;
   Ea_Theme_Color_Table_Item *item;

   DBG("changeable state: %d,  winset style (%d)",
       ea_theme_changeable_ui_enabled_get(), style);

   // get config file
   if (!ea_theme_changeable_ui_enabled_get())
     config = _color_config_get(EINA_FALSE);
   else
     config = _color_config_get(EINA_TRUE);
   if (!config)
     {
        ERR("cannot get color config file");
        return EINA_FALSE;
     }

   // read table
   if (config->index >= EA_THEME_COLOR_TABLE_3)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_3);
   else if (config->index >= EA_THEME_COLOR_TABLE_2)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_2);
   else
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_1);

   table = ea_theme_color_table_new(buf);
   if (!table)
     {
        ERR("cannot get winset color table");
        _color_config_free(config);
        return EINA_FALSE;
     }

   item = eina_list_data_get(table);
   if (!item)
     {
        ERR("cannot get winset color table data");
        ea_theme_color_table_free(table);
        _color_config_free(config);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_DARK)
     theme_list = item->dark;
   else if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     {
        if (!strcmp(config->style, "Dark"))
          theme_list = item->dark;
        else if (!strcmp(config->style, "Light"))
          theme_list = item->light;
     }
   if (!theme_list)
     {
        ERR("cannot get winset color table data");
        _color_config_free(config);
        ea_theme_color_table_free(table);
        return EINA_FALSE;
     }

   _color_overlays_set(theme_list, config->list, EINA_FALSE, NULL);
   _cur_style = style;

   if (!ea_theme_changeable_ui_enabled_get() &&
       style != EA_THEME_STYLE_DEFAULT)
     _theme_changeable_ui_data_set(EINA_FALSE);

   ea_theme_color_table_free(table);
   _color_config_free(config);

   return EINA_TRUE;
}

Ea_Theme_Style
ea_theme_style_get(void)
{
   DBG("current winset style: %d", _cur_style);

   if (_cur_style == EA_THEME_STYLE_DEFAULT)
     {
        Ea_Theme_Color_Config *config;
        Ea_Theme_Style style = -1;

        config = _color_config_get(EINA_TRUE);

        if (config && !strcmp(config->style, "Dark"))
          style = EA_THEME_STYLE_DARK;
        else if (config && !strcmp(config->style, "Light"))
          style = EA_THEME_STYLE_LIGHT;

        _color_config_free(config);

        return style;
     }

   return _cur_style;
}

Eina_Bool
ea_theme_fixed_style_set(int num, Ea_Theme_Style style)
{
   Ea_Theme_Color_Table *table;
   Ea_Theme_Color_hsv *color;
   Eina_List *input_colors, *theme_list = NULL;
   Ea_Theme_Color_Table_Item *item;
   char buf[BUF_SIZE];

   if (num >= EA_THEME_COLOR_TABLE_3)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_3);
   else if (num >= EA_THEME_COLOR_TABLE_2)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_2);
   else
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_1);

   table = ea_theme_color_table_new(buf);
   if (!table) return EINA_FALSE;

   item = eina_list_data_get(table);
   if (!item)
     {
        ea_theme_color_table_free(table);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     theme_list = item->dark;
   if (!theme_list)
     {
        ea_theme_color_table_free(table);
        return EINA_FALSE;
     }

   input_colors = ea_theme_input_colors_get(num);
   _color_overlays_set(theme_list, input_colors, EINA_FALSE, NULL);

   ea_theme_color_table_free(table);
   EINA_LIST_FREE(input_colors, color)
      free(color);

   return EINA_TRUE;
}

Eina_Bool
ea_theme_object_colors_set(Evas_Object *obj, Ea_Theme_Color_Table *table, Ea_Theme_Style style)
{
   Eina_List *theme_list = NULL, *l;
   Ea_Theme_Color_Table_Item *item = NULL;
   Ea_Theme_Color_Config *config;
   Ea_Theme_App_Color *ac;
   Evas_Object *edje;
   void *data = NULL;

   if (!table) return EINA_FALSE;

   if (!obj || !evas_object_smart_type_check(obj, "elm_layout"))
     return EINA_FALSE;

   edje = elm_layout_edje_get(obj);
   if (!edje) return EINA_FALSE;

   DBG("changeable state: %d, table: %x, obj: %x (%s), theme style: %d",
       (int)evas_object_data_get(obj, "changeable_ui"),
       table, obj, evas_object_type_get(obj), style);

   // get config file
   data = evas_object_data_get(obj, "changeable_ui");
   if (!data)
     config = _color_config_get(EINA_FALSE);
   else
     config = _color_config_get(EINA_TRUE);
   if (!config)
     {
        ERR("cannot get color config file");
        return EINA_FALSE;
     }

   // read table
   if (config->index >= EA_THEME_COLOR_TABLE_3)
     item = eina_list_nth(table, 2);
   else if (config->index >= EA_THEME_COLOR_TABLE_2)
     item = eina_list_nth(table, 1);
   else
     item = eina_list_data_get(table);

   if (!item)
     item = eina_list_data_get(table);
   if (!item)
     {
        ERR("cannot get color table data");
        _color_config_free(config);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_DARK)
     theme_list = item->dark;
   else if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     {
        if (!strcmp(config->style, "Dark"))
          theme_list = item->dark;
        else if (!strcmp(config->style, "Light"))
          theme_list = item->light;
     }
   if (!theme_list)
     {
        ERR("cannot get color table data");
        return EINA_FALSE;
     }

   _color_overlays_set(theme_list, config->list, EINA_FALSE, edje);
   evas_object_data_set(obj, "color_overlay", edje);
   evas_object_data_set(edje, "color_overlay", edje);

   _color_config_free(config);

   // add to data
   EINA_LIST_FOREACH(_app_colors, l, ac)
     {
        if (ac->obj == obj && ac->table == table && ac->style == style)
          break;
     }
   if (!ac)
     {
        ac = calloc(1, sizeof(Ea_Theme_App_Color));
        if (ac)
          {
             ac->obj = obj;
             ac->table = table;
             ac->style = style;
             _app_colors = eina_list_append(_app_colors, ac);
          }
     }

   return EINA_TRUE;
}

Eina_Bool
ea_theme_object_style_set(Evas_Object *obj, Ea_Theme_Style style)
{
   Ea_Theme_Color_Table *table;
   Ea_Theme_Color_Table_Item *item;
   Ea_Theme_App_Color *ac;
   char buf[BUF_SIZE];
   Ea_Theme_Color_Config *config;
   Eina_List *theme_list = NULL, *l;
   Evas_Object *edje;
   void *data = NULL;

   if (!obj || !evas_object_smart_type_check(obj, "elm_layout"))
     return EINA_FALSE;

   edje = elm_layout_edje_get(obj);
   if (!edje) return EINA_FALSE;

   DBG("changeable state: %d, obj: %x (%s), theme style: %d",
       (int)evas_object_data_get(obj, "changeable_ui"),
       obj, evas_object_type_get(obj), style);

   // get config file
   data = evas_object_data_get(obj, "changeable_ui");
   if (!data)
     config = _color_config_get(EINA_FALSE);
   else
     config = _color_config_get(EINA_TRUE);
   if (!config)
     {
        ERR("cannot get color config file");
        return EINA_FALSE;
     }

   // read table
   if (config->index >= EA_THEME_COLOR_TABLE_3)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_3);
   else if (config->index >= EA_THEME_COLOR_TABLE_2)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_2);
   else
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_1);

   table = ea_theme_color_table_new(buf);
   if (!table)
     {
        ERR("cannot get winset color table");
        _color_config_free(config);
        return EINA_FALSE;
     }

   item = eina_list_data_get(table);
   if (!item)
     {
        ERR("cannot get winset color table data");
        ea_theme_color_table_free(table);
        _color_config_free(config);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_DARK)
     theme_list = item->dark;
   else if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     {
        if (!strcmp(config->style, "Dark"))
          theme_list = item->dark;
        else if (!strcmp(config->style, "Light"))
          theme_list = item->light;
     }
   if (!theme_list)
     {
        ERR("cannot get winset color table data");
        _color_config_free(config);
        ea_theme_color_table_free(table);
        return EINA_FALSE;
     }

   _color_overlays_set(theme_list, config->list, EINA_FALSE, edje);
   evas_object_data_set(obj, "color_overlay", edje);
   evas_object_data_set(edje, "color_overlay", edje);

   ea_theme_color_table_free(table);
   _color_config_free(config);

   // add to data
   EINA_LIST_FOREACH(_app_colors, l, ac)
     {
        if (ac->obj == obj && ac->style == style)
          break;
     }
   if (!ac)
     {
        ac = calloc(1, sizeof(Ea_Theme_App_Color));
        if (ac)
          {
             ac->obj = obj;
             ac->style = style;
             _app_colors = eina_list_append(_app_colors, ac);
          }
     }

   return EINA_TRUE;
}

Eina_Bool
ea_theme_object_fixed_style_set(Evas_Object *obj, int num, Ea_Theme_Style style)
{
   Ea_Theme_Color_Table *table;
   Ea_Theme_Color_hsv *color;
   char buf[BUF_SIZE];
   Eina_List *input_colors, *theme_list;
   Ea_Theme_Color_Table_Item *item;
   Evas_Object *edje;

   if (!obj || !evas_object_smart_type_check(obj, "elm_layout"))
     return EINA_FALSE;

   edje = elm_layout_edje_get(obj);
   if (!edje) return EINA_FALSE;

   if (num >= EA_THEME_COLOR_TABLE_3)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_3);
   else if (num >= EA_THEME_COLOR_TABLE_2)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_2);
   else
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_1);

   table = ea_theme_color_table_new(buf);
   if (!table) return EINA_FALSE;

   item = eina_list_data_get(table);
   if (!item)
     {
        ea_theme_color_table_free(table);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_LIGHT)
     theme_list = item->light;
   else
     theme_list = item->dark;
   if (!theme_list)
     {
        ea_theme_color_table_free(table);
        return EINA_FALSE;
     }

   input_colors = ea_theme_input_colors_get(num);
   _color_overlays_set(theme_list, input_colors, EINA_FALSE, edje);
   evas_object_data_set(obj, "color_overlay", edje);
   evas_object_data_set(edje, "color_overlay", edje);

   ea_theme_color_table_free(table);
   EINA_LIST_FREE(input_colors, color)
      free(color);

   return EINA_TRUE;
}

Eina_Bool
ea_theme_object_color_set(Evas_Object *obj, const char *code)
{
   int r, g, b, a;

   if (!obj) return EINA_FALSE;
   if (!code) return EINA_FALSE;

   if (ea_theme_object_color_get(obj, code,
                                 &r, &g, &b, &a,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL))
     {
        Ea_Theme_Object_Color *oc;

        evas_color_argb_premul(a, &r, &g, &b);
        evas_object_color_set(obj, r, g, b, a);
        INFO("obj: %x (%s), color code: %s, color [%d, %d, %d, %d]",
            obj, evas_object_type_get(obj), code, r, g, b, a);

        oc = calloc(1, sizeof(Ea_Theme_Object_Color));
        if (oc)
          {
             oc->obj = obj;
             oc->code = eina_stringshare_add(code);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                            _on_object_del, oc);

             _object_colors = eina_list_append(_object_colors, oc);
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

Eina_Bool
ea_theme_object_color_replace(Evas_Object *obj, const char *code, const char *new_code)
{
   Evas_Object *edje;
   int r, g, b, a;
   int r2, g2, b2, a2;
   int  r3, g3, b3, a3;

   if (!obj || !evas_object_smart_type_check(obj, "elm_layout"))
     EINA_FALSE;

   if (!code) return EINA_FALSE;
   if (!new_code) return EINA_FALSE;

   edje = elm_layout_edje_get(obj);
   if (!edje) return EINA_FALSE;

   if (ea_theme_object_color_get(obj, new_code,
                                 &r, &g, &b, &a,
                                 &r2, &g2, &b2, &a2,
                                 &r3, &g3, &b3, &a3))
     {
        Ea_Theme_Object_Color *oc;

        edje_object_color_class_set(edje, code,
                                    r, g, b, a,
                                    r2, g2, b2, a2,
                                    r3, b3, g3, a3);

        INFO("obj: %x (%s), color code replace: (%s -> %s), new color [%d, %d, %d, %d]",
            obj, evas_object_type_get(obj), code, new_code, r, g, b, a);

        oc = calloc(1, sizeof(Ea_Theme_Object_Color));
        if (oc)
          {
             oc->obj = obj;
             oc->code = eina_stringshare_add(code);
             oc->new_code = eina_stringshare_add(new_code);
             evas_object_event_callback_add(edje, EVAS_CALLBACK_DEL,
                                            _on_object_del, oc);

             _object_colors = eina_list_append(_object_colors, oc);
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

Eina_Bool
ea_theme_object_item_color_replace(Elm_Object_Item *item, const char *code, const char *new_code)
{
   Evas_Object *edje;
   int r, g, b, a;
   int r2, g2, b2, a2;
   int  r3, g3, b3, a3;

   if (!item) return EINA_FALSE;
   if (!code) return EINA_FALSE;
   if (!new_code) return EINA_FALSE;

   edje = elm_object_item_edje_get(item);
   if (!edje) return EINA_FALSE;

   if (ea_theme_object_color_get(edje, new_code,
                                 &r, &g, &b, &a,
                                 &r2, &g2, &b2, &a2,
                                 &r3, &g3, &b3, &a3))
     {
        Ea_Theme_Object_Color *oc;

        edje_object_color_class_set(edje, code,
                                    r, g, b, a,
                                    r2, g2, b2, a2,
                                    r3, b3, g3, a3);

        INFO("item: %x, color code replace: (%s -> %s), new color [%d, %d, %d, %d]",
            item, code, new_code, r, g, b, a);

        oc = calloc(1, sizeof(Ea_Theme_Object_Color));
        if (oc)
          {
             oc->item = item;
             oc->code = eina_stringshare_add(code);
             oc->new_code = eina_stringshare_add(new_code);
             evas_object_event_callback_add(edje, EVAS_CALLBACK_DEL,
                                            _on_object_del, oc);

             _object_colors = eina_list_append(_object_colors, oc);
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

void
ea_theme_object_colors_reload(void)
{
   Eina_List *l;
   Ea_Theme_Object_Color *oc;
   Evas_Object *edje;
   int r, g, b, a;
   int r2, g2, b2, a2;
   int r3, g3, b3, a3;

   EINA_LIST_FOREACH(_object_colors, l, oc)
     {
        if (oc->new_code)
          {
             if (oc->obj) // ea_theme_object_color_replace
               edje = elm_layout_edje_get(oc->obj);
             else if (oc->item) // ea_theme_object_item_color_replace
               edje = elm_object_item_edje_get(oc->item);
             else
               edje = NULL;

             if (ea_theme_object_color_get(edje, oc->new_code,
                                           &r, &g, &b, &a,
                                           &r2, &g2, &b2, &a2,
                                           &r3, &g3, &b3, &a3))
               {
                  edje_object_color_class_set(edje, oc->code,
                                              r, g, b, a,
                                              r2, g2, b2, a2,
                                              r3, b3, g3, a3);
               }
          }
        else if (oc->obj) // ea_theme_object_color_set
          {
             if (ea_theme_object_color_get(oc->obj, oc->code,
                                           &r, &g, &b, &a,
                                           NULL, NULL, NULL, NULL,
                                           NULL, NULL, NULL, NULL))
               {
                  evas_color_argb_premul(a, &r, &g, &b);
                  evas_object_color_set(oc->obj, r, g, b, a);
               }
          }
     }
}

Eina_Bool
ea_theme_object_color_get(Evas_Object *obj, const char *code,
                          int *r, int *g, int *b, int *a,
                          int *r2, int *g2, int *b2, int *a2,
                          int *r3, int *g3, int *b3, int *a3)
{
   Evas_Object *edje = NULL;

   if (!obj) return EINA_FALSE;
   if (!code) return EINA_FALSE;

   // get color from object level
   if (evas_object_type_get(obj) &&
       !strcmp(evas_object_type_get(obj), "edje"))
     edje = obj;
   else if (evas_object_smart_type_check(obj, "elm_layout"))
     edje = elm_layout_edje_get(obj);
   if (edje)
     {
        if (edje_object_color_class_get(edje, code,
                                        r, g, b, a,
                                        r2, g2, b2, a2,
                                        r3, g3, b3, a3))
          return EINA_TRUE;
     }

   // get color from parent level ex) elm_image (not layout object)
   edje = evas_object_data_get(obj, "color_overlay");
   if (edje)
     {
        if (edje_object_color_class_get(edje, code,
                                        r, g, b, a,
                                        r2, g2, b2, a2,
                                        r3, g3, b3, a3))
          return EINA_TRUE;
     }

   // get color from global level
   if (ea_theme_color_get(code,
                          r, g, b, a,
                          r2, g2, b2, a2,
                          r3, g3, b3, a3))
     return EINA_TRUE;

   return EINA_FALSE;
}

void
ea_theme_system_colors_apply(void)
{
#if 0
   Ea_Theme_Color_Table *table;
   Ea_Theme_Color_Table_Item *item;
   Ea_Theme_Color_Config *config;
   char buf[BUF_SIZE];

   config = _color_config_get();
   if (!config) return;

   if (config->index >= EA_THEME_COLOR_TABLE_3)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_3);
   else if (config->index >= EA_THEME_COLOR_TABLE_2)
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_2);
   else
     snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _color_table_1);

   table = ea_theme_color_table_new(buf);
   if (!table)
     {
        _color_config_free(config);
        return;
     }

   item = eina_list_nth(table, 0);
   if (!item)
     {
        ea_theme_color_table_free(table);
        _color_config_free(config);
        return;
     }

   _color_overlays_set(item->dark, config->list, EINA_TRUE, NULL);
   elm_config_all_flush();
   elm_config_save();

   ea_theme_color_table_free(table);
   _color_config_free(config);
#endif
}

Eina_Bool
ea_theme_input_colors_set(int num, Ea_Theme_Style style)
{
   Ea_Theme_Color_Config *config;
   Eina_Bool ret;

   DBG("color index: %d, theme style: %d", num, style);

   config = calloc(1, sizeof(Ea_Theme_Color_Config));
   if (!config) return EINA_FALSE;

   config->index = num;
   config->list = ea_theme_input_colors_get(num);
   if (!config->list)
     {
        ERR("cannot get input color for index [%d]", num);
        free(config);
        return EINA_FALSE;
     }

   if (style == EA_THEME_STYLE_DARK)
     config->style = eina_stringshare_add("Dark");
   else if (style == EA_THEME_STYLE_LIGHT)
     config->style = eina_stringshare_add("Light");
   else
     {
        Ea_Theme_Color_Config *sconfig;

        sconfig = _color_config_get(EINA_FALSE);
        if (!sconfig)
          {
             _color_config_free(config);
             return EINA_FALSE;
          }

        if (!strcmp(sconfig->style, "Dark"))
          config->style = eina_stringshare_add("Dark");
        else if (!strcmp(sconfig->style, "Light"))
          config->style = eina_stringshare_add("Light");

        _color_config_free(sconfig);
     }

   ret = _color_config_set(config);
   _color_config_free(config);

   if (!ret)
     {
        ERR("cannot set color config file. Theme is not set!!");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_List *
ea_theme_input_colors_get(int num)
{
   Eina_List *table, *l, *ret = NULL;
   Ea_Theme_Input_Colors *colors = NULL;
   Ea_Theme_Color_hsv *color, *new_color;
   char buf[BUF_SIZE];

   // get input color table
   snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _input_color_table);
   table = _theme_xml_table_get(buf, _input_color_token_parse_cb);
   if (!table) return NULL;

   // get input colors
   EINA_LIST_FOREACH(table, l, colors)
     {
        if (colors->index == num)
          break;
        colors = NULL;
     }
   if (!colors)
     {
        EINA_LIST_FREE(table, colors)
          {
             EINA_LIST_FREE(colors->list, color)
                free(color);
             free(colors);
          }
        return NULL;
     }

   // add colors to the new list
   EINA_LIST_FOREACH(colors->list, l, color)
     {
        new_color = malloc(sizeof(Ea_Theme_Color_hsv));
        if (!new_color) continue;

        new_color->h = color->h;
        new_color->s = color->s;
        new_color->v = color->v;

        ret = eina_list_append(ret, new_color);
     }

   // free table
   EINA_LIST_FREE(table, colors)
     {
        EINA_LIST_FREE(colors->list, color)
           free(color);
        free(colors);
     }

   return ret;
}

Eina_Bool
ea_theme_system_color_set(Ea_Theme_Color_hsv color, Ea_Theme_Style style)
{
   Ea_Theme_Color_Config *config;
   Ea_Theme_Color_hsv *scolor;
   Eina_Bool ret;

   DBG("color h: %d, s: %d, v: %d, style: %d", color.h, color.s, color.v, style);

   config = calloc(1, sizeof(Ea_Theme_Color_Config));
   if (!config) return EINA_FALSE;

   scolor = malloc(sizeof(Ea_Theme_Color_hsv));
   if (!scolor)
     {
        free(config);
        return EINA_FALSE;
     }

   scolor->h = color.h;
   scolor->s = color.s;
   scolor->v = color.v;

   config->index = -1;
   config->list = eina_list_append(config->list, scolor);

   if (style == EA_THEME_STYLE_DARK)
     config->style = eina_stringshare_add("Dark");
   else if (style == EA_THEME_STYLE_LIGHT)
     config->style = eina_stringshare_add("Light");
   else
     {
        Ea_Theme_Color_Config *sconfig;

        sconfig = _color_config_get(EINA_FALSE);
        if (!sconfig)
          {
             _color_config_free(config);
             return EINA_FALSE;
          }

        if (!strcmp(sconfig->style, "Dark"))
          config->style = eina_stringshare_add("Dark");
        else if (!strcmp(sconfig->style, "Light"))
          config->style = eina_stringshare_add("Light");

        _color_config_free(sconfig);
     }

   ret = _color_config_set(config);
   _color_config_free(config);

   if (!ret)
     {
        ERR("cannot set color config file. Theme is not set!!");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
ea_theme_system_color_get(Ea_Theme_Color_hsv *color)
{
   Ea_Theme_Color_Config *config;
   Ea_Theme_Color_hsv *scolor;

   if (!color) return EINA_FALSE;

   config = _color_config_get(EINA_TRUE);
   if (!config) return EINA_FALSE;

   scolor = eina_list_data_get(config->list);
   if (!scolor)
     {
        _color_config_free(config);
        return EINA_FALSE;
     }

   color->h = scolor->h;
   color->s = scolor->s;
   color->v = scolor->v;

   _color_config_free(config);

   return EINA_TRUE;
}

int
ea_theme_suitable_theme_get(int r, int g, int b)
{
   Eina_List *table;
   Ea_Theme_Input_Colors *colors;
   double minDistance, curDistance;
   double origin_X, origin_Y, origin_Z, origin_L, origin_a, origin_b;
   double compare_X, compare_Y, compare_Z, compare_L, compare_a, compare_b;
   int c_r, c_g, c_b, i, num;
   Ea_Theme_Color_hsv *color;
   char buf[BUF_SIZE];

   // get input color table
   snprintf(buf, BUF_SIZE, "%s/%s", _data_path, _input_color_table);
   table = _theme_xml_table_get(buf, _input_color_token_parse_cb);
   if (!table) return EINA_FALSE;

   colors = eina_list_data_get(table);
   color = eina_list_data_get(colors->list);
   evas_color_hsv_to_rgb(color->h, color->s/100.0f, color->v/100.0f, &c_r, &c_g, &c_b);

   //Using CIELab color space
   _color_convert_rgb_to_XYZ(r, g, b, &origin_X, &origin_Y, &origin_Z);
   _color_convert_XYZ_to_Lab(origin_X, origin_Y, origin_Z, &origin_L, &origin_a, &origin_b);

   _color_convert_rgb_to_XYZ(c_r, c_g, c_b, &compare_X, &compare_Y, &compare_Z);
   _color_convert_XYZ_to_Lab(compare_X, compare_Y, compare_Z, &compare_L, &compare_a, &compare_b);

   minDistance = sqrt(pow(compare_L-origin_L,2) + pow(compare_a-origin_a,2) + pow(compare_b-origin_b,2));

   num = colors->index;
   for (i = 1; i < eina_list_count(table); i++)
     {
        colors = eina_list_nth(table, i);
        color = eina_list_data_get(colors->list);
        evas_color_hsv_to_rgb(color->h, color->s/100.0f, color->v/100.0f, &c_r, &c_g, &c_b);

        _color_convert_rgb_to_XYZ(c_r, c_g, c_b, &compare_X, &compare_Y, &compare_Z);
        _color_convert_XYZ_to_Lab(compare_X, compare_Y, compare_Z, &compare_L, &compare_a, &compare_b);

        curDistance = sqrt(pow(compare_L-origin_L,2) + pow(compare_a-origin_a,2) + pow(compare_b-origin_b,2));
        if (minDistance > curDistance)
          {
             minDistance = curDistance;
             num = colors->index;
          }
     }

   // free table
   EINA_LIST_FREE(table, colors)
     {
        EINA_LIST_FREE(colors->list, color)
           free(color);
        free(colors);
     }

   return num;
}

int
ea_theme_suitable_theme_get_from_image(const char *path)
{
  int theme_index;
  int* pcolor = ea_collect_color_set_image(path, 1);
  int r, g, b;

  if (pcolor == NULL)
    return -1;

  r = (*pcolor & 0x00FF0000) >> 16;
  g = (*pcolor & 0x0000FF00) >> 8;
  b = *pcolor & 0x000000FF;

  theme_index = ea_theme_suitable_theme_get(r, g, b);
  free(pcolor);

  return theme_index;
}
