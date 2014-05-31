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

#include <regex.h>
#include <ctype.h>
#include <alloca.h>
#include <string.h>
#include <time.h>
#include "efl_assist.h"
#include "efl_assist_private.h"

#define MAX_DATETIME_NUM 6
#define DEF_STRING_LEN_MAX 256
#define DEF_AHREF_PREFIX "<a href=%s%s>"
#define DEF_AHREF_POSTFIX "</a>"
#define EA_CUTLINK_SCHEME_MARKUP "markup"

/*===========================================================================*
 *                                 Local                                     *
 *===========================================================================*/

typedef struct ea_cutlink_scheme_s *ea_cutlink_scheme_h;
typedef struct ea_cutlink_def_scheme_s *ea_cutlink_def_scheme_h;
typedef void (*ea_cutlink_anchor_cb)(ea_cutlink_h cutlink,
				     ea_cutlink_scheme_h scheme,
				     char *matched);
typedef bool (*ea_cutlink_def_match_cb)(const ea_cutlink_h cutlink,
				    const char *text,
				    int *start_offset,
				    int *end_offset,
				    ea_cutlink_scheme_h scheme);

struct ea_cutlink_def_scheme_s {
	ea_cutlink_mask_e mask;
	char *name;
	char *pattern;
	ea_cutlink_anchor_cb afunc;
	ea_cutlink_match_cb mfunc;
	ea_cutlink_trans_cb tfunc;
};

struct ea_cutlink_scheme_s {
	ea_cutlink_mask_e mask;
	char *name;
	char *pattern;
	ea_cutlink_anchor_cb afunc;
	ea_cutlink_match_cb def_mfunc;
	ea_cutlink_match_cb user_mfunc;
	ea_cutlink_trans_cb def_tfunc;
	ea_cutlink_trans_cb user_tfunc;
	void *match_data;
	void *trans_data;
	bool anchor_enable;
	char *prefix;
	char *postfix;
	bool excepted;
};

struct ea_cutlink_s {
	EINA_INLIST;
	EA_MAGIC;

	Evas_Object *entry;
	char *txt;

	Eina_List *sc_list;
	Eina_Strbuf *tagged_txt;

	char *file;
	Elm_Text_Format format;

	Eina_List *cc_list;
};

struct ea_match_info_s {
	ea_cutlink_scheme_h sc;
	int so;
	int eo;
};

static bool _match_phone_cb(ea_cutlink_h cutlink, const char *text,
			    int *start_offset, int *end_offset,
			    const char *scheme, void *userdata);
static bool _match_datetime_cb(ea_cutlink_h cutlink, const char *text,
			       int *start_offset, int *end_offset,
			       const char *scheme, void *userdata);

static void _trans_datetime_cb(const ea_cutlink_h cutlink,
			       char **match_str, char **scheme,
			       void *userdata);

static const int country_code[] = {
	  1,  20, 212, 213, 216, 218, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
	234, 235, 236, 237, 238, 239, 240, 241, 242, 243,
	244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
	254, 255, 256, 257, 258, 260, 261, 262, 263, 264,
	265, 266, 267, 268, 269,  27, 290, 291, 297, 298,
	299,  30,  31,  32,  33,  34, 350, 351, 352, 353,
	354, 355, 356, 357, 358, 359,  36, 370, 371, 372,
	373, 374, 375, 376, 377, 378, 380, 381, 382, 385,
	386, 387, 389,  39,  40,  41, 420, 421, 423,  43,
	 44,  45,  46,  47,  48,  49, 500, 501, 502, 503,
	504, 505, 506, 507, 508, 509,  51,  52,  53,  54,
	 55,  56,  57,  58, 590, 591, 592, 593, 594, 595,
	596, 597, 598, 599,  60,  61,  62,  63,  64,  65,
	 66, 670, 672, 673, 674, 675, 676, 677, 678, 679,
	680, 681, 682, 683, 685, 686, 687, 688, 689, 690,
	691, 692,   7,  81,  82,  84, 850, 852, 853, 855,
	856,  86, 880, 886,  90,  91,  92,  93,  94,  95,
	961, 962, 963, 964, 965, 966, 967, 968, 970, 971,
	972, 973, 974, 975, 976, 977,  98, 992, 993, 994,
	995, 996, 998,   0
};

