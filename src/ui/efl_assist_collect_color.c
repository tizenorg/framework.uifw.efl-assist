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

#include <glib.h>
#include <image_util.h>
#include <stdlib.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include "efl_assist_private.h"

#define EA_COLOR_MASK (0xFFE0E0E0)
#define EA_TARGET_SIZE (60)

typedef struct
{
        const char *path;
        int result_count;
        int find_result_index;
        int width;
        int height;
        unsigned char * img_target;
        GHashTable *hash_table;
        GList *node_list;

} CollectColor_t;

typedef struct
{
        int key;
        int value;
}CollectColorNode_t;

static void _ea_collect_color_swap(CollectColorNode_t *node_1, CollectColorNode_t *node_2)
{
        int temp;

        temp = node_1->key;
        node_1->key = node_2->key;
        node_2->key = temp;

        temp = node_1->value;
        node_1->value = node_2->value;
        node_2->value = temp;
}

static bool _ea_collect_color_sift_down(GList *node_list, int start, int end)
{
        int root = start;
        int child;
        int swap;

        while(root * 2 + 1 <= end)
	  {
                child = root * 2 + 1;
                swap = root;
                CollectColorNode_t *node_1 = g_list_nth_data(node_list, swap);
                CollectColorNode_t *node_2 = g_list_nth_data(node_list, child);
                if(node_1->value > node_2->value)
		  {
                        swap = child;
                  }
                CollectColorNode_t *node_3 = g_list_nth_data(node_list, swap);
                CollectColorNode_t *node_4 = g_list_nth_data(node_list, child+1);
                if(child+1 <= end && node_3->value > node_4->value)
		  {
                        swap = child + 1;
                  }
                if(swap != root)
		  {
                        CollectColorNode_t *node_5 = g_list_nth_data(node_list, swap);
                        CollectColorNode_t *node_6 = g_list_nth_data(node_list, root);
                        _ea_collect_color_swap(node_5, node_6);

                        root = swap;
                  }
                else
                        return true;
          }
        return false;
}

static void _ea_collect_color_sort(GList *node_list)
{
        int length = g_list_length(node_list);
        int end = length - 1;
        while(end > 0)
	  {
                _ea_collect_color_swap((CollectColorNode_t *)(g_list_nth_data(node_list, 0)), (CollectColorNode_t *)(g_list_nth_data(node_list, end)));
                end--;
                _ea_collect_color_sift_down(node_list, 0, end);
          }
}

static void _ea_collect_color_make_tree(GList *node_list)
{
       int length = g_list_length(node_list);
       int start = (length - 2) / 2;
        while(start >= 0)
	  {
                _ea_collect_color_sift_down(node_list, start, length - 1);
                start--;
          }
}

static void _ea_collect_color_check_tree(GList *node_list)
{
       _ea_collect_color_sift_down(node_list, 0, g_list_length(node_list)-1);
}

static int* _ea_collect_color_get_RGB(int* color, int length)
{
        int* result = (int *)calloc(length*3, sizeof(int));
	if (result == NULL)
	  return NULL;

	int index = 0;
        int i = 0;
        for(; i<length; i++)
	  {
                result[index++] = (color[i] & 0x00FF0000) >> 16;
                result[index++] = (color[i] & 0x0000FF00) >> 8;
                result[index++] = color[i] & 0x000000FF;
          }
        return result;
}

static int _ea_collect_color_get_int(int r, int g, int b)
{
        int result = (0xFF000000) | (r << 16) | (g << 8) | b;
        return result;
}


