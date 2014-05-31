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

#ifndef __EFL_ASSIST_CUTLINK_H__
#define __EFL_ASSIST_CUTLINK_H__

#include <Elementary.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page ea_cutlink_01_example_page Basic cutlink usage
 * @dontinclude ea_cutlink_01.c
 *
 * To the use of cutlink function, you need to include efl_assist.h file.
 * @skip efl_assist.h
 * @until #include
 *
 * This the callback we are going to use to the decide whether accepts
 * the matching or not. It this callback returns @c true, it indicates
 * that the matching is accepted. Then, the clickable link will be generated
 * against the matching string. If this callback returns @c false,
 * then the clickable link will not be generated. If you want to adjust the
 * range of clickable link, then give the appropriate value to start and end
 * variables.
 *
 * @remarks If the adjusted start and end variable is incorrect, then
 *          the clickable link will not be generated even the callback
 *          function returns @c true.
 *
 * @skip static
 * @until }
 *
 * This the callback we are going to use to the change the matched string.
 * As well as the matched string, even the scheme name can be adjustable.
 * The clickable link will be generated as the form of
 * "scheme name""matched string". For example, if the scheme name is "http://"
 * and the matched string is "www.tizen.org", then the clickable link
 * will be generated as "http://www.tizen.org".
 *
 * @remarks It does not changed the text displayed. It just change the string
 *          which passess the link callback function.
 *
 * @remarks If a scheme name is changed, then the default link callback
 *          function will not be called forcefully.
 *
 * @until }
 *
 * This the callback we are going to use to react when a link is clicked.
 * You can also register link callback function.
 *
 * @remarks It is called prior to the default link callback function.
 *          If you do not want to the defautl link callback function is called,
 *          then call ea_cutlink_anchor_enable_set() with @c false parameter.
 *
 * @until }
 *
 * Before the create cutlink object, the entry object should be created at first.
 *
 * @skip elm_entry_add
 * @until elm_entry_add
 *
 * To create a cutlink objectk, call ea_cutlink_crete() function with
 * the pre-created entry object and the combination of scheme type that
 * you want to create a clickable link.
 *
 * @skip ea_cutlink_create
 * @until ea_cutlink_create
 *
 * If you want to add a custom regular expressions, then call
 * ea_cutlink_scheme_add() function with appropriate scheme name and
 * regex string.
 *
 * @skip ea_cutlink_scheme_add
 * @until ea_cutlink_scheme_add
 *
 * If you want to register match_cb function, then call
 * ea_cutlink_match_cb_set() function.
 *
 * @skip ea_cutlink_match_cb_set
 * @until ea_cutlink_match_cb_set
 *
 * If you want to register trans_cb function, then call
 * ea_cutlink_trans_cb_set() function.
 *
 * @skip ea_cutlink_trans_cb_set
 * @until ea_cutlink_trans_cb_set
 *
 * If you want to change the style of link, then call
 * ea_cutlink_prefix_set() and ea_cutlink_postfix_set() function with
 * proper argument. The prefix string will be prepended before the matched string.
 * And the postfix string will be appended after the matched string.
 *
 * @skip ea_cutlink_prefix_set
 * @until ea_cutlink_postfix_set
 *
 * If you want to do someting when the link is clicked, then register
 * anchor,clicked callback function.
 *
 * @skip evas_object_smart_callback_add
 * @until cutlink
 *
 * To set the text to be parsed, just call ea_cutlink_markup_set() function.
 *
 * @skip ea_cutlink_markup_set
 * @until DEF_TEST_TEXT_01
 *
 * To get the text to be parsed, just call ea_cutlink_markup_get() function.
 *
 * @skip ea_cutlink_markup_get
 * @until ea_cutlink_markup_get
 *
 * The full source ocde can be found on the examples folder
 * on the @ref ea_cutlink_01_c "ea_cutlink_01.c" file.
 */

/**
 * @page ea_cutlink_01_c Basic cutlink usage example
 *
 * @include ea_cutlink_01.c
 * @example ea_cutlink_01.c
 */