static const struct ea_cutlink_def_scheme_s def_scheme[] = {
	{
		EA_CUTLINK_ALL,
		EA_CUTLINK_SCHEME_MARKUP,
		"<[^<>]*>",
		NULL,
		NULL,
		NULL
	},
	{
		EA_CUTLINK_PHONE,
		EA_CUTLINK_SCHEME_PHONE,
		"\\+?[0-9]{,3}\\(?[0-9]{1,3}\\)?([-\\+ /]|[0-9]){2,}\\(?[0-9]{1,3}\\)?",
		NULL,
		_match_phone_cb,
		NULL
	},
	{
		EA_CUTLINK_EMAIL,
		EA_CUTLINK_SCHEME_EMAIL,
		"\\b\\w+([-+.']\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*",
		NULL,
		NULL,
		NULL
	},
	{
		EA_CUTLINK_DATETIME,
		EA_CUTLINK_SCHEME_DATETIME,
		"(\\b(((([0-9]{2})?[0-9]{2}[-/\\.])?([0-9]{1,2}[-/\\.])([0-9]{1,2})([-/\\.]([0-9]{2})?[0-9]{2})?)|([0-9]{1,2}((:[0-9]{1,2})|( ?[ap]m))+))+"
		"( [0-9]{1,2}((:[0-9]{1,2})|( ?[ap]m))+)?"
		"( (([0-9]{2})?[0-9]{2}[-/\\.])?([0-9]{1,2}[-/\\.])([0-9]{1,2})([-/\\.]([0-9]{2})?[0-9]{2})?)?)|"
		"(\\b([0-9]{2}(st|nd|rd|th)? )?"
		"([0-9]{4} )?"
		"([0-9]{1,2}((:[0-9]{1,2})|( ?[ap]m))+ )?"
		"((Jan(uary)?|Feb(ruary)?|Mar(ch)?|Apr(il)?|May|Jun(e)?|Jul(y)?|Aug(ust)?|Sep(tember)?|Oct(ober)?|Nov(ember)?|Dec(ember)?|Tonight|Today|Tomorrow|Monday|Tuesday|Wednesday|Thurthday|Friday|Saturday|Sunday|morning|afternoon|night|evening)\\.?([ ,](morning|afternoon|night|evening))?)"
		"( [0-9]{2}(st|nd|rd|th)?)?"
		"( [0-9]{4})?"
		"( [0-9]{1,2}((:[0-9]{1,2})|( ?[ap]m))+)?)",
		NULL,
		_match_datetime_cb,
		_trans_datetime_cb
	},
	{
		EA_CUTLINK_UNC,
		EA_CUTLINK_SCHEME_UNC,
		"\\b\\\\{2}[0-9a-zA-Z_\\.]+",
		NULL,
		NULL,
		NULL
	},
	{
		EA_CUTLINK_URI,
		EA_CUTLINK_SCHEME_URI,
		"(\\b(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[1-9])(\\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])){3})|"
		"(\\b(rtsp|mailto|http(s)?|www|wap|mms|file|ftp|geo):(//)?[0-9a-z_\\./]+@?[0-9a-z_\\.,/\\?&\\(\\)=;\\-]+)|"
		"(\\b(www|wap)\\.[0-9a-z_\\.,/\\?&\\(\\)=;\\-]+)[0-9a-z]|"
		"(\\b[0-9a-z][0-9a-z_\\.,/\\?&\\(\\)=;]+(\\.com|\\.net|\\.org|\\.pl))",
		NULL,
		NULL,
		NULL
	},
};

static int
_country_code_comp_func(const void *data1, const void *data2)
{
	int v1, v2;

	v1 = (int)(data1);
	v2 = (int)(data2);

	if (v1 < v2)
		return -1;
	else if (v1 == v2)
		return 0;

	return 1;
}

static ea_cutlink_scheme_h
_ea_cutlink_scheme_new(unsigned char mask, char *name, char *pattern,
		       ea_cutlink_anchor_cb afunc,
		       ea_cutlink_match_cb mfunc,
		       ea_cutlink_trans_cb tfunc)
{
	ea_cutlink_scheme_h sc;

	if (!name || !pattern)
		return NULL;

	sc = calloc(1, sizeof(struct ea_cutlink_scheme_s));
	if (!sc)
		return NULL;

	sc->mask = mask;
	sc->name = strdup(name);
	sc->pattern = strdup(pattern);
	sc->afunc = afunc;
	sc->def_mfunc = mfunc;
	sc->def_tfunc = tfunc;
	sc->user_mfunc = NULL;
	sc->user_tfunc = NULL;
	sc->match_data = NULL;
	sc->trans_data = NULL;
	sc->anchor_enable = true;
	sc->prefix = NULL;
	sc->postfix = NULL;
	sc->excepted = false;

	LOGD("new scheme (%p)", sc);

	return sc;
}

static void
_ea_cutlink_scheme_free(void *data)
{
	ea_cutlink_scheme_h sc = (ea_cutlink_scheme_h)data;

	if (!sc)
		return;

	LOGD("free scheme(%p)", sc);

	sc->mask = EA_CUTLINK_NONE;
	if (sc->name)
		free(sc->name);
	if (sc->pattern)
		free(sc->pattern);
	sc->afunc = NULL;
	sc->def_mfunc = NULL;
	sc->def_tfunc = NULL;
	sc->user_mfunc = NULL;
	sc->user_tfunc = NULL;
	sc->match_data = NULL;
	sc->trans_data = NULL;
	sc->anchor_enable = false;
	if (sc->prefix)
		free(sc->prefix);
	if (sc->postfix)
		free(sc->postfix);
	sc->excepted = false;

	free(sc);
}

static bool
_check_country_code(ea_cutlink_h cutlink, int ccode)
{
	void *result = NULL;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK, "_check_country_code");
		return false;
	}

	result = eina_list_search_sorted(cutlink->cc_list,
				_country_code_comp_func, (void *)ccode);
	if (!result)
		return false;

	return true;
}

