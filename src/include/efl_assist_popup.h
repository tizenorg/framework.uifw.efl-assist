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

#ifndef __EFL_ASSIST_POPUP_H__
#define __EFL_ASSIST_POPUP_H__

#include <Elementary.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @internal
 * @defgroup EFL_Assist_Popup_Group Efl Assist Popup
 * @ingroup EFL_Assist_Group
 * @brief This group provides functionalities to enhance the popup widget.
 *
 * A Center popup and a menu popup are combinations of widgets used to support
 * multi-window feature. Multi-window is a concept that an device can show
 * more than one application in the screen. When Multi-window feature is enabled,
 * the top window of each application can be resized. And this means that
 * it cannot have full screen size. However, there are some cases popup
 * or ctxpopup should be out of the top window. Therefore, to make this
 * feature enable, center popup and menu popup concept come
 * which make window before popup or ctxpopup is made.
 *
 * For a center popup object, the parent window has the same size as the popup
 * because objects behind the popup can handle mouse events.
 * Also, the parent window can move anywhere user wants.
 *
 * For a menu popup object, the parent window has full size. Before adding
 * a ctxpopup, full size window is made since menu popup should be in the
 * top layer and the menu popup gets all mouse events according to the concept.
 *
 * When a center popup and menu popup are adding, the action of their
 * parent window has to be similar with them. So they have some callbacks for their parent.
 *
 * @{
 */

/**
 * @brief Add a new Center popup object to the parent.
 *
 * @param parent The parent object
 * @return The new object or @c NULL, if it cannot be created.
 *
 * A center popup has a window object as its parent internally.
 * So the param "parent" is not a real parent of the center popup.
 * However, user doesn't have to know the existence usually
 * except for the case user wants to do something with the parent window.
 */
EAPI Evas_Object *               ea_center_popup_add(Evas_Object *parent);

/**
 * @brief Get the window object which a center popup has internally.
 *
 * @param popup The center popup object
 * @return The window object or @c NULL, if center popup wasn't created.
 *
 * @see ea_center_popup_add()
 */
EAPI Evas_Object *               ea_center_popup_win_get(Evas_Object *popup);

/**
 * @brief Add a new Menu popup object to the parent.
 *
 * @param parent The parent object
 * @return The new object or @c NULL, if it cannot be created.
 *
 * A menu popup has a window object as its parent internally.
 * So the param "parent" is not a real parent of menu popup.
 * However, user doesn't have to know the existence usually
 * except for the case user wants to do something with the parent window.
 */
EAPI Evas_Object *               ea_menu_popup_add(Evas_Object *parent);

/**
 * @brief Get the window object which a menu popup has internally.
 *
 * @param obj The menu popup object
 * @return The window object or @c NULL, if menu popup wasn't created.
 *
 * @see ea_menu_popup_add()
 */
EAPI Evas_Object *               ea_menu_popup_win_get(Evas_Object *obj);

/**
 * @brief Move a Menu popup object automatically near hardware menu button
 *
 * @param obj The menu popup object
 *
 * The position menu popup should be is already decided and it's near the
 * hardware menu button. It depends on orientation of application's top window.
 * This API helps users not to think about the case for position of the menu popup.
 *
 * see ea_menu_popup_add()
 */
EAPI void                       ea_menu_popup_move(Evas_Object *obj);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_POPUP_H__ */
