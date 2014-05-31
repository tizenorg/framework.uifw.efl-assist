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

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "efl-assist"
#define __CONSTRUCTOR__ __attribute__ ((constructor))
#define __DESTRUCTOR__ __attribute__ ((destructor))

/* eina magic types */
#define EA_MAGIC_NONE 0x87657890
#define EA_MAGIC_CUTLINK 0x78908765

typedef unsigned int ea_magic;
#define EA_MAGIC                ea_magic __magic

#define EA_MAGIC_SET(d, m)      (d)->__magic = (m)
#define EA_MAGIC_CHECK(d, m)    ((d) && ((d)->__magic == (m)))
#define EA_MAGIC_FAIL(d, m, fn) \
		_ea_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

/* String for efl-assist */
# define ASSIST_STR_UNAVAILABLE_TEXT    dgettext(PACKAGE, "IDS_SCR_SBODY_SCREEN_READER_IS_NOT_SUPPORTED_BY_THIS_APPLICATION_PRESS_THE_HOME_OR_BACK_KEY_TO_RETURN_TO_THE_HOME_SCREEN")

void _ea_magic_fail(const void *d, ea_magic m,
		    ea_magic req_m, const char *fname);

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_PRIVATE_H__ */