static bool
_match_phone_cb(ea_cutlink_h cutlink, const char *text,
		int *so, int *eo, const char *scheme, void *userdata)
{
	char *ss;
	char *str;
	int i;
	int cnt;
	int ps, pe;
	int inum = 0;
	bool chk_inum = false;
	bool skip_pt = false;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK, "_match_phone_cb");
		return false;
	}

	if (!text || !so || !eo || !scheme) {
		LOGE("invalid argument");
		return false;
	}

	str = alloca(*eo - *so + 1);
	if (!str) {
		LOGE("fail to allocate memory");
		return false;
	}
	snprintf(str, *eo - *so + 1, "%.*s", *eo - *so, text + *so);

	LOGD("_match_phone_cb str(%s), so(%d), eo(%d)", str, *so, *eo);

	i = *so;
	cnt = 0;
	ps = pe = 0;
	chk_inum = false;
	inum = 0;
	skip_pt = false;
	while (i < *eo) {
		switch (text[i]) {
		case '/':
		case '-':
		case ' ':
			/* continous separator */
			if (i+1 < *eo &&
				(text[i+1] == '-' ||
				 text[i+1] == ' ' ||
				 text[i+1] == '/'))
					return false;
			/* continous one number */
			if (i+2 < *eo &&
				(text[i+2] == '-' ||
				 text[i+2] == ' ' ||
				 text[i+2] == '/'))
					return false;
			break;
		case '(':
			/* more than one parenthesis */
			if (ps)
				return false;

			ss = strchr(text+i+1, ')');
			if (!ss || (ss && ss >= text+(*eo))) {
				*so = i+1;
				cnt = 0;
				chk_inum = false;
				inum = 0;
				break;
			}
			ps = i;

			if (chk_inum) {
				chk_inum = false;
				if (inum == 0) {
					skip_pt = true;
					break;
				}
				if (!_check_country_code(cutlink, inum))
					skip_pt  = true;
			}
			break;
		case ')':
			if (!ps) {
				if (i+1 < (*eo)) {
					*so = i+1;
					cnt = 0;
					break;
				}
				return false;
			}
			pe = i;

			if (skip_pt) {
				if (i+1 >= (*eo))
					return false;
				skip_pt = false;
				*so = i+1;
				cnt = 0;
				break;
			}

			if (i+1 >= (*eo)) {
				if (ps-1 > (*so)) {
					*eo = ps;
					cnt -= (pe - ps - 1);
					break;
				}
			}
			break;
		case '+':
			if (i+1 < *eo) {
				ss = strchr(text+i+1, '+');
				if (ss && ss < text+(*eo))
					return false;
			}
			*so = i;
			cnt = 0;
			chk_inum = true;
			break;
		default:
			if (isdigit((int)text[i])) {
				cnt++;
				if (chk_inum)
					inum = inum*10 + (text[i]-'0');
			}
			else
				return false;
			break;
		}
		i++;
	}

	/* check length */
	if (cnt < 4 || cnt > 15) {
		LOGD("ignore match length(%d)", cnt);
		return false;
	}

	return true;
}

static bool
_match_datetime_cb(ea_cutlink_h cutlink, const char *text,
		int *so, int *eo, const char *scheme, void *userdata)
{
	int len;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK, "_match_datetime_cb");
		return false;
	}

	if (!text || !so || !eo || !scheme) {
		LOGE("invalid argument");
		return false;
	}

	LOGD("_match_datetime_cb str(%.*s), so(%d), eo(%d)",
		*eo - *so, text + *so, *so, *eo);

	len = strlen(text);

	if (*so > 1 && isdigit((int)text[*so-1]))
		return false;

	if (*eo < len-1 && isdigit((int)text[*eo]))
		return false;

	return true;
}

static bool
_process_mfunc(const ea_cutlink_h cutlink, int *so,
	       int *eo, ea_cutlink_scheme_h sc)
{
	int txt_len;
	bool ret;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
			      "_process_mfunc");
		return false;
	}

	if (!so || !eo || !sc) {
		LOGW("invalid argument");
		return false;
	}

	txt_len = strlen(cutlink->txt);
	if (sc->def_mfunc) {

		ret = sc->def_mfunc(cutlink,
				    cutlink->txt,
				    so,
				    eo,
				    sc->name,
				    NULL);

		if (!ret) {
			LOGD("disable scheme(%s)", sc->name);
			return false;
		}

		if (*so < 0 || *so > txt_len ||
		    *eo < 0 || *eo > txt_len || *so > *eo) {
			LOGW("invalid offset, %d:%d (%d)", *so, *eo, txt_len);
			return false;
		}
	}

	if (sc->user_mfunc) {

		ret = sc->user_mfunc(cutlink,
				     cutlink->txt,
				     so,
				     eo,
				     sc->name,
				     sc->match_data);

		if (!ret) {
			LOGD("disable scheme(%s)", sc->name);
			return false;
		}

		if (*so < 0 || *so > txt_len ||
		    *eo < 0 || *eo > txt_len || *so > *eo) {
			LOGW("invalid offset, %d:%d (%d)", *so, *eo, txt_len);
			return false;
		}
	}

	return true;
}

