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

#include <Elementary.h>
#include <efl_assist.h>
#include <stdio.h>
#include <stdbool.h>

#if 0
#define DEF_TEST_TEXT_01 "" \
			"1234\n" \
			"12345\n" \
			"123456789012345\n" \
			"1234567890123456\n" \
			"010-5134-0370\n" \
			"1-234-5-678\n" \
			"01-0-513-4-0370\n" \
			"010--123-456\n" \
			"010 5134 0370\n" \
			"010  5134 0370\n" \
			"+010-5134-0370\n" \
			"82+010-5134-0370\n" \
			"ab+010-5134-0370\n" \
			"82+010-5134+0370\n" \
			"(02)2255-1234\n" \
			"02)1234-5678\n" \
			"2255-1234(02)\n" \
			"(02)2255-12(34)\n" \
			"(02)(031)2255-1234\n" \
			"123;010-5134-0370,321\n" \
			"010;513,403\n" \
			"+82(02)2255-1234\n" \
			"+85(02)2255-1234\n" \
			"+(82)2255-1234\n"
#else
#define DEF_TEST_TEXT_01 "\n" \
			"0. 1 \n" \
			"1. 12 \n" \
			"2. 123 \n" \
			"3. 1234 \n" \
			"4. 12345 \n" \
			"5. 123456 \n" \
			"6. 1234567 \n" \
			"7. 12345678 \n" \
			"8. 12345678901234 \n" \
			"9. 123456789012345 \n" \
			"0. 1234567890123456 \n" \
			"a. <color=#000000FF> \n" \
			"a. tel+1456789 \n" \
			"b. +1234567890 \n" \
			"c. +123456789 \n" \
			"d. 023456 \n" \
			"e. 1234567 \n" \
			"f. 1234567890 \n" \
			"g. 023456 \n" \
			"h. 0234567890123456 \n" \
			"i. 123-456-6890 \n" \
			"j. 123 456-6890 \n" \
			"k. 0-234-5-678 \n" \
			"l. 023--456-789 \n" \
			"m. +123-456-7890 \n" \
			"n. 12+2345-6789-01 \n" \
			"o. ab+1456-7890-12 \n" \
			"p. 12+045-678-90+123 \n" \
			"q. (02)1234-5678 \n" \
			"r. (021234-5678 \n" \
			"s. 02)0234-5678 \n" \
			"t. 1234-5678(90) \n" \
			"u. (02)1234-5678(9) \n" \
			"v. (02)(034)1234-5678 \n" \
			"x. 123;1567-8901,23 \n" \
			"y. 023,056;089 \n" \
			"z. +82(02)12345678 \n" \
			"a. +85(02)12345678 \n" \
			"b. +(82)12345678 \n" \
			"c. Tel : 131-123-1234 \n" \
			"d. Tel :110-123-1234 \n" \
			"e. Tel : +12-31-123-1234 \n" \
			"f. Tel : +12-10-1234-2567 \n" \
			"a. abc@bbdfdsa.com \n" \
			"b. 3214abc@bbdfdsa.com \n" \
			"c. ab_fdfdc@bbdfdsa.com \n" \
			"d. ab.fdfdc@bbdfdsa.com \n" \
			"e. ab..fdfdc@bbdfdsa.com \n" \
			"f. ab.+fdfdc@bbdfdsa.com \n" \
			"g. ab._fdfdc@bbdfdsa.com \n" \
			"h. ab.f_dfdc@bbdfdsa.com \n" \
			"i. [abc@bbfda.com] \n" \
			"j. <abc@bbdfdsa.com> \n" \
			"a. http://www.naver.com \n" \
			"b. https://www.naver.com \n" \
			"c. www.naver.com \n" \
			"d. mms://123.456.789.123 \n" \
			"e. file://123.456.789.123 \n" \
			"f. ftp://123.456.789.123 \n" \
			"g. rtsp://localhost/test.wav \n" \
			"h. wap://123.456.789.123 \n" \
			"i. 123.456.789.123 \n" \
			"j. \\\\server\\share\\file_path \n" \
			"k. \\\\activesync\\unck10 \n" \
			"l. mailto:abc@def.com \n" \
			"m. 123.456.789 \n" \
			"a. 2011/1/21 \n" \
			"b. 11/1/21 \n" \
			"c. 1/21/2011 \n" \
			"d. 1/21/11 \n" \
			"e. 21/1/2011 \n" \
			"f. 21/1/11 \n" \
			"g. 2011-1-21 \n" \
			"h. 11-1-21 \n" \
			"i. 1-21-2011 \n" \
			"j. 1-21-11 \n" \
			"k. 21-1-2011 \n" \
			"l. 21-1-11 \n" \
			"m. 2011.1.21 \n" \
			"n. 1.21.2011 \n" \
			"o. 1.21.11 \n" \
			"p. 21.1.2011 \n" \
			"q. 21.1.11 \n" \
			"r. 10 am \n" \
			"s. 10am \n" \
			"t. 10:00 \n" \
			"u. 10:00 am \n" \
			"v. 2011/1/21 9am \n" \
			"w. 1/21/2011 12:30 \n" \
			"x. 21/1/2011 7:30 pm \n" \
			"y. 10am 2011/1/21 \n" \
			"z. 11:20 1/21/2011 \n" \
			"a. 8:30pm 21/1/2011 \n" \
			"b. January 21 2011 \n" \
			"c. Jan. 21 2011 \n" \
			"d. 21 January 2011 \n" \
			"e. 21 Jan. 2011 \n" \
			"f. 21 January 2011 11am \n" \
			"g. 21 January 2011 1:40 \n" \
			"h. 21 Jan. 2011 8:10pm \n" \
			"i. January 21 2011 11am \n" \
			"j. January 21 2011 1:40 \n" \
			"k. Jan. 21 2011 8:10pm \n" \
			"l. 11am January 21 2011 \n" \
			"m. 1:40 January 21 2011 \n" \
			"n. 8:10pm Jan. 21 2011 \n" \
			"o. Tonight \n" \
			"p. Tonight 11:30 \n" \
			"q. Tonight 11am \n" \
			"r. Friday night 9:30 \n" \
			"s. Friday 9am \n" \
			"t. Morning 9pm \n" \
			"u. Friday night 9:30am \n" \
			"0. 111.111.111.111 \n" \
			"1. 255.256.255.255 \n" \
			"2. 0.0.0.0 \n" \
			"3. 0.1.1.1 \n" \
			"4. 1.1.1.1 \n" \
			"5. Scheme:abderfsdf \n" \
			"6. http://111.111.111.111 \n" \
			"7. http://111.111.111 \n" \
			"8. 111.111.111 \n" \
			"0. www.hello.pl \n" \
			"1. ww.hello.co.kr \n" \
			"2. wap.hello.pl \n" \
			"3. http://wap.hello.pl \n" \
			"4. abc.com \n" \
			"5. abccom \n" \
			"6. geo:37.21706,137.468928?q=tokyo&z=3 \n" \
			"7. geo:40.716463,-73.998351 \n" \
			"8. geo:37.013718,-85.8212?star_addr=NY&dest_addr=Boston \n"