/**
 * @addtogroup efl_assist_cutlink cutlink
 *
 * @brief These functions provide cutlink management.
 *
 * The cutlink is a clickable link in a rich text. The cutlink library
 * takes a piece of text and a regular expression, then turns
 * all of the regex matches in the text into the clickable links.
 * It is useful for matching things like phone number, email addresses,
 * web urls, etc. And allowing them to react when the user click them.
 *
 * Every patten that is to be matched should have a scheme prefix.
 * The scheme prefix will prepended to the matched text when
 * the clickable link is created. For instance, if there is a scheme
 * "http://", and the pattern matches "www.tizen.org", then the scheme
 * prefix will be prepended to the matched string. As a result the final
 * string "http://www.tizen.org" will be generated when the clickable
 * link is created.
 *
 * The cutlink library provide default schemes and patterns that are
 * phone number, email address, date & time, unc and uri. The user
 * can choose which scheme and patterns will be applied when
 * creating cutlink object.
 *
 * The cutlink library itself registers anchor,clicked callback
 * to the entry which is passed when creating cutlink object.
 * If the application registers anchor,clicked callback at the same time,
 * the application callback function will be called prior to the callback
 * of cutlink library.
 *
 * The cutlink library provides default application services such as
 * launch phone application in case of the link of phone number is clicked.
 * If you want to disable the default application services,
 * disables anchor,clicked callback of the cutlink library by
 * calling ea_cutlink_anchor_enable_set() with @c false parameter.
 * Currently,
 *
 *
 * See here some examples:
 * @li @ref ea_cutlink_01_example_page
 */

/**
 * @defgroup efl_assist_cutlink cutlink
 *
 * @{
 */


/**
 * @brief Cutlink type: telephone number.
 */
#define EA_CUTLINK_SCHEME_PHONE "tel:"

/**
 * @brief Cutlink type: email address.
 */
#define EA_CUTLINK_SCHEME_EMAIL "mailto:"

/**
 * @brief Cutlink type: date time.
 */
#define EA_CUTLINK_SCHEME_DATETIME "datetime:"

/**
 * @brief Cutlink type: UNC (Uniform Naming Convension)
 */
#define EA_CUTLINK_SCHEME_UNC "unc:"

/**
 * @brief Cutlink type: URI (Uniform Resource Identifier)
 */
#define EA_CUTLINK_SCHEME_URI "uri:"

/**
 * @brief Cutlink handle.
 */
typedef struct ea_cutlink_s *ea_cutlink_h;

/**
 * @brief Enumeration of error code for cutlink
 */
typedef enum {
	EA_CUTLINK_ERROR_NONE = 0,
	EA_CUTLINK_ERROR_INVALID_PARAMETER =  -EINVAL,
	EA_CUTLINK_ERROR_OUT_OF_MEMORY = -ENOMEM
} ea_cutlink_error_e;

/**
 * @brief Enumeration of cutlink type.
 */
typedef enum {
	EA_CUTLINK_NONE = 0x00,		/**< None */
	EA_CUTLINK_USER = 0x01,		/**< User defined */
	EA_CUTLINK_PHONE = 0x02,	/**< Phone number */
	EA_CUTLINK_EMAIL = 0x04,	/**< Email address */
	EA_CUTLINK_DATETIME = 0x08,	/**< Date & Time */
	EA_CUTLINK_UNC = 0x10,		/**< UNC(Uniform Naming Convension) */
	EA_CUTLINK_URI = 0x20,		/**< URI(Uniform Resource Identifier)*/
	EA_CUTLINK_ALL = 0xFF,		/**< All of type */
} ea_cutlink_mask_e;

