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
#include <cairo.h>
#include <math.h>
#include <image_util.h>

typedef enum {
    EA_EFFECT_TYPE_NONE = 0,
    EA_EFFECT_TYPE_OUTER_SHADOW,
//  EA_EFFECT_TYPE_INNER_SHADOW,
//  EA_EFFECT_TYPE_LINEAR_GRADIENT,
//  EA_EFFECT_TYPE_STROKE,
//  EA_EFFECT_TYPE_OUTER_GLOW,
} ea_effect_type_e;

typedef struct ea_effect_data_s ea_effect_data_h;
struct ea_effect_data_s
{
   int type;
   union
     {
        struct
          {
             float angle;
             float offset;
             float softness;
             unsigned int color;

             float x_offset;
             float y_offset;
          } outer_shadow;
     };
};

struct ea_effect_s
{
   Eina_List *effect_list;
};

static ea_effect_data_h *
_ea_image_effect_outer_shadow_new (float angle, float offset, float softness, unsigned int color)
{
   ea_effect_data_h *effect_data;

   effect_data = calloc (1, sizeof (ea_effect_data_h));
   if (!effect_data)
     {
        LOGE("Outer shadow effect creation failed\n");
        return NULL;
     }

   effect_data->type = EA_EFFECT_TYPE_OUTER_SHADOW;
   effect_data->outer_shadow.angle = angle;
   effect_data->outer_shadow.offset = offset;
   effect_data->outer_shadow.softness = softness;
   effect_data->outer_shadow.color = color;

   effect_data->outer_shadow.x_offset = effect_data->outer_shadow.offset * cos(effect_data->outer_shadow.angle * M_PI / 180.0f);
   effect_data->outer_shadow.y_offset = -effect_data->outer_shadow.offset * sin(effect_data->outer_shadow.angle * M_PI / 180.0f);

   return effect_data;
}

static void
_ea_image_effect_outer_shadow_apply (ea_effect_data_h *effect_data,
                    cairo_surface_t *src_surface,
                    cairo_surface_t *dst_surface)
{
   double x_translate, y_translate;
   double red, green, blue, alpha;

   if (!src_surface)
     {
        LOGE("Invalid source surface\n");
        return;
     }

   if (!dst_surface)
     {
        LOGE("Invalid destination surface\n");
        return;
     }

   int src_surface_width = cairo_image_surface_get_width (src_surface);
   int src_surface_height = cairo_image_surface_get_height (src_surface);

   alpha = ((effect_data->outer_shadow.color & 0xFF000000) >> 24) / 255.0f;
   red = ((effect_data->outer_shadow.color & 0xFF0000) >> 16) / 255.0f;
   green = ((effect_data->outer_shadow.color & 0xFF00) >> 8) / 255.0f;
   blue = (effect_data->outer_shadow.color & 0xFF) / 255.0f;

   x_translate = effect_data->outer_shadow.softness;
   if (effect_data->outer_shadow.x_offset < 0)
      x_translate -= effect_data->outer_shadow.x_offset;
   y_translate = effect_data->outer_shadow.softness;
   if (effect_data->outer_shadow.y_offset < 0)
      y_translate -= effect_data->outer_shadow.y_offset;

   cairo_t *cr = cairo_create (dst_surface);
   cairo_save (cr);
   cairo_set_source_rgba (cr, 0, 0, 0, 0);
   cairo_paint (cr);
   cairo_translate (cr, (int)x_translate, (int)y_translate);
   cairo_set_source_surface (cr, src_surface, 0, 0);
   cairo_set_shadow (cr, CAIRO_SHADOW_DROP);
   cairo_set_shadow_offset (cr, effect_data->outer_shadow.x_offset, effect_data->outer_shadow.y_offset);
   cairo_set_shadow_rgba (cr, red, green, blue, alpha);
   cairo_set_shadow_blur (cr, effect_data->outer_shadow.softness, effect_data->outer_shadow.softness);
   cairo_rectangle (cr, 0, 0, src_surface_width, src_surface_height);
   cairo_fill (cr);
   cairo_restore (cr);
   cairo_destroy (cr);
}

static void
_ea_image_effect_data_free (void *data)
{
   ea_effect_data_h *effect_data = (ea_effect_data_h *)data;

   if (effect_data) free (effect_data);
}