#endif

static bool
_match_cb(const ea_cutlink_h cutlink, const char *text, int *start,
	  int *end, const char *scheme, void *userdata)
{
	printf("name(%s), start(%d), end(%d)\n", scheme, *start, *end);

	/* remove the last chracter in clickable link */
	*end = *end - 1;

	/* disable clickable link in case of EA_CUTLINK_DATETIME scheme */
	if (!strcmp(scheme, EA_CUTLINK_SCHEME_DATETIME))
		return false;

	return true;
}

static void
_trans_cb(const ea_cutlink_h cutlink, char **matched,
	  char **scheme, void *userdata)
{
	printf("scheme(%s), matched(%s)\n", *scheme, *matched);

	/* remove scheme name string */
	//*scheme = NULL;
}

static void
_anchor_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Entry_Anchor_Info *ainfo = event_info;
	ea_cutlink_h cutlink = data;

	printf("anchor_clicked (%s)\n", ainfo->name);

	if (!strncmp(ainfo->name, EA_CUTLINK_SCHEME_UNC,
		    strlen(EA_CUTLINK_SCHEME_UNC)))
		/* disable the default anchor,clicked callback */
		ea_cutlink_anchor_enable_set(cutlink,
			     EA_CUTLINK_SCHEME_UNC, false);
}

int
elm_main(int argc, char **argv)
{
	Evas_Object *win, *bg, *entry, *scroller;
	ea_cutlink_h cutlink;
	char *txt = NULL;

	win = elm_win_add(NULL, "cutlink test window", ELM_WIN_BASIC);
	if (!win) {
		printf("fail to create window\n");
		return -1;
	}

	elm_win_title_set(win, "cutlink test");

	evas_object_resize(win, 720, 1280);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	bg = elm_bg_add(win);
	elm_bg_color_set(bg, 0, 0, 0);
	evas_object_size_hint_weight_set(bg,
					 EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	entry = elm_entry_add(win);
	elm_entry_editable_set(entry, EINA_FALSE);
	evas_object_size_hint_weight_set(entry,
					 EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, entry);
	evas_object_show(entry);

	scroller = elm_scroller_add(win);
	evas_object_size_hint_weight_set(scroller,
					 EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, scroller);
	evas_object_show(scroller);
	elm_object_content_set(scroller, entry);

	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(scroller,
				ELM_SCROLLER_POLICY_ON,
				ELM_SCROLLER_POLICY_ON);
	elm_scroller_propagate_events_set(scroller, EINA_TRUE);

	/* create cutlink object */
	ea_cutlink_create(&cutlink, entry, EA_CUTLINK_ALL);

	/* add a new scheme */
	ea_cutlink_scheme_add(cutlink, "hello:", "simple!");

	/* set match_cb for a new scheme */
	ea_cutlink_match_cb_set(cutlink, "hello:", _match_cb, NULL);

	/* set trans_cb for a new scheme */
	ea_cutlink_trans_cb_set(cutlink, EA_CUTLINK_SCHEME_URI,
				_trans_cb, NULL);

	/* set prefix and postfix for a new scheme */
	ea_cutlink_prefix_set(cutlink, EA_CUTLINK_SCHEME_URI,
			      "<color=#22819DEF underline=on "
			      "underline_color=#22819dFF>");
	ea_cutlink_postfix_set(cutlink, EA_CUTLINK_SCHEME_URI, "</color>");

	evas_object_smart_callback_add(entry, "anchor,clicked",
				       _anchor_clicked, cutlink);

	/* set text for parsing */
	ea_cutlink_markup_set(cutlink,
			      elm_entry_utf8_to_markup(DEF_TEST_TEXT_01));
	//ea_cutlink_file_set(cutlink, "test.txt", ELM_TEXT_FORMAT_PLAIN_UTF8);

	evas_object_show(win);

	/* get a text */
	ea_cutlink_markup_get(cutlink, &txt);

	printf("Origin Text:\n%s\n", DEF_TEST_TEXT_01);
	printf("Markup Text:\n%s\n", txt);

	ea_cutlink_markup_apply(cutlink, DEF_TEST_TEXT_01, &txt);
	printf("Applied Text:\n%s\n", txt);

	if (txt)
		free(txt);

	elm_run();

	/* destroy cutlink object */
	ea_cutlink_destroy(cutlink);

	elm_shutdown();

	return 0;
}
ELM_MAIN()