/**
 * @brief Determine whether it accepts the matching or not
 *
 * @details This callback function makes application can determine whether
 *          it accepts the matching or not. If this callback returns @c true,
 *          it indicates the matching is accepted. Then, the clickable link
 *          will be generated automatically. If this callback returns @c false,
 *          then the clickable link will not be generated.
 *          The both start offset and end offset also can be adjustable.
 *
 * @param [in] cutlink The cutlink object.
 * @param [in] text The original text used in parsing
 * @param [in,out] start_offset The start offset of matched string
 * @param [in,out] end_offset offset The end offset of matched string
 * @param [in] scheme The scheme name
 * @param [in] userdata The user data passed from ea_cutlink_match_cb_set()
 *
 * @return @c true to accept the matching, @c false to deny the matching.
 *
 */
typedef bool (*ea_cutlink_match_cb)(const ea_cutlink_h cutlink,
				    const char *text,
				    int *start_offset,
				    int *end_offset,
				    const char *scheme,
				    void *userdata);

/**
 * @brief Trasform the matched string
 *
 * @details This callback function makes application can transform
 *          both the scheme name and the matched string.
 *
 * @remarks If @a match_str or @a scheme has been changed,
 *          the original buffer pointer must be released with
 *          free() by application. Also, if @a scheme is changed,
 *          The default anchor,clicked callback function is called
 *          based on scheme name. Therefore, if the scheme is changed,
 *          the defaut anchor,clicked callback function will not be called.
 *
 * @param [in] cutlink The cutlink object.
 * @param [in,out] match_str The string matched
 * @param [in,out] scheme The scheme name
 * @param [in] userdata The user data passed from ea_cutlink_trans_cb_set()
 *
 */
typedef void (*ea_cutlink_trans_cb)(const ea_cutlink_h cutlink,
				    char **match_str,
				    char **scheme,
				    void *userdata);

/**
 * @brief Create a new cutlink object.
 *
 * @details This function creates a new cutlink object.
 *
 * @param [out] cutlink A cutlink handle to be nerly created on success
 * @param [in] entry An entry object to view the result text
 * @param [in] mask The combination of cutlink type to find clickable links
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_create(ea_cutlink_h *cutlink,
				  Evas_Object *entry,
				  unsigned char mask);

/**
 * @brief Destroy the cutlink object.
 *
 * @details This function destroy the existing cutlink object that
 *         is created by ea_cutlink_create() function.
 *
 * @param [in] cutlink The cutlink object
 *
 */
void	ea_cutlink_destroy(ea_cutlink_h cutlink);