EXPORT_API ea_effect_h *
ea_image_effect_create(void)
{
   ea_effect_h *effect;

   effect = calloc (1, sizeof (ea_effect_h));
   if (!effect)
     {
        LOGE("Image effect creation failed\n");
        return NULL;
     }

   return effect;
}

EXPORT_API void
ea_image_effect_destroy(ea_effect_h *effect)
{
   ea_effect_data_h *effect_data;

   if (!effect)
     {
        LOGE("Invalid effect object\n");
        return;
     }

   EINA_LIST_FREE(effect->effect_list, effect_data)
     {
        _ea_image_effect_data_free(effect_data);
     }

   free (effect);
}

EXPORT_API void
ea_image_effect_add_outer_shadow(ea_effect_h *effect,
                    float angle,
                    float offset,
                    float softness,
                    unsigned int color)
{
   ea_effect_data_h *effect_data;

   if (!effect)
     {
        LOGE("Invalid effect object\n");
        return;
     }

   effect_data = _ea_image_effect_outer_shadow_new (angle, offset, softness, color);
   if (!effect_data)
     {
        LOGE("Invalid effect data\n");
        return;
     }

   effect->effect_list = eina_list_append (effect->effect_list, effect_data);
}

EXPORT_API void
ea_image_effect_offset_get(ea_effect_h *effect, int *x, int *y)
{
   ea_effect_data_h *effect_data;
   Eina_List *l;
   int x_translate = 0;
   int y_translate = 0;

   if (!effect)
     {
        LOGE("Invalid effect object\n");
        return;
     }

   EINA_LIST_FOREACH(effect->effect_list, l, effect_data)
     {
        if (effect_data->type == EA_EFFECT_TYPE_OUTER_SHADOW)
          {
             x_translate = effect_data->outer_shadow.softness;
             if (effect_data->outer_shadow.x_offset < 0)
                x_translate -= effect_data->outer_shadow.x_offset;

             y_translate = effect_data->outer_shadow.softness;
             if (effect_data->outer_shadow.y_offset < 0)
                y_translate -= effect_data->outer_shadow.y_offset;
          }
     }

   *x = x_translate;
   *y = y_translate;
}

EXPORT_API void
ea_object_image_effect_set(Evas_Object *obj, ea_effect_h *effect)
{
   ea_effect_data_h *effect_data = NULL;
   Eina_List *l;
   unsigned char *data = NULL;
   int width, height;
   int new_width, new_height;
   cairo_surface_t *src_surface = NULL, *dst_surface = NULL;
   const char *obj_type = NULL;

   if (!obj)
     {
        LOGE("Invalid evas object\n");
        return;
     }

   if (!effect)
     {
        LOGE("Invalid effect object\n");
        return;
     }

   obj_type = evas_object_type_get (obj);
   if (strcmp (obj_type, "image"))
     {
        LOGE("Image effect can only be set to IMAGE type\n");
        return;
     }

   data = (unsigned char *)evas_object_image_data_get (obj, EINA_FALSE);
   if (!data)
     {
        LOGE("evas_object_image_data_get failed\n");
        return;
     }

   evas_object_image_size_get (obj, &width, &height);
   new_width = width;
   new_height = height;

   src_surface = cairo_image_surface_create_for_data (data,
                                                      CAIRO_FORMAT_ARGB32,
                                                      width,
                                                      height,
                                                      cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width));
   if ((src_surface == NULL) ||
       (cairo_surface_status (src_surface) != CAIRO_STATUS_SUCCESS))
     {
        LOGE("cairo source surface creation failed\n");
        goto finish;
     }

   EINA_LIST_FOREACH(effect->effect_list, l, effect_data)
     {
        if (effect_data->type == EA_EFFECT_TYPE_OUTER_SHADOW)
          {
             new_width = width + abs(effect_data->outer_shadow.x_offset) + effect_data->outer_shadow.softness * 2;
             new_height = height + abs(effect_data->outer_shadow.y_offset) + effect_data->outer_shadow.softness * 2;

             dst_surface = cairo_surface_create_similar (src_surface, CAIRO_CONTENT_COLOR_ALPHA, new_width, new_height);
             if ((dst_surface == NULL) ||
                 (cairo_surface_status (dst_surface) != CAIRO_STATUS_SUCCESS))
               {
                  LOGE("cairo destination surface creation failed\n");
                  goto finish;
               }

             _ea_image_effect_outer_shadow_apply (effect_data, src_surface, dst_surface);
          }
        else
          {
             // Other types not supported yet
             LOGE("Unsupported effect type: %d\n", effect_data->type);
             goto finish;
          }
     }

   evas_object_resize (obj, new_width, new_height);
   evas_object_image_fill_set (obj, 0, 0, new_width, new_height);
   evas_object_image_size_set (obj, new_width, new_height);
   evas_object_image_data_copy_set (obj, (void *)cairo_image_surface_get_data (dst_surface));

