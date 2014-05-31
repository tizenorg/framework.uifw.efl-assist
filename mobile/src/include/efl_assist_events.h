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

#ifndef __EFL_ASSIST_EVENTS_H__
#define __EFL_ASSIST_EVENTS_H__

#include <Elementary.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convenient macro function that sends back key events to the popup
 *        to be removed.
 *
 * @see   ea_object_event_callback_add()
 */
static inline void
ea_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(obj);
}

/**
 * @brief Convenient macro function that sends back key events to the ctxpopup
 *        to be dismissed.
 *
 * @see   ea_object_event_callback_add()
 */
static inline void
ea_ctxpopup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_ctxpopup_dismiss(obj);
}

/**
 * @brief Convenient macro function that sends more key events to the naviframe
 *        top item.
 * @see   ea_object_event_callback_add()
 */
static inline void
ea_naviframe_more_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *top = elm_naviframe_top_item_get(obj);
   if (!top) return;
   Evas_Object *more_btn = elm_object_item_part_content_get(top, "toolbar_more_btn");
   if (more_btn)
     evas_object_smart_callback_call(more_btn, "clicked", NULL);
   else
     elm_object_item_signal_emit(top, "elm,action,more_event", "");
}

/**
 * @brief Convenient macro function that pop the naviframe item.
 * @see   ea_object_event_callback_add()
 */
static inline void
ea_naviframe_back_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_naviframe_item_pop(obj);
}

/**
 * Identifier of callbacks to be set for Ea events.
 *
 * @see ea_object_event_callback_add()
 * @see ea_object_event_callback_del()
 */
typedef enum _Ea_Callback_Type
{
   EA_CALLBACK_BACK,
   EA_CALLBACK_MORE,
   EA_CALLBACK_LAST
} Ea_Callback_Type;

/** <Ea event callback function signature */
typedef void (*Ea_Event_Cb)(void *data, Evas_Object *obj, void *event_info);

/**
 * Delete a callback function from an object.
 *
 * @param[in] obj Object to remove a callback from.
 * @param[in] type The type of event that was triggering the callback.
 * @param[in] func The function that was to be called when the event was
 *            triggered
 * @return    data The data pointer that was to be passed to the callback.
 *
 * @brief     This function removes the most recently added callback from the
 *            object @p obj which was triggered by the type @p type and
 *            was calling the function @p func when triggered. If the removal is
 *            successful it will also return the data pointer that was passed to
 *            ea_object_event_callback_add() when the callback was added to
 *            the object. If not successful @c NULl will be returned.
 *
 * @see ea_object_event_callback_add()
 */

EAPI void *ea_object_event_callback_del(Evas_Object *obj, Ea_Callback_Type type, Ea_Event_Cb);

/**
 * Add (register) a callback function to a given evas object.
 *
 * @param[in] obj evas object.
 * @param[in] type The type of event that will trigger the callback.
 * @param[in] func The function to be called when the key event is triggered.
 * @param[in] data The data pointer to be passed to @p func.
 *
 * @brief This function adds a function callback to an object when the key event
 *        occurs on object @p obj. The key event on the object is only triggered
 *        when the object is the most top in objects stack and visible. This
 *        means, like the naviframe widget, if your application needs to have
 *        the events based on the view but not focus, you can use this callback.
 *
 *        A callback function must have the Ea_Event_Cb prototype definition.
 *        The first parameter (@p data) in this definition will have the same
 *        value passed to ea_object_event_callback_add() as the @p data
 *        parameter, at runtime. The second parameter @p obj is the evas object
 *        on which event occurred. Finally, the third parameter @p event_info is
 *        a pointer to a data structure that may or may not be passed to the
 *        callback, depending on the event type that triggered the callback.
 *        This is so because some events don't carry extra context with them,
 *        but others do.
 *
 * @see ea_object_event_callback_del()
 */

EAPI void ea_object_event_callback_add(Evas_Object *obj, Ea_Callback_Type type, Ea_Event_Cb func, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __EFL_ASSIST_EVENTS_H__ */