static void
_trans_datetime_cb(const ea_cutlink_h cutlink, char **match_str,
		char **scheme,  void *userdata)
{
	enum {
		DT_UNKNOWN,
		DT_DATE,
		DT_TIME
	};
	char *str;
	char *res;
	time_t tval = 0;
	time_t cur_tval = 0;
	int i = 0;
	int len;
	int idx = 0;
	int date_num = 0;
	int time_num = 0;
	struct {
		int num;
		int type;
	} dtnum[MAX_DATETIME_NUM] = { {-1, DT_UNKNOWN}, };
	struct tm tm_val = {0, };
	struct tm *cur_tm;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK, "_trans_datetime_cb");
		return;
	}

	if (!match_str || !scheme) {
		LOGE("invalid argument");
		return;
	}

	for (i = 0 ; i < MAX_DATETIME_NUM ; i++) {
		dtnum[i].num = -1;
		dtnum[i].type = DT_UNKNOWN;
	}

	cur_tval = time(NULL);
	cur_tm = localtime(&cur_tval);
	if (!cur_tm)
		return;

	i = idx = date_num = time_num = 0;
	str = strdup(*match_str);
	len = strlen(str);
	while (i < len && idx < MAX_DATETIME_NUM) {
		switch (str[i]) {

		case '.':
			if ((i > 0) && (isalpha((int)str[i-1])))
				break;

		case '-':
		case '/':

			if (dtnum[idx].num >= 0) {
				dtnum[idx].type = DT_DATE;
				if (idx+1 < MAX_DATETIME_NUM)
					dtnum[idx+1].type = DT_DATE;
				idx++;
				date_num++;
			}
			break;

		case ':':
			if (dtnum[idx].num >= 0) {
				dtnum[idx].type = DT_TIME;
				if (idx+1 < MAX_DATETIME_NUM)
					dtnum[idx+1].type = DT_TIME;
				idx++;
				time_num++;
			}
			break;

		case ' ':
			if (dtnum[idx].num >= 0)
				idx++;
			break;

		default:

			if (isdigit((int)str[i])) {
				if (dtnum[idx].num < 0)
					dtnum[idx].num = 0;
				dtnum[idx].num = dtnum[idx].num*10 + (str[i]-'0');
			}
			else {
				if (dtnum[idx].num < 0 && idx > 0)
					idx--;

				if ((strncasecmp(str+i, "st", 2) == 0) ||
				    (strncasecmp(str+i, "nd", 2) == 0) ||
				    (strncasecmp(str+i, "rd", 2) == 0) ||
				    (strncasecmp(str+i, "th", 2) == 0)) {
					dtnum[idx].type = DT_DATE;
					idx++;
					date_num++;
				}
				else if (strncasecmp(str+i, "am", 2) == 0) {
					dtnum[idx].type = DT_TIME;
					idx++;
					time_num++;
				}
				else if (strncasecmp(str+i, "pm", 2) == 0) {
					dtnum[idx].type = DT_TIME;
					if (time_num > 0 && idx > 0) {
						dtnum[idx-1].num += 12;
					}
					else {
						dtnum[idx].num += 12;
					}
					idx++;
					time_num++;
				}
				else if (strncasecmp(str+i, "jan", 3) == 0) {
					tm_val.tm_mon = 1;
				}
				else if (strncasecmp(str+i, "feb", 3) == 0) {
					tm_val.tm_mon = 2;
				}
				else if (strncasecmp(str+i, "mar", 3) == 0) {
					tm_val.tm_mon = 3;
				}
				else if (strncasecmp(str+i, "apr", 3) == 0) {
					tm_val.tm_mon = 4;
				}
				else if (strncasecmp(str+i, "may", 3) == 0) {
					tm_val.tm_mon = 5;
				}
				else if (strncasecmp(str+i, "jun", 3) == 0) {
					tm_val.tm_mon = 6;
				}
				else if (strncasecmp(str+i, "jul", 3) == 0) {
					tm_val.tm_mon = 7;
				}
				else if (strncasecmp(str+i, "aug", 3) == 0) {
					tm_val.tm_mon = 8;
				}
				else if (strncasecmp(str+i, "sep", 3) == 0) {
					tm_val.tm_mon = 9;
				}
				else if (strncasecmp(str+i, "oct", 3) == 0) {
					tm_val.tm_mon = 10;
				}
				else if (strncasecmp(str+i, "nov", 3) == 0) {
					tm_val.tm_mon = 11;
				}
				else if (strncasecmp(str+i, "dec", 3) == 0) {
					tm_val.tm_mon = 12;
				}
				else if (strncasecmp(str+i, "sunday", 6) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon+1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 0;
				}
				else if (strncasecmp(str+i, "monday", 6) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon+1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 1;
				}
				else if (strncasecmp(str+i, "tuesday", 7) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon+1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 2;
				}
				else if (strncasecmp(str+i, "wednesday", 9) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon+1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 3;
				}
				else if (strncasecmp(str+i, "thurthday", 9) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon+1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 4;
				}
				else if (strncasecmp(str+i, "friday", 6) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon + 1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 5;
				}
				else if (strncasecmp(str+i, "saturday", 8) == 0) {
					tm_val.tm_year = cur_tm->tm_year;
					tm_val.tm_mon = cur_tm->tm_mon+1;
					tm_val.tm_mday = cur_tm->tm_mday -
							cur_tm->tm_wday + 6;
				}
			}
			break;
		}
		i++;
	}

	for (i = 0 ; i < MAX_DATETIME_NUM ; i++) {

		//LOGE("# (%d), num = %d, type = %d\n", i, dtnum[i].num, dtnum[i].type);

		if (dtnum[i].type == DT_TIME) {
			tm_val.tm_hour = dtnum[i].num;
			if (i+1 < MAX_DATETIME_NUM && dtnum[i+1].type == DT_TIME) {
				i++;
				tm_val.tm_min = dtnum[i].num;
			}
			continue;
		}
		else if (dtnum[i].type == DT_DATE) {
			if (i+2 < MAX_DATETIME_NUM && dtnum[i+2].type == DT_DATE) {
				if (dtnum[i].num > 31) {
					if (dtnum[i+1].num > 12) {
						if (dtnum[i].num >= 1900)
							dtnum[i].num -= 1900;
						tm_val.tm_year = dtnum[i++].num;
						tm_val.tm_mday = dtnum[i++].num;
						tm_val.tm_mon = dtnum[i].num;
					}
					else {
						if (dtnum[i].num >= 1900)
							dtnum[i].num -= 1900;
						tm_val.tm_year = dtnum[i++].num;
						tm_val.tm_mon = dtnum[i++].num;
						tm_val.tm_mday = dtnum[i].num;
					}
					continue;
				}

				if (dtnum[i+1].num > 12) {
					tm_val.tm_mon = dtnum[i++].num;
					tm_val.tm_mday = dtnum[i++].num;
					if (dtnum[i].num >= 1900)
						dtnum[i].num -= 1900;
					tm_val.tm_year = dtnum[i].num;
				}
				else {
					tm_val.tm_mday = dtnum[i++].num;
					tm_val.tm_mon = dtnum[i++].num;
					if (dtnum[i].num >= 1900)
						dtnum[i].num -= 1900;
					tm_val.tm_year = dtnum[i].num;
				}
				continue;

			}
			else if (i+1 < MAX_DATETIME_NUM && dtnum[i+1].type == DT_DATE) {
				if (dtnum[i].num > 12) {
					tm_val.tm_mday = dtnum[i++].num;
					tm_val.tm_mon = dtnum[i].num;
					continue;
				}

				tm_val.tm_mon = dtnum[i++].num;
				tm_val.tm_mday = dtnum[i].num;
			}
		}
		else if (dtnum[i].type == DT_UNKNOWN &&
			 dtnum[i].num > 0 &&
			 tm_val.tm_mon > 0) {

			if (dtnum[i].num > 31) {
				if (dtnum[i].num >= 1900)
					dtnum[i].num -= 1900;
				tm_val.tm_year = dtnum[i++].num;
				tm_val.tm_mday = dtnum[i].num;
				continue;
			}

			tm_val.tm_mday = dtnum[i++].num;
			if (dtnum[i].num >= 1900)
				dtnum[i].num -= 1900;
			tm_val.tm_year = dtnum[i].num;
		}
	}

	if (tm_val.tm_mon > 0)
		tm_val.tm_mon -= 1;

	if (tm_val.tm_year == 0) {
		if (cur_tm > 0) {
			tm_val.tm_year = cur_tm->tm_year;
			tm_val.tm_mon = cur_tm->tm_mon;
			tm_val.tm_mday = cur_tm->tm_mday;
		}
	}
	tval = mktime(&tm_val);

	LOGD("# year(%d), mon(%d), mday(%d), hour(%d), min(%d) ==> %s\n",
		tm_val.tm_year, tm_val.tm_mon, tm_val.tm_mday,
		tm_val.tm_hour, tm_val.tm_min, asctime(&tm_val));

	res = calloc(1, len + 20);
	if (!res) {
		LOGE("fail to allocate memory");
		return;
	}
	sprintf(res, "%lu:%s", tval, str);

	free(*match_str);
	free(str);

	*match_str = res;

}