finish:
   if (dst_surface) cairo_surface_destroy (dst_surface);
   if (src_surface) cairo_surface_destroy (src_surface);
}

EXPORT_API void *
ea_image_effect_mask(const char *image, const char *mask, int w, int h, int mask_x, int mask_y)
{
   int img_width, img_height;
   int ret = 0;
   unsigned char *img_source = NULL;
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *obj_img;
   void *result = NULL, *resized_img = NULL;
   int resized_width = w, resized_height = h;
   cairo_t *cr = NULL;
   cairo_surface_t *dst_surface = NULL, *img_surface = NULL, *mask_surface = NULL;

   ee = ecore_evas_buffer_new (w, h);
   evas = ecore_evas_get (ee);
   obj_img = evas_object_image_add (evas);
   evas_object_image_file_set (obj_img, image, NULL);
   evas_object_image_colorspace_set (obj_img, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_size_get (obj_img, &img_width, &img_height);
   evas_object_image_fill_set (obj_img, 0, 0, img_width, img_height);
   ecore_evas_show (ee);

   img_source = (unsigned char *)evas_object_image_data_get (obj_img, EINA_TRUE);

   if ((w != img_width)  || (h != img_height))
     {
        resized_img = malloc (w * h * 4);
        if (resized_img == NULL)
          {
             LOGE("memory alloc for resized image failed\n");
             goto error;
          }

        ret = image_util_resize(resized_img, &resized_width, &resized_height, img_source, img_width, img_height, IMAGE_UTIL_COLORSPACE_BGRA8888);
        if (ret) //resize fail
          {
             LOGE("image resize failed\n");
             goto error;
          }
        else //resize success
          {
             img_source = resized_img;
          }
     }

   dst_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, resized_width, resized_height);
   if ((dst_surface == NULL) ||
       (cairo_surface_status (dst_surface) != CAIRO_STATUS_SUCCESS))
     {
        LOGE("cairo dst surface creation failed\n");
        goto error;
     }

   cr = cairo_create (dst_surface);

   img_surface = cairo_image_surface_create_for_data (img_source, CAIRO_FORMAT_ARGB32,
      resized_width, resized_height, cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, resized_width));
   if ((img_surface == NULL) ||
       (cairo_surface_status (dst_surface) != CAIRO_STATUS_SUCCESS))
     {
        LOGE("cairo img surface creation failed\n");
        goto error;
     }

   cairo_set_source_surface(cr, img_surface, 0, 0);

   mask_surface = cairo_image_surface_create_from_png (mask);
   if ((mask_surface == NULL) ||
       (cairo_surface_status (mask_surface) != CAIRO_STATUS_SUCCESS))
     {
        LOGE("cairo mask surface creation failed\n");
        goto error;
     }

   cairo_mask_surface (cr, mask_surface, mask_x, mask_y);
   cairo_fill (cr);

   result = malloc(resized_width * resized_height* 4);
   if (result == NULL)
     {
         LOGE("memory alloc failed\n");
         goto error;
     }

   memcpy(result, cairo_image_surface_get_data(dst_surface), resized_width * resized_height * 4);

   cairo_surface_destroy(dst_surface);
   cairo_surface_destroy(img_surface);
   if (resized_img) free(resized_img);
   cairo_surface_destroy(mask_surface);
   cairo_destroy (cr);

   evas_object_del(obj_img);
   ecore_evas_free(ee);

   return result;

error:

   if (resized_img) free(resized_img);

   if (dst_surface) cairo_surface_destroy(dst_surface);
   if (img_surface) cairo_surface_destroy(img_surface);
   if (mask_surface) cairo_surface_destroy(mask_surface);

   if (cr) cairo_destroy (cr);

   evas_object_del(obj_img);
   ecore_evas_free(ee);

   return NULL;
}
