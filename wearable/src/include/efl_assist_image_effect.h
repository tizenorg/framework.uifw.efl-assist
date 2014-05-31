/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef __EFL_ASSIST_IMAGE_EFFECT_H__
#define __EFL_ASSIST_IMAGE_EFFECT_H__

#include <Elementary.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ea_effect_s ea_effect_h;

ea_effect_h *ea_image_effect_create(void);

void ea_image_effect_destroy(ea_effect_h *effect);

void ea_image_effect_add_outer_shadow(ea_effect_h *effect,
                    float angle,
                    float offset,
                    float softness,
                    unsigned int color);

void ea_image_effect_offset_get(ea_effect_h *effect, int *x, int *y);

void ea_object_image_effect_set(Evas_Object *obj, ea_effect_h *effect);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_IMAGE_EFFECT_H__ */