static bool
_process_tfunc(const ea_cutlink_h cutlink, ea_cutlink_scheme_h sc,
	       int so, int eo, char **str, char **name)
{
	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
			      "_process_tfunc");
		return false;
	}

	if (!sc || !str || !name) {
		LOGW("invalid argument");
		return false;
	}

	*name = strdup(sc->name);
	*str = calloc(1, eo - so + 2);
	if (!*str) {
		LOGE("fail to allocate memory");
		return false;
	}

	snprintf(*str, eo-so+1, "%.*s", eo - so, cutlink->txt + so);

	if (sc->user_tfunc) {

		sc->user_tfunc(cutlink, str, name, sc->trans_data);

		LOGD("trans_cb, tstr(%s), tname(%s)", *str, *name);

		return true;
	}

	if (sc->def_tfunc)
		sc->def_tfunc(cutlink, str, name, NULL);

	return true;
}

static void
_generate_markup(const ea_cutlink_h cutlink, int offset, int so, int eo,
		 char *prefix, char *name, char *str, char *postfix,
		 bool ignore_scheme)
{
	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
			      "_generate_markup");
		return;
	}

	/* before prefix */
	eina_strbuf_append_length(cutlink->tagged_txt,
					  cutlink->txt + offset, so - offset);

        if ((ignore_scheme) ||
	    (name && !strcasecmp(name, EA_CUTLINK_SCHEME_MARKUP))) {

                /* matched text */
                eina_strbuf_append_length(cutlink->tagged_txt,
                                          cutlink->txt + so, eo - so);

                return;
        }

	/* user defined prefix */
	if (prefix)
		eina_strbuf_append(cutlink->tagged_txt, prefix);

	/* default prefix for a href */
	eina_strbuf_append_printf(cutlink->tagged_txt,
				  DEF_AHREF_PREFIX,
				  name ? name : "",
				  str ? str : "");

	/* matched text */
	eina_strbuf_append_length(cutlink->tagged_txt, cutlink->txt + so, eo - so);

	/* default postfix for a href */
	eina_strbuf_append_printf(cutlink->tagged_txt, DEF_AHREF_POSTFIX);

	/* user defined postfix */
	if (postfix)
		eina_strbuf_append(cutlink->tagged_txt, postfix);

}

static int
_minfo_compare_cb(const void *data1, const void *data2)
{
	struct ea_match_info_s *minfo1 = (struct ea_match_info_s *)data1;
	struct ea_match_info_s *minfo2 = (struct ea_match_info_s *)data2;

	if (!minfo1)
		return 1;
	if (!minfo2)
		return -1;

	if (minfo1->so < minfo2->so)
		return -1;
	if (minfo1->so > minfo2->so)
		return 1;

	if (minfo1->eo < minfo2->eo)
		return 1;
	if (minfo1->eo > minfo2->eo)
		return -1;

	return 0;
}

static Eina_List *
_ea_cutlink_get_match_list(ea_cutlink_h cutlink)
{
	regex_t reg;
	regmatch_t match;
	ea_cutlink_scheme_h sc;
	Eina_List *l;
	Eina_List *mlist = NULL;
	struct ea_match_info_s *minfo = NULL;
	char err[DEF_STRING_LEN_MAX];
	int ret;
	int txt_len;
	int offset = 0;
	int mso = 0;
	int meo = 0;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
			      "_ea_cutlink_get_match_list");
		return NULL;
	}

	if (!cutlink->txt) {
		LOGW("text is not set, yet");
		return NULL;
	}

	offset = 0;
	txt_len = strlen(cutlink->txt);
	EINA_LIST_FOREACH(cutlink->sc_list, l, sc) {

		offset = 0;

		ret = regcomp(&reg, sc->pattern, REG_EXTENDED | REG_ICASE);
		if (ret != 0) {
			regerror(ret, &reg, err, DEF_STRING_LEN_MAX);
			LOGE("fail to compile regex(%d:%s)", ret, err);
			return NULL;
		}

reg_retry:

		ret = regexec(&reg, cutlink->txt + offset, 1, &match, 0);
		if (ret == 0) {

			mso = offset + match.rm_so;
			meo = offset + match.rm_eo;

			if (_process_mfunc(cutlink, &mso, &meo, sc) != true) {
				offset = meo;
				goto reg_retry;
			}

			minfo = calloc(1, sizeof(struct ea_match_info_s));
			if (!minfo) {
				LOGE("fail to allocate memory");
				regfree(&reg);
				return mlist;
			}

			minfo->sc = sc;
			minfo->so = mso;
			minfo->eo = meo;

			mlist = eina_list_sorted_insert(mlist, _minfo_compare_cb, minfo);

			offset = meo;
			if (offset < txt_len)
				goto reg_retry;
		}

		regfree(&reg);

	}

	return mlist;
}