static int _ea_collect_color_get_sim_average(int color)
{
        int multi = 8;
        int r = (color & 0x00FF0000) >> 16;
        int g = (color & 0x0000FF00) >> 8;
        int b = color & 0x000000FF;
        int average = (r + g + b) / 3 / 2 / multi;

        if(rand() % 2 == true)
                r = r + (rand() % average) * multi;
        else
                r = r - (rand() % average) * multi;
        if(r > 255)
                r = 255;
        else if(r < 0)
                r = 0;

        if(rand() % 2 == true)
                g = g + (rand() % average) * multi;
        else
                g = g - (rand() % average) * multi;
        if(g > 255)
                g = 255;
        else if(g < 0)
                g = 0;

        if(rand() % 2 == true)
                b = b + (rand() % average) * multi;
        else
                b = b - (rand() % average) * multi;
        if(b > 255)
                b = 255;
        else if(b < 0)
                b = 0;
        int result = _ea_collect_color_get_int(r, g, b);
        return result;
}

static int* _ea_collect_color_get_result(CollectColor_t *collect_color)
{
        GList *node_list = collect_color->node_list;

        int* result = (int *)calloc(collect_color->result_count, sizeof(int));
	if (result == NULL)
	  return NULL;

	int i = 0;
        for(; i<collect_color->result_count; i++)
	  {
                if(collect_color->find_result_index <= i) {
                        result[i] = _ea_collect_color_get_sim_average(result[0]);
                        continue;
                }
                CollectColorNode_t *node = g_list_nth_data(node_list, i);
                result[i] = node->key;
          }
        return result;
}

static bool _ea_collect_color_insert(int key, int value, GList *node_list)
{
        CollectColorNode_t *node = g_list_nth_data(node_list, 0);
        if(node->value < value)
	  {
                node->key = key;
                node->value = value;
                _ea_collect_color_check_tree(node_list);
                return true;
          }
        return false;
}

static GList* _ea_collect_color_copy_to_array(CollectColor_t *collect_color)
{
        GList *node_list = NULL;
        int nodes_index = 0;
        int temp_key;
        int temp_val;
        bool nodesIsFull = false;
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init (&iter, collect_color->hash_table);
        while (g_hash_table_iter_next (&iter, &key, &value))
          {
                temp_key = *((int*)key);
                temp_val = *((int*)value);
                if(nodesIsFull)
		  {
                        _ea_collect_color_insert(temp_key, temp_val, node_list);
                  }
                  else
		    {
                        CollectColorNode_t *node = (CollectColorNode_t *)calloc(1, sizeof(CollectColorNode_t));
                        node->key = temp_key;
                        node->value = temp_val;
                        node_list = g_list_append(node_list, node);
                        nodes_index++;
                        if(nodes_index == collect_color->result_count)
			  {
                                nodesIsFull = true;
                                _ea_collect_color_make_tree(node_list);
                          }
                    }
          }

        collect_color->find_result_index = nodes_index;
        _ea_collect_color_make_tree(node_list);
        return node_list;
}
static void _ea_collect_color_add_hash(GHashTable *hash_table, int pixel)
{
        pixel &= EA_COLOR_MASK;

        int *ret = (int *)g_hash_table_lookup(hash_table, &pixel);
        gint *key = g_new(gint, 1);
        *key = pixel;
        gint *value = g_new(gint, 1);
        if (ret)
	  {
                *value = *ret+1;
                g_hash_table_replace(hash_table, key, value);
          }
        else
	  {
                *value = 1;
                g_hash_table_insert(hash_table, key, value);
          }
}

static void _ea_collect_color_run_detail(CollectColor_t *collect_color)
{
        int key = 0;
        int i = 0;
        int j = 0;
        int src_idx = 0;
        unsigned char *src = (unsigned char *)collect_color->img_target;
        for(i = 0; i<collect_color->height; i++)
	  {
                for(j = 0; j<collect_color->width; j++)
		  {
                        src_idx = (i*collect_color->width+j)*4;
                        key = _ea_collect_color_get_int(*(src+src_idx+2), *(src+src_idx+1), *(src+src_idx));
                        _ea_collect_color_add_hash(collect_color->hash_table, key);
                  }
          }
}

