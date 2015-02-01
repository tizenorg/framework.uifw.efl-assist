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

#ifndef __EFL_ASSIST_THEME_H__
#define __EFL_ASSIST_THEME_H__

#include <Elementary.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @internal
 * @defgroup EFL_Assist_Theme_Group Efl Assist Theme
 * @ingroup EFL_Assist_Group
 * @brief This group provides functionalities to support color theme for
 *        the EFL.
 *
 * @{
 */

/**
 * @brief Enueration of Color theme style.
 */
typedef enum
{
   EA_THEME_STYLE_DEFAULT,
   EA_THEME_STYLE_DARK,
   EA_THEME_STYLE_LIGHT,
} Ea_Theme_Style;

/**
 * @brief Enueration of Event callback type.
 */
typedef enum
{
   EA_THEME_CALLBACK_TYPE_COLOR,
   EA_THEME_CALLBACK_TYPE_FONT,
} Ea_Theme_Callback_Type;

/**
 * @brief Enueration of Color table index.
 */
typedef enum
{
   EA_THEME_COLOR_TABLE_1 = 0,
   EA_THEME_COLOR_TABLE_2 = 100,
   EA_THEME_COLOR_TABLE_3 = 200
} Ea_Theme_Color_Table_Index;

/**
 * @brief Theme color HSV value.
 */
typedef struct _Ea_Theme_Color_hsv {
   int h;
   int s;
   int v;
} Ea_Theme_Color_hsv;

typedef Eina_List Ea_Theme_Color_Table;
typedef Eina_List Ea_Theme_Font_Table;

typedef void (*Ea_Theme_Event_Cb)(void *data);


//////////// color table ///////////////

/**
 * @brief Open the application's color table and load the data.
 *
 * @remarks The color table should be described as the suitable format of xml.
 *
 * @code
 * <?xml version="1.0" encoding="UTF-8"?>
 *<ChangeableColorTables>
 *  <ChangeableColorTable num="1">
 *    <Theme style="Dark">
 *      <ChangeableColorInfo id="B011" inputColor="2" hue="1" saturation="-1" value="7" alpha="100" fixedHue="false" fixedSaturation="false" fixedValue="false" minHue="0" minSaturation="0" minValue="0" maxHue="360" maxSaturation="100" maxValue="100"/>
 *    </Theme>
 * </ChangeableColorTable>
 *</ChangeableColorTables>
 * @endcode
 *
 * @param [in] file The file path of the applitation's color table
 *
 * @return The color table data. If it is failed, return NULL.
 *
 * @see ea_theme_color_table_free().
 */
EAPI Ea_Theme_Color_Table *ea_theme_color_table_new(const char *file);

/**
 * @brief Free the color table data.
 *
 * @param [in] table The color table
 *
 * @see ea_theme_color_table_new()
 */
EAPI void ea_theme_color_table_free(Ea_Theme_Color_Table *table);


///////////// colors for the application /////////////

/**
 * @brief Set changeable color features.
 *
 * @details If enabled, when system color theme is changed, colors for controls are changed.
 *          The default value is false.
 *
 * @param [in] enabled the changeable state
 */
EAPI void ea_theme_changeable_ui_enabled_set(Eina_Bool enabled);

/**
 * @brief Get whether the application enable changeable color feature
 *
 * @return The changeable color state
 */
EAPI Eina_Bool ea_theme_changeable_ui_enabled_get(void);

/**
 * @brief Set the application's changeable colors described in the color table.
 *
 * @param [in] table The color table
 * @param [in] style The theme style
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @see ea_theme_color_table_new()
 */
EAPI Eina_Bool ea_theme_colors_set(Ea_Theme_Color_Table *table, Ea_Theme_Style style);

/**
 * @brief Unset the application's changeable colors described in the color table.
 *
 * @remarks Application is sometimes needed to cancel colors and free memory
 *       for the color information. When the colors are freed, default
 *       color values are white.
 *
 * @param [in] table The color table
 * @param [in] style The theme style
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @see ea_theme_colors_set()
 */
EAPI Eina_Bool ea_theme_colors_unset(Ea_Theme_Color_Table *table, Ea_Theme_Style style);

/**
 * @brief Get the current color value.
 *
 * @details The first color is the normal color, the second is the text outline,
 *          and the third is the text shadow. (Note that the second two only apply
 *          to text parts).
 *
 * @param [in] code The color code
 * @param [out] r Red value
 * @param [out] g Green value
 * @param [out] b Blue value
 * @param [out] a Alpha value
 * @param [out] r2 Outline red value
 * @param [out] g2 Outline green value
 * @param [out] b2 Outline blue value
 * @param [out] a2 Outline alpha value
 * @param [out] r3 Shadow red value
 * @param [out] g3 Shadow green value
 * @param [out] b3 Shadow blue value
 * @param [out] a3 Shadow alpha value
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_color_get(const char *code,
                             int *r, int *g, int *b, int *a,
                             int *r2, int *g2, int *b2, int *a2,
                             int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Set the default theme style.
 *
 * @details If application wants to change default theme style, use this function.
 *          It only affects the application's process.
 *
 * @param [in] style Theme style
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_style_set(Ea_Theme_Style style);

/**
 * @brief Get the default theme style in the application.
 *
 * @remarks Call this function after elm_window is created.
 *
 * @return The theme style
 */
EAPI Ea_Theme_Style ea_theme_style_get(void);

/**
 * @brief Set the fixed colors to winset components among system themes
 *
 * @remarks If application wants to change winset colors to the fixed colors,
 *          use this function. It doesn't follow system themes.
 *          It only affects the application's process.
 *          Call this function after elm_window is created.
 *
 * @param [in] num The theme number
 * @param [in] style Theme style
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_fixed_style_set(int num, Ea_Theme_Style style);


///////////// colors for the base object ////////////

/**
 * @brief Set changeable color features.
 *
 * @details If enabled, when system color theme is changed, colors for controls are changed.
 *          The default value is false.
 *
 * @remarks It affects colors to the child objects.
 *          Recommend to call this function to the base layout object of the application
 *
 * @param [in] obj The object
 * @param [in] enabled the changeable state
 */
EAPI void ea_theme_object_changeable_ui_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * @brief Set the application's changeable colors described in the color table
 *        to the object.
 *
 * @remarks It affects colors to the child objects.
 *          Recommend to call this function to the base layout object of the
 *          application
 *
 * @param [in] obj The object
 * @param [in] table The color table
 * @param [in] style The theme style
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @see ea_theme_color_table_new()
 */
EAPI Eina_Bool ea_theme_object_colors_set(Evas_Object *obj, Ea_Theme_Color_Table *table, Ea_Theme_Style style);

/**
 * @brief Set the theme style to the object
 *
 * @remarks It affects colors to the child objects.
 *          Recommend to call this function to the base layout object of the
 *          application
 *
 * @param [in] obj The object
 * @param [in] style Theme style
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_style_set(Evas_Object *obj, Ea_Theme_Style style);

/**
 * @brief Set the fixed colors to winset components among system themes
 *
 * @remarks It affects colors to the child objects.
 *          Recommend to call this function to the base layout object of the
 *          application
 *
 * @param [in] obj The object
 * @param [in] num The theme number
 * @param [in] style Theme style
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_fixed_style_set(Evas_Object *obj, int num, Ea_Theme_Style style);


/////////// colors for the specific object ////////////

/**
 * @brief Set the current color to the object
 *
 * @remarks This function calls evas_object_color_set() to the object.
 *
 * @param [in] obj The given Evas_Object
 * @param [in] code The color code
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_color_set(Evas_Object *obj, const char *code);

/**
 * @brief Replace the color with the new color of the code in the object
 *
 * @remarks This function is for customizing object's color without changing its
 *          layout. It should be called after all color codes are set.
 *
 * @param [in] obj The given Evas_Object
 * @param [in] code The color code of the object
 * @param [in] new_code The new color code
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_color_replace(Evas_Object *obj, const char *code, const char *new_code);

/**
 * @brief Replace the color with the new color of the code in the object item
 *
 * @remarks This function is for customizing object item's color without
 *          changing its layout. It should be called after all color code are
 *          set.
 *
 * @param [in] item The given Elm_Object_Item
 * @param [in] code The color code of the object item
 * @param [in] new_code The new color code
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_item_color_replace(Elm_Object_Item *item, const char *code, const char *new_code);

/**
 * @brief Reload colors of the objects when the system theme color is changed
 *
 * @remarks This function is for applying theme colors again to the objects
 *
 * @see ea_theme_object_color_set()
 * @see ea_theme_object_color_replace()
 * @see ea_theme_object_item_color_replace()
 */
EAPI void ea_theme_object_colors_reload(void);

/**
 * @brief Get the current color value of the object.
 *
 * @remarks The first color is the normal color, the second is the text
 *          outline, and the third is the text shadow. (Note that the second 
 *          two only apply to text parts).
 *
 * @param [in] obj The object
 * @param [in] code The color code
 * @param [out] r Red value
 * @param [out] g Green value
 * @param [out] b Blue value
 * @param [out] a Alpha value
 * @param [out] r2 Outline red value
 * @param [out] g2 Outline green value
 * @param [out] b2 Outline blue value
 * @param [out] a2 Outline alpha value
 * @param [out] r3 Shadow red value
 * @param [out] g3 Shadow green value
 * @param [out] b3 Shadow blue value
 * @param [out] a3 Shadow alpha value
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_color_get(Evas_Object *obj, const char *code,
                                         int *r, int *g, int *b, int *a,
                                         int *r2, int *g2, int *b2, int *a2,
                                         int *r3, int *g3, int *b3, int *a3);


/////////// colors for the system //////////

/**
 * @brief Apply the color changes for system components (widgets).
 *
 * @remarks This is for changing current theme.
 *
 * @note Don't use this function in other applications except for setting.
 */
EAPI void ea_theme_system_colors_apply(void);

/**
 * @brief Set the input colors and style of the theme.
 *
 * @remarks This is for changing current theme.
 *
 * @note Don't use this function in other applications except for setting.
 *
 * @param [in] num The theme number
 * @param [in] style Theme style
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @see ea_theme_input_colors_get()
 */
EAPI Eina_Bool ea_theme_input_colors_set(int num, Ea_Theme_Style style);

/**
 * @brief Get the input colors of the theme.
 *
 * @note The list and data should be released after using.
 *
 * @param [in] num The theme number
 *
 * @return The list of input colors, with @c Ea_Theme_Color_hsv blobs as data
 *
 * @see ea_theme_input_colors_set();
 */
EAPI Eina_List *ea_theme_input_colors_get(int num);

/**
 * @brief Change theme colors
 *
 * @remarks This is for changing current theme.
 *
 * @param [in] color The input color
 * @param [in] style Theme style
 *
 * @note Don't use this function in other applications except for setting.
 */
EAPI Eina_Bool ea_theme_system_color_set(Ea_Theme_Color_hsv color, Ea_Theme_Style style);

/**
 * @brief Get current theme color
 *
 * @param [out] color The input color
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @note The color should be released after using.
 */
EAPI Eina_Bool ea_theme_system_color_get(Ea_Theme_Color_hsv *color);

/**
 * @brief Get the closest input color number of the theme compared to r, g, b value
 *
 * @param [in] r Red value
 * @param [in] g Green value
 * @param [in] b Blue value
 *
 * @return The theme number
 *
 * @see ea_theme_input_colors_get()
 */
EAPI int ea_theme_suitable_theme_get(int r, int g, int b);

/**
 * @brief Get the closest input color number of the theme by analyzing the image file.
 *
 * @param [in] path Image file name
 *
 * @return The theme number
 *
 * @see ea_theme_input_colors_get()
 */
EAPI int ea_theme_suitable_theme_get_from_image(const char *path);


///////////// font table //////////////

/**
 * @brief Open the application's font table and load the data.
 *
 * @remarks The font table should be described as the suitable format of xml.
 *
 * @code
 *<?xml version="1.0" encoding="UTF-8"?>
 *<FontInfoTable>
 *   <FontInfo id="T011" typeface="SamsungSans" style="R" size="38"/>
 *</FontInfoTable>
 * @endcode
 *
 * @param [in] file The file path of the applitation's font table
 *
 * @return The list of font data
 *
 * @see ea_theme_font_table_free().
 */
EAPI Ea_Theme_Font_Table *ea_theme_font_table_new(const char *file);

/**
 * @brief Free the font table list.
 *
 * @param [in] table The font table
 *
 * @see ea_theme_font_table_new()
 */
EAPI void ea_theme_font_table_free(Ea_Theme_Font_Table *table);


/////////// fonts for the application //////////

/**
 * @brief Set the application's font data described in the font list.
 *
 * @param [in] table The font table
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @see ea_theme_font_table_new()
 */
EAPI Eina_Bool ea_theme_fonts_set(Ea_Theme_Font_Table *table);

/**
 * @brief Unset the application's font data described in the color table.
 *
 * @remarks Application is sometimes needed to cancel fonts and free memory
 *          for the font information.
 *
 * @param [in] table The color table
 *
 * @return The return value that indicates this function works properly or not.
 *
 * @see ea_theme_font_table_set()
 */
EAPI Eina_Bool ea_theme_fonts_unset(Ea_Theme_Font_Table *table);

/**
 * @brief Get the current font value.
 *
 * @param [in] code The font code
 * @param [out] font Font name and style string. It should be freed after using.
 * @param [out] size Font size
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_font_get(const char *code, char **font, int *size);


////////// fonts for the base object /////////

/**
 * @brief Set the application's font data described in the font list to the
 *        object.
 *
 * @remarks It affects colors to the child objects.
 *          Recommend to call this function to the base layout object of the
 *          application
 *
 * @param [in] obj The object
 * @param [in] table The font table
 *
 * @see ea_theme_font_table_new()
 */
EAPI Eina_Bool ea_theme_object_fonts_set(Evas_Object *obj, Ea_Theme_Font_Table *table);


//////////// fonts for the specific object //////////

/**
 * @brief Replace the font with the new font of the code in the object
 *
 * @remarks This function is for customizing object's font without changing
 *          its layout
 *          It should be called after all font codes are set.
 *
 * @param [in] obj The given Evas_Object
 * @param [in] code The font code of the object
 * @param [in] new_code The new font code
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_font_replace(Evas_Object *obj, const char *code, const char *new_code);

/**
 * @brief Replace the font with the new font of the code in the object item
 *
 * @remarks This function is for customizing object item's font without
 *          changing its layout
 *          It should be called after all font code are set.
 *
 * @param [in] item The given Elm_Object_Item
 * @param [in] code The font code of the object item
 * @param [in] new_code The new font code
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_item_font_replace(Elm_Object_Item *item, const char *code, const char *new_code);

/**
 * @brief Reload fonts of the objects when the system font is changed.
 *
 * @remarks This function is for applying fonts again to the objects.
 *
 * @see ea_theme_object_font_replace()
 * @see ea_theme_object_item_font_replace()
 */
EAPI void ea_theme_object_fonts_reload(void);

/**
 * @brief Get the current font value of the object.
 *
 * @param [in] obj The object
 * @param [in] code The font code
 * @param [out] font Font name and style string. It should be freed after using.
 * @param [out] size Font size
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_object_font_get(Evas_Object *obj, const char *code, char **font, int *size);


////////// fonts for the system ///////////

/**
 * @brief Apply the font name and size for system components (widgets).
 *
 * @note Don't use this function in other applications except for setting.
 */
EAPI void ea_theme_system_fonts_apply(void);

/**
 * @brief Change default font and set size percentage for accessibility.
 *
 * @param [in] font Font name and style string.
 * @param [in] size Font size
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_system_font_set(const char *font, int size);

/**
 * @brief Get default font and size percentage for accessibility.
 *
 * @param [out] font Font name and style string. It should be freed after using.
 * @param [out] size Font size
 *
 * @return The return value that indicates this function works properly or not.
 */
EAPI Eina_Bool ea_theme_system_font_get(char **font, int *size);


//////////// callback for system color/font change /////////

/**
 * @brief Add (register) a callback function for the event type
 *
 * @remarks The color event is triggerd when system theme color is changed.
 *          Them font event is triggerd when system font type or size is changed.
 *
 * @param[in] type The type of event that will trigger the callback.
 * @param[in] func The function to be called when the event is triggered.
 * @param[in] data The data pointer to be passed to @p func.
 */
EAPI void ea_theme_event_callback_add(Ea_Theme_Callback_Type type, Ea_Theme_Event_Cb func, void *data);

/**
 * @brief Delete a callback function for the event type
 *
 * @param[in] type The type of event that will trigger the callback.
 * @param[in] func The function to be called when the event is triggered.
 *
 * @return The data pointer that was passed to the callback
 */
EAPI void *ea_theme_event_callback_del(Ea_Theme_Callback_Type type, Ea_Theme_Event_Cb func);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_THEME_H__ */