static bool
_ea_cutlink_process(ea_cutlink_h cutlink)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;
	bool ret;
	int last_eo = 0;
	char *tname = NULL;
	char *tstr = NULL;
	Eina_List *mlist = NULL;
	struct ea_match_info_s *minfo = NULL;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
			      "_ea_cutlink_process");
		return false;
	}

	if (!cutlink->txt) {
		LOGW("text is not set, yet");
		return false;
	}

	mlist = _ea_cutlink_get_match_list(cutlink);
	if (!mlist) {
		eina_strbuf_append(cutlink->tagged_txt, cutlink->txt);
		return true;
	}

	last_eo = 0;
	tstr = NULL;
	tname = NULL;
	EINA_LIST_FOREACH(mlist, l, minfo) {
		if (!minfo) break;

		if (minfo->so < last_eo)
			continue;

		sc = minfo->sc;
		if (sc->excepted) {
			_generate_markup(cutlink, last_eo, minfo->so, minfo->eo,
					 NULL, NULL, NULL, NULL,
					 true);
			last_eo = minfo->eo;
			continue;
		}

		ret = _process_tfunc(cutlink, minfo->sc, minfo->so, minfo->eo,
				     &tstr, &tname);
		if (ret) {
			_generate_markup(cutlink, last_eo, minfo->so, minfo->eo,
					 sc->prefix, tname, tstr, sc->postfix,
					 false);
		}

		if (tname)
			free(tname);
		tname = NULL;
		if (tstr)
			free(tstr);
		tstr = NULL;

		last_eo = minfo->eo;
	}

	eina_strbuf_append(cutlink->tagged_txt, cutlink->txt + last_eo);

	EINA_LIST_FREE(mlist, minfo)
		free(minfo);

	return true;
}

static void
_anchor_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Entry_Anchor_Info *ainfo = event_info;
	ea_cutlink_h cutlink = (ea_cutlink_h)data;
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK, "_anchor_clicked_cb");
		return;
	}

	LOGI("_anchor_clicked_cb");

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strncasecmp(sc->name, ainfo->name, strlen(sc->name))) {
			if (sc->anchor_enable && sc->afunc) {

				LOGD("_anchor_clicked, cutlink(%p), name(%s)",
				     cutlink, sc->name);

				sc->afunc(cutlink, sc,
						(char *)ainfo->name);

				return;
			}
		}

}

static void
_entry_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	ea_cutlink_h cutlink = (ea_cutlink_h)data;

	evas_object_event_callback_del(cutlink->entry,
				       EVAS_CALLBACK_DEL, _entry_del_cb);
	evas_object_smart_callback_del(cutlink->entry,
				       "anchor,clicked", _anchor_clicked_cb);
	cutlink->entry = NULL;
}

static char *
_load_file(const char *file)
{
	Eina_File *f;
	void *tmp;
	char *text = NULL;

	if (!file)
		return NULL;

	f = eina_file_open(file, false);
	if (!f) {
		LOGE("fail to open file(%s)", file);
		return NULL;
	}

	tmp = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
	if (!tmp) {
		LOGE("fail to map memory");
		goto ret_fail;
	}

	text = calloc(1, eina_file_size_get(f) + 1);
	if (!text)
		goto ret_fail;

	memcpy(text, tmp, eina_file_size_get(f));

	if (eina_file_map_faulted(f, tmp)) {
		LOGE("map faulted");
		free(text);
		text = NULL;
	}

ret_fail:

	if (tmp) eina_file_map_free(f, tmp);
	eina_file_close(f);

	return text;

}

/*===========================================================================*
 *                                Global                                     *
 *===========================================================================*/


/*===========================================================================*
 *                                  API                                      *
 *===========================================================================*/