/**
 * @brief Add a new scheme
 *
 * @details This function adds a new user-defined scheme.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] pattern The regex pattern to be used for finding links.
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_scheme_add(ea_cutlink_h cutlink,
				      char *scheme,
				      char *pattern);
/**
 * @brief Delete a scheme
 *
 * @details This function deletes a scheme.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_scheme_del(ea_cutlink_h cutlink, char *scheme);

/**
 * @brief Set match function
 *
 * @details This function set match function.
 *          The match_func allow the user to determine the generation of links.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] match_func The user callback function is called
 *                        when pattern matches
 * @param [in] userdata The user data which will be delivered as a parameter
 *                      when user callback function is called.
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_match_cb_set(ea_cutlink_h cutlink,
					char *scheme,
					ea_cutlink_match_cb match_func,
					void *userdata);

/**
 * @brief Set transform function
 *
 * @details This function set transform function.
 *          The trans_func allow the user to change the matched string.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] trans_func The user callback function is called
 *                        when pattern matches
 * @param [in] userdata The user data which will be delivered as a parameter
 *                      when user callback function is called.
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_trans_cb_set(ea_cutlink_h cutlink,
					char *scheme,
					ea_cutlink_trans_cb trans_func,
					void *userdata);

/**
 * @brief enable or disable default action for anchor,clicked event
 *
 * @details This function enables or disables call the default anchor,click
 *          callback function. The default anchor,click callback function
 *          launches predefined services using application service.
 *          If the application registers anchor,click callback function,
 *          the application registered callback function is called
 *          prior to the default callback function. If the application
 *          do not want to the default callback function is called,
 *          then call this function with enable equals to false in
 *          application callback functions.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] enable Whether enable or disable default anchor,click
 *                    callback function
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_anchor_enable_set(ea_cutlink_h cutlink,
					     char *scheme,
					     bool enable);

/**
 * @brief Set the regex pattern string
 *
 * @details This function used to set the regex pattern string
 *          of the given scheme.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] pattern The pattern string
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_pattern_set(ea_cutlink_h cutlink,
				       char *scheme,
				       char *pattern);

/**
 * @brief Retrieves the regex pattern string
 *
 * @details This function used to get the regex pattern string
 *          of the given scheme.
 *
 * @remarks The @a pattern must be released with free() by application.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [out] pattern The pattern string
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_pattern_get(ea_cutlink_h cutlink,
				       char *scheme,
				       char **pattern);

/**
 * @brief Set a text to be parsed
 *
 * @details This function set text to be parsed. The text should be
 *          markuped using elm_entry_utf8_to_markup() function.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] markup The markup text
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_markup_set(ea_cutlink_h cutlink,
				      const char *markup);

/**
 * @brief Get a parsed text
 *
 * @remarks The @a markup must be released with free() by application.
 *
 * @details This function used to get a parsed text. The parsed text is
 *          a markup text. If you want to remove markup, then call
 *          elm_entry_markup_to_utf8() function.
 *
 * @param [in] cutlink The cutlink object
 * @param [out] markup The markup text
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_markup_get(ea_cutlink_h cutlink, char **markup);


/**
 * @brief Apply to parse text
 *
 * @details This function apply to parse text. The text should be
 *          markuped using elm_entry_utf8_to_markup() function.
 *          This function does not set parsed text to the entry.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] markup The markup text
 * @param [out] tagged The tagged text
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_markup_apply(ea_cutlink_h cutlink,
				      const char *markup, char **tagged);



/**
 * @brief Set a file to be parsed
 *
 * @details This function read a given file, then parse it.
 *          If file is a plain text format, it will be changed to
 *          markup text internally.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] file The file name
 * @param [in] format The text format (plain/markup)
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_file_set(ea_cutlink_h cutlink,
				    const char *file,
				    Elm_Text_Format format);

/**
 * @brief Get a file
 *
 * @details This function is used to get a file name and format
 *          set by user previously.
 *
 * @remarks The @a file must be released with free() by application.
 *
 * @param [in] cutlink The cutlink object
 * @param [out] file The pointer to store the file name
 * @param [out] format The pointer to store the text format (plain/markup)
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_file_get(ea_cutlink_h cutlink,
				    const char **file,
				    Elm_Text_Format *format);

/**
 * @brief Set a prefix
 *
 * @details This function set a prefix string in the given scheme name.
 *          The prefix string is appended before the anchor.
 *          It can be used to change the clickable link style.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] prefix The prefix string
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_prefix_set(ea_cutlink_h cutlink,
				      char *scheme,
				      char *prefix);

/**
 * @brief Get a prefix
 *
 * @details This function returns a prefix string in given scheme name.
 *
 * @remarks The @a prefix must be released with free() by application.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [out] prefix The prefix string
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_prefix_get(ea_cutlink_h cutlink,
				      char *scheme,
				      char **prefix);

/**
 * @brief Set a postfix
 *
 * @details This function set a postfix string in the given scheme name.
 *          The postfix string is appended after the anchor.
 *          It can be used to change the clickable link style.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [in] postfix The postfix string
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_postfix_set(ea_cutlink_h cutlink,
				       char *scheme,
				       char *postfix);

/**
 * @brief Get a postfix
 *
 * @details This function returns a postfix string in given scheme name.
 *
 * @remarks The @a postfix must be released with free() by application.
 *
 * @param [in] cutlink The cutlink object
 * @param [in] scheme The scheme name
 * @param [out] postfix The postfix string
 *
 * @return 0 on success, otherwise a negetive error value.
 *
 */
int	ea_cutlink_postfix_get(ea_cutlink_h cutlink,
				       char *scheme,
				       char **postfix);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_CUTLINK_H__ */