static void _ea_collect_color_decode_image(CollectColor_t *collect_color)
{
        int width = 0;
        int height = 0;
        int resized_width = EA_TARGET_SIZE;
        int resized_height = EA_TARGET_SIZE;
        int ret = 0;
        unsigned int size_decode = 0;
        unsigned char *img_source = 0;
        const image_util_colorspace_e colorspace = IMAGE_UTIL_COLORSPACE_BGRA8888;
        Ecore_Evas *ee;
        Evas* evas;
        Evas_Object *obj_img;

        ee = ecore_evas_buffer_new(720, 1280);
        evas = ecore_evas_get(ee);
        obj_img = evas_object_image_add(evas);
        evas_object_image_file_set(obj_img, collect_color->path, NULL);
        evas_object_image_colorspace_set(obj_img, EVAS_COLORSPACE_ARGB8888);
        evas_object_image_size_get(obj_img, &width, &height);
        evas_object_image_fill_set(obj_img,0,0,width,height);
        ecore_evas_show(ee);
        img_source = (unsigned char *)evas_object_image_data_get(obj_img, EINA_TRUE);

        image_util_calculate_buffer_size(width, height, colorspace, &size_decode);
        collect_color->img_target = malloc( size_decode );

        ret = image_util_resize( collect_color->img_target, &resized_width, &resized_height, img_source, width, height, colorspace );
        if (ret) //resize fail
        {
            collect_color->img_target = img_source;
            collect_color->width = width;
            collect_color->height = height;
            evas_object_del(obj_img);
            ecore_evas_free(ee);
        }
        else //resize success
        {
            evas_object_del(obj_img);
            ecore_evas_free(ee);
            collect_color->width = resized_width;
            collect_color->height = resized_height;
        }
}

void _ea_collect_color_table_value_destory(gpointer data)
{
        free(data);
}

void _ea_collect_color_table_key_destory(gpointer data)
{
        free(data);
}

static GHashTable* _ea_collect_color_table_init()
{
        GHashTable *hash_table = NULL;

	hash_table = g_hash_table_new_full(g_int_hash, g_int_equal, _ea_collect_color_table_key_destory, _ea_collect_color_table_value_destory);
        return hash_table;
}

static CollectColor_t * _ea_collect_color_create()
{
        CollectColor_t *collect_color = (CollectColor_t *)calloc(1, sizeof(CollectColor_t));

        collect_color->hash_table = _ea_collect_color_table_init();
        g_hash_table_remove_all(collect_color->hash_table);

        return collect_color;
}

static void _ea_collect_color_list_free_cb(gpointer data)
{
        CollectColorNode_t *node = (CollectColorNode_t *)data;

        free(node);
}

static void _ea_collect_color_destroy(CollectColor_t *collect_color)
{
       if (collect_color->hash_table)
         {
                g_hash_table_remove_all(collect_color->hash_table);
                g_hash_table_destroy(collect_color->hash_table);
          }
        free(collect_color->img_target);

        if (collect_color->node_list)
	  {
                g_list_free_full(collect_color->node_list, _ea_collect_color_list_free_cb);
          }
        free(collect_color);
}

int* ea_collect_color_set_image(const char *path, int resultCount)
{
        if (resultCount < 1)
                return NULL;

	CollectColor_t *collect_color = _ea_collect_color_create();

        collect_color->find_result_index = 0;
        collect_color->path = path;
        collect_color->result_count = resultCount;

        _ea_collect_color_decode_image(collect_color);

	if (collect_color->img_target == NULL)
	  {
	    free(collect_color);
	    return NULL;
	  }

	_ea_collect_color_run_detail(collect_color);

	collect_color->node_list = _ea_collect_color_copy_to_array(collect_color);
        _ea_collect_color_sort(collect_color->node_list);

        int *colors = _ea_collect_color_get_result(collect_color);
        _ea_collect_color_destroy(collect_color);
        return colors;
}

