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

#ifndef __EFL_ASSIST_PRIVATE_H__
#define __EFL_ASSIST_PRIVATE_H__

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @internal
 *
 * @ {
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "EFL-ASSIST"
#define __CONSTRUCTOR__ __attribute__ ((constructor))
#define __DESTRUCTOR__ __attribute__ ((destructor))

/* eina magic types */
#define EA_MAGIC_NONE 0x87657890

typedef unsigned int ea_magic;
#define EA_MAGIC                ea_magic __magic

#define EA_MAGIC_SET(d, m)      (d)->__magic = (m)
#define EA_MAGIC_CHECK(d, m)    ((d) && ((d)->__magic == (m)))
#define EA_MAGIC_FAIL(d, m, fn) \
		_ea_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

#define DBG(fmt , args...) \
        do { \
                LOGD(""fmt"", ##args); \
        } while (0)

#define INFO(fmt , args...) \
        do { \
                LOGI(""fmt"", ##args); \
        } while (0)

#define WARN(fmt , args...) \
        do { \
                LOGW(""fmt"", ##args); \
        } while (0)

#define ERR(fmt , args...) \
        do { \
                LOGE(""fmt"", ##args); \
        } while (0)

/* String for efl-assist */
# define ASSIST_STR_UNAVAILABLE_TEXT    dgettext(PACKAGE, "IDS_SCR_SBODY_SCREEN_READER_IS_NOT_SUPPORTED_BY_THIS_APPLICATION_PRESS_THE_HOME_OR_BACK_KEY_TO_RETURN_TO_THE_HOME_SCREEN")

void _ea_magic_fail(const void *d, ea_magic m,
		    ea_magic req_m, const char *fname);

/* private functions and data for ea_theme */
#define BUF_SIZE 256

Eina_List      *_theme_xml_table_get(const char *file, Eina_Simple_XML_Cb func);
void            _theme_changeable_ui_data_set(Eina_Bool enabled);
void            _theme_color_monitor_add(void);
void            _theme_font_monitor_add(void);
char           *_theme_system_config_path_get(void);
char           *_theme_user_config_path_get(void);
void            _theme_colors_update(void *data);
void            _theme_fonts_update(void *data);

/**
 * @brief collect specified count highest frequency colors of specified image file.
 * @param path [IN] path of image file.
 * @param resultCount [IN] color count need to collect.
 * @return rgb of colors, need to free by caller.
 */
int* ea_collect_color_set_image(const char *path, int resultCount);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_PRIVATE_H__ */
