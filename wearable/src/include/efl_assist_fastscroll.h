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

#ifndef __EFL_ASSIST_FASTSCROLL_H__
#define __EFL_ASSIST_FASTSCROLL_H__

#include <Elementary.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get if the locale is latin or not
 *
 * @details This function returns if the locale is latin language or not.
 *
 * @param [in] the locale
 *
 * @return EINA_TRUE if the locale is latin, otherwise EINA_FALSE.
 *
 */
EAPI Eina_Bool ea_locale_latin_get(const char *locale);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_FASTSCROLL_H__ */
