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

#ifndef __EFL_ASSIST_POPUP_H__
#define __EFL_ASSIST_POPUP_H__

#include <Elementary.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


EAPI Evas_Object *               ea_center_popup_add(Evas_Object *parent);

EAPI Evas_Object *               ea_center_popup_win_get(Evas_Object *popup);

EAPI Evas_Object *               ea_menu_popup_add(Evas_Object *parent);

EAPI Evas_Object *               ea_menu_popup_win_get(Evas_Object *obj);

EAPI void                       ea_menu_popup_move(Evas_Object *obj);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_POPUP_H__ */