EXPORT_API int
ea_cutlink_create(ea_cutlink_h *cutlink,
		  Evas_Object *entry,
		  unsigned char mask)
{
	ea_cutlink_h cl;
	ea_cutlink_scheme_h sc;
	int i;
	int dt_idx = 0;

	if (!cutlink || !entry ||
	    (strcmp(elm_object_widget_type_get(entry), "elm_entry") &&
	    strcmp(elm_object_widget_type_get(entry), "elm_label"))) {
		LOGW("invalid argument");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	cl = calloc(1, sizeof(struct ea_cutlink_s));
	if (!cl)
		return EA_CUTLINK_ERROR_OUT_OF_MEMORY;

	cl->tagged_txt = eina_strbuf_new();
	if (!cl->tagged_txt) {
		free(cl);
		return EA_CUTLINK_ERROR_OUT_OF_MEMORY;
	}

	LOGI("ea_cutlink_create, entry(%p), mask(%d)", entry, mask);

	for (i = 0 ; i < EINA_C_ARRAY_LENGTH(def_scheme) ; i++) {

		if (def_scheme[i].mask == EA_CUTLINK_DATETIME)
			dt_idx = i;

		if (mask & def_scheme[i].mask) {
			sc = _ea_cutlink_scheme_new(def_scheme[i].mask,
						    def_scheme[i].name,
						    def_scheme[i].pattern,
						    def_scheme[i].afunc,
						    def_scheme[i].mfunc,
						    def_scheme[i].tfunc);
			if (!sc) {
				LOGE("fail to crete new scheme");
				continue;
			}
			cl->sc_list = eina_list_append(cl->sc_list, sc);

			LOGI("mask(%d) scheme(%s), pattern(%s) added",
				sc->mask, sc->name, sc->pattern);
		}
	}

	if ((mask & EA_CUTLINK_PHONE) && !(mask & EA_CUTLINK_DATETIME)) {
		sc = _ea_cutlink_scheme_new(def_scheme[dt_idx].mask,
					    def_scheme[dt_idx].name,
					    def_scheme[dt_idx].pattern,
					    def_scheme[dt_idx].afunc,
					    def_scheme[dt_idx].mfunc,
					    def_scheme[dt_idx].tfunc);
		if (!sc) {
			LOGE("fail to crete new scheme");
		}
		else {
			sc->excepted = true;
			cl->sc_list = eina_list_append(cl->sc_list, sc);

			LOGI("mask(%d) scheme(%s), pattern(%s) excepted added",
				sc->mask, sc->name, sc->pattern);
		}
	}

	cl->entry = entry;
	cl->txt = NULL;
	cl->file = NULL;
	cl->format = ELM_TEXT_FORMAT_PLAIN_UTF8;

	for (i = 0 ; i < EINA_C_ARRAY_LENGTH(country_code) ; i++)
		cl->cc_list = eina_list_sorted_insert(cl->cc_list,
			_country_code_comp_func, (void *)(country_code[i]));

	evas_object_event_callback_add(cl->entry, EVAS_CALLBACK_DEL,
					_entry_del_cb, cl);
	evas_object_smart_callback_priority_add(cl->entry, "anchor,clicked",
						EVAS_CALLBACK_PRIORITY_AFTER,
						_anchor_clicked_cb, cl);

	EA_MAGIC_SET(cl, EA_MAGIC_CUTLINK);

	(*cutlink) = cl;

	return EA_CUTLINK_ERROR_NONE;
}

EXPORT_API void
ea_cutlink_destroy(ea_cutlink_h cutlink)
{
	ea_cutlink_scheme_h sc;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK, "ea_cutlink_destroy");
		return;
	}

	EA_MAGIC_SET(cutlink, EA_MAGIC_NONE);

	EINA_LIST_FREE(cutlink->sc_list, sc)
		_ea_cutlink_scheme_free(sc);

	if (cutlink->entry) {
		evas_object_event_callback_del(cutlink->entry,
						EVAS_CALLBACK_DEL,
						_entry_del_cb);
		evas_object_smart_callback_del(cutlink->entry,
						"anchor,clicked",
						_anchor_clicked_cb);
		cutlink->entry = NULL;
	}

	if (cutlink->txt)
		free(cutlink->txt);
	if (cutlink->tagged_txt)
		eina_strbuf_free(cutlink->tagged_txt);
	if (cutlink->file)
		free(cutlink->file);
	if (cutlink->cc_list)
		eina_list_free(cutlink->cc_list);

	free(cutlink);

	LOGI("ea_cutlink_destroy");
}

EXPORT_API int
ea_cutlink_scheme_add(ea_cutlink_h cutlink, char *scheme, char *pattern)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_scheme_add");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !pattern) {
		LOGE("invalid argument");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			LOGW("already exist scheme(%s)", scheme);
			return EA_CUTLINK_ERROR_INVALID_PARAMETER;
		}

	sc = _ea_cutlink_scheme_new(EA_CUTLINK_USER, scheme,
				    pattern, NULL, NULL, NULL);
	if (!sc)
		return EA_CUTLINK_ERROR_OUT_OF_MEMORY;

	cutlink->sc_list = eina_list_append(cutlink->sc_list, sc);

	LOGI("match(%d), scheme(%s), pattern(%s) is added",
	    EA_CUTLINK_USER, scheme, pattern);

	return EA_CUTLINK_ERROR_NONE;
}

EXPORT_API int
ea_cutlink_scheme_del(ea_cutlink_h cutlink, char *scheme)
{
	ea_cutlink_scheme_h sc = NULL;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
			      "ea_cutlink_scheme_del");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			cutlink->sc_list = eina_list_remove_list(
							cutlink->sc_list, l);
			_ea_cutlink_scheme_free(sc);

			LOGI("name(%s) is deleted", scheme);

			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_match_cb_set(ea_cutlink_h cutlink,
			char *scheme,
			ea_cutlink_match_cb mfunc,
			void *userdata)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_match_cb_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			sc->user_mfunc = mfunc;
			sc->match_data = userdata;

			LOGD("scheme(%s), mfunc(%p), userdata(%p)",
			    scheme, mfunc, userdata);

			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_trans_cb_set(ea_cutlink_h cutlink,
			char *scheme,
			ea_cutlink_trans_cb user_tfunc,
			void *userdata)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_trans_cb_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			sc->user_tfunc = user_tfunc;
			sc->trans_data = userdata;

			LOGD("scheme(%s), user_tfunc(%p), userdata(%p)",
			    scheme, user_tfunc, userdata);

			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_anchor_enable_set(ea_cutlink_h cutlink,
			     char *scheme,
			     bool enable)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_anchor_enable_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			sc->anchor_enable = enable;

			LOGD("scheme(%s), anchor_enable(%d)", scheme, enable);

			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_pattern_set(ea_cutlink_h cutlink, char *scheme, char *pattern)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_pattern_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !pattern) {
		LOGE("invalid argument");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			if (sc->pattern) free(sc->pattern);
			sc->pattern = strdup(pattern);
			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}


EXPORT_API int
ea_cutlink_pattern_get(ea_cutlink_h cutlink, char *scheme, char **pattern)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_pattern_get");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !pattern)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			(*pattern) = strdup(sc->pattern);
			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_markup_set(ea_cutlink_h cutlink, const char *markup)
{
	bool ret;
	const char *str;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_markup_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!markup) {
		LOGE("invalid text");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!cutlink->entry) {
		LOGE("entry is invalid");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (cutlink->txt)
		free(cutlink->txt);
	cutlink->txt = strdup(markup);

	if (cutlink->tagged_txt)
		eina_strbuf_reset(cutlink->tagged_txt);

	ret = _ea_cutlink_process(cutlink);
	if (!ret) {
		LOGE("fail to process txt");
		elm_object_text_set(cutlink->entry, "");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	str = eina_strbuf_string_get(cutlink->tagged_txt);

	LOGD("set txt: %-30s", markup);
	LOGD("tagged txt: %-30s", str);

	elm_object_text_set(cutlink->entry, str);

	return EA_CUTLINK_ERROR_NONE;
}

EXPORT_API int
ea_cutlink_markup_get(ea_cutlink_h cutlink, char **markup)
{
	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_markup_get");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!cutlink->entry || !markup) {
		LOGE("invalid parameter");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	(*markup) = strdup(elm_object_text_get(cutlink->entry));

	return EA_CUTLINK_ERROR_NONE;
}

EXPORT_API int
ea_cutlink_markup_apply(ea_cutlink_h cutlink, const char *markup, char **tagged)
{
	bool ret;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_markup_apply");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!markup || !tagged) {
		LOGE("invalid parameter");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (cutlink->txt)
		free(cutlink->txt);
	cutlink->txt = strdup(markup);

	if (cutlink->tagged_txt)
		eina_strbuf_reset(cutlink->tagged_txt);

	ret = _ea_cutlink_process(cutlink);
	if (!ret) {
		LOGE("fail to process txt");
                *tagged = NULL;
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	(*tagged) = strdup(eina_strbuf_string_get(cutlink->tagged_txt));

	LOGD("set txt: %-30s", markup);
	LOGD("tagged txt: %-30s", *tagged);

	return EA_CUTLINK_ERROR_NONE;
}


EXPORT_API int
ea_cutlink_file_set(ea_cutlink_h cutlink,
		    const char *file,
		    Elm_Text_Format format)
{
	char *markup = NULL;
	char *str;
	int ret;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_file_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!cutlink->entry)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	if (!file) {
		elm_object_text_set(cutlink->entry, "");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (cutlink->file)
		free(cutlink->file);
	cutlink->file = strdup(file);
	cutlink->format = format;

	switch (format) {
	case ELM_TEXT_FORMAT_PLAIN_UTF8:
		str = _load_file(file);
		if (str)
			markup = elm_entry_utf8_to_markup(str);
		free(str);
		break;

	case ELM_TEXT_FORMAT_MARKUP_UTF8:
		markup = _load_file(file);
		break;

	default:
		markup = NULL;
		break;
	}

	ret = ea_cutlink_markup_set(cutlink, markup);

	if (markup)
		free(markup);
	if (ret)
		elm_object_text_set(cutlink->entry, "");

   return ret;
}

EXPORT_API int
ea_cutlink_file_get(ea_cutlink_h cutlink,
		    const char **file,
		    Elm_Text_Format *format)
{
	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_file_get");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!file || !format) {
		LOGE("invalid argument");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (cutlink->file)
		(*file) = strdup(cutlink->file);
	else
		(*file) = NULL;

	if (format)
		(*format) = cutlink->format;

	return EA_CUTLINK_ERROR_NONE;
}

EXPORT_API int
ea_cutlink_prefix_set(ea_cutlink_h cutlink, char *scheme, char *prefix)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_prefix_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !prefix) {
		LOGE("invalid argument");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			if (sc->prefix)
				free(sc->prefix);
			sc->prefix = strdup(prefix);
			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_prefix_get(ea_cutlink_h cutlink, char *scheme, char **prefix)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_prefix_get");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !prefix)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme))  {
			if (sc->prefix)
				(*prefix) = strdup(sc->prefix);
			else
				(*prefix) = NULL;

			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_postfix_set(ea_cutlink_h cutlink, char *scheme, char *postfix)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_postfix_set");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !postfix) {
		LOGE("invalid argument");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			if (sc->postfix)
				free(sc->postfix);
			sc->postfix = strdup(postfix);
			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}

EXPORT_API int
ea_cutlink_postfix_get(ea_cutlink_h cutlink, char *scheme, char **postfix)
{
	ea_cutlink_scheme_h sc;
	Eina_List *l;

	if (!EA_MAGIC_CHECK(cutlink, EA_MAGIC_CUTLINK)) {
		EA_MAGIC_FAIL(cutlink, EA_MAGIC_CUTLINK,
				"ea_cutlink_postfix_get");
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;
	}

	if (!scheme || !postfix)
		return EA_CUTLINK_ERROR_INVALID_PARAMETER;

	EINA_LIST_FOREACH(cutlink->sc_list, l, sc)
		if (!strcasecmp(sc->name, scheme)) {
			if (sc->postfix)
				(*postfix) = strdup(sc->postfix);
			else
				(*postfix) = NULL;

			return EA_CUTLINK_ERROR_NONE;
		}

	LOGD("cannot find scheme(%s)", scheme);

	return EA_CUTLINK_ERROR_INVALID_PARAMETER;
}
