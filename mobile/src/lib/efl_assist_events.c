#include "efl_assist.h"
#include "efl_assist_private.h"

typedef struct _Ea_Event_Mgr Ea_Event_Mgr;

struct _Ea_Event_Mgr
{
   Eina_List *obj_events;
   Evas *e;
   Evas_Object *key_grab_rect;
};

typedef struct _Ea_Object_Event
{
   Evas_Object *obj;
   Evas_Object *parent;
   Eina_List *callbacks;
   Eina_Bool delete_me : 1;
   Eina_Bool on_callback : 1;
} Ea_Object_Event;

typedef struct _Ea_Event_Callback
{
   Ea_Callback_Type type;
   void (*func)(void *data, Evas_Object *obj, void *event_info);
   void *data;
} Ea_Event_Callback;

const char *EA_OBJ_KEY_EVENT_MGR = "_ea_obj_key_event_mgr";
const char *EA_OBJ_KEY_OBJ_EVENT = "_ea_obj_key_obj_event";
const char *EA_KEY_STOP = "XF86Stop";
const char *EA_KEY_STOP2 = "Escape";
const char *EA_KEY_SEND = "XF86Send";
const char *EA_KEY_SEND2 = "Menu";


static Eina_List *event_mgrs = NULL;

static void
_ea_event_mgr_del(Ea_Event_Mgr *event_mgr)
{
   if (event_mgr->obj_events) return;

   //Redundant Event Mgr. Remove it.
   evas_object_del(event_mgr->key_grab_rect);
   event_mgrs = eina_list_remove(event_mgrs, event_mgr);
   free(event_mgr);
}

static void
_ea_object_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ea_Event_Mgr *event_mgr = evas_object_data_get(obj, EA_OBJ_KEY_EVENT_MGR);
   Ea_Object_Event *obj_event = data;
   Eina_List *l;
   Ea_Event_Callback *callback;

   l = eina_list_data_find_list(event_mgr->obj_events, obj_event);
   if (!l) return;

   event_mgr->obj_events = eina_list_remove_list(event_mgr->obj_events, l);

   EINA_LIST_FOREACH(obj_event->callbacks, l, callback)
      free(callback);
   obj_event->callbacks = eina_list_free(obj_event->callbacks);

   if (obj_event->on_callback) obj_event->delete_me = EINA_TRUE;
   else free(obj_event);

   _ea_event_mgr_del(event_mgr);
}

static int
_ea_layer_sort_cb(const void *data1, const void *data2)
{
   const Ea_Object_Event *obj_event = data1;
   const Ea_Object_Event *obj_event2 = data2;

   //1. Layer Compare
   int layer1 = evas_object_layer_get(obj_event->obj);
   int layer2 = evas_object_layer_get(obj_event2->obj);

   return (layer1 < layer2) ? -1 : 1;
}

static Evas_Object *
_ea_top_parent_candidates(Eina_List **candidates, Evas *e)
{
   Evas_Object *temp;
   Evas_Object *parent;
   Eina_List *l, *l_next;
   Ea_Object_Event *obj_event;
   Eina_List *_candidates = *candidates;
   Eina_Bool found = EINA_FALSE;

   //Get the top parent lists
   EINA_LIST_FOREACH(_candidates, l, obj_event)
     {
        temp = obj_event->obj;
        parent = obj_event->obj;
        while (temp)
          {
             parent = temp;
             temp = evas_object_smart_parent_get(temp);
          }
        obj_event->parent = parent;
     }

   //Leave only parent candidates.
   parent = evas_object_top_get(e);

   while (parent)
     {
        EINA_LIST_FOREACH(_candidates, l, obj_event)
          {
             if (parent == obj_event->parent)
               {
                  found = EINA_TRUE;
                  obj_event->parent = NULL;
               }
          }
        if (found) break;
        parent = evas_object_below_get(parent);
     }

   //Redundant parents (no candidates)
   EINA_LIST_FOREACH_SAFE(_candidates, l, l_next, obj_event)
     {
        if (!obj_event->parent) continue;
        _candidates = eina_list_remove_list(_candidates, l);
     }

   *candidates = _candidates;
   return parent;
}

static Ea_Object_Event *
_ea_find_event_target(Eina_List *candidates, Evas_Object *parent)
{
   Ea_Object_Event *obj_event = NULL;
   Eina_List *l;
   Evas_Object *obj;
   Eina_List *members = evas_object_smart_members_get(parent);
   if (members)
     {
        EINA_LIST_REVERSE_FOREACH(members, l, obj)
          {
             obj_event = _ea_find_event_target(candidates, obj);
             //got you!
             if (obj_event)
               {
                  eina_list_free(members);
                  return obj_event;
               }
          }
        eina_list_free(members);
     }

   EINA_LIST_REVERSE_FOREACH(candidates, l, obj_event)
     {
        //got you!
        if (parent == obj_event->obj) return obj_event;
     }

   return NULL;
}

static Ea_Object_Event *
_ea_top_obj_event_find(Ea_Event_Mgr *event_mgr)
{
   Ea_Object_Event *obj_event = NULL;
   Eina_List *l, *l_next;
   Eina_List *candidates = NULL;
   Evas_Object *parent;
   int top_layer;
   Eina_Bool invisible;

   //1. filter the invisible objs
   EINA_LIST_FOREACH(event_mgr->obj_events, l, obj_event)
     {
        parent = obj_event->obj;
        invisible = EINA_FALSE;

        while (parent)
          {
             if (!evas_object_visible_get(parent))
               {
                  invisible = EINA_TRUE;
                  break;
               }
             parent = evas_object_smart_parent_get(parent);
          }
        if (invisible) continue;
        candidates = eina_list_append(candidates, obj_event);
     }
   if (!candidates) return NULL;   //no visible objects.
   if (eina_list_count(candidates) == 1) goto found;

   //2.1. sort by layer order
   candidates = eina_list_sort(candidates, eina_list_count(candidates),
                               _ea_layer_sort_cb);

   //2.2. leave the only top layer
   obj_event = eina_list_data_get(eina_list_last(candidates));
   top_layer = evas_object_layer_get(obj_event->obj);

   EINA_LIST_FOREACH_SAFE(candidates, l, l_next, obj_event)
     {
        if (evas_object_layer_get(obj_event->obj) < top_layer)
          candidates = eina_list_remove_list(candidates, l);
     }
   if (eina_list_count(candidates) == 1) goto found;

   //3. find the top parent candidate.
   parent = _ea_top_parent_candidates(&candidates, event_mgr->e);
   if (eina_list_count(candidates) == 1) goto found;

   //4. find the target in this parent tree.
   return _ea_find_event_target(candidates, parent);

found:
   obj_event = eina_list_data_get(candidates);
   eina_list_free(candidates);
   return obj_event;
}

static void
_ea_key_grab_rect_key_up_cb(void *data, Evas *e, Evas_Object *obj,
                            void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Ea_Event_Mgr *event_mgr = data;
   Ea_Object_Event *obj_event;
   Ea_Event_Callback *callback;
   Ea_Callback_Type type;
   Eina_List *l;

   obj_event = _ea_top_obj_event_find(event_mgr);
   if (!obj_event) return;

   if (!strcmp(ev->keyname, EA_KEY_STOP) || !strcmp(ev->keyname, EA_KEY_STOP2))
     type = EA_CALLBACK_BACK;
   else if (!strcmp(ev->keyname, EA_KEY_SEND) ||
            !strcmp(ev->keyname, EA_KEY_SEND2))
     type = EA_CALLBACK_MORE;
   else return;

   obj_event->on_callback = EINA_TRUE;
   EINA_LIST_FOREACH(obj_event->callbacks, l, callback)
     {
        if (obj_event->delete_me) break;
        if (callback->type != type) continue;
        callback->func(callback->data, obj_event->obj, (void*) type);
     }
   if (obj_event->delete_me) free(obj_event);
   else obj_event->on_callback = EINA_FALSE;
}

static void
_ea_key_grab_obj_create(Ea_Event_Mgr *event_mgr)
{
   Evas_Object *key_grab_rect = evas_object_rectangle_add(event_mgr->e);

   evas_object_event_callback_add(key_grab_rect, EVAS_CALLBACK_KEY_UP,
                                  _ea_key_grab_rect_key_up_cb, event_mgr);
   if (!evas_object_key_grab(key_grab_rect, EA_KEY_STOP, 0, 0, EINA_FALSE))
     LOGE("Failed to grab END KEY\n");

   if (!evas_object_key_grab(key_grab_rect, EA_KEY_STOP2, 0, 0, EINA_FALSE))
     LOGE("Failed to grab END KEY\n");

   if (!evas_object_key_grab(key_grab_rect, EA_KEY_SEND, 0, 0, EINA_FALSE))
     LOGE("Failed to grab MORE KEY\n");

   if (!evas_object_key_grab(key_grab_rect, EA_KEY_SEND2, 0, 0, EINA_FALSE))
     LOGE("Failed to grab MORE KEY\n");

   event_mgr->key_grab_rect = key_grab_rect;
}

static Ea_Event_Mgr *
_ea_event_mgr_new(Evas *e)
{
   Ea_Event_Mgr *event_mgr = calloc(1, sizeof(Ea_Event_Mgr));
   if (!event_mgr)
     {
        LOGE("Failed to allocate event manager");
        return NULL;
     }
   event_mgr->e = e;
   _ea_key_grab_obj_create(event_mgr);

   return event_mgr;
}

EAPI void *
ea_object_event_callback_del(Evas_Object *obj, Ea_Callback_Type type, Ea_Event_Cb func)
{
   Ea_Object_Event *obj_event;
   Ea_Event_Mgr *event_mgr;
   Eina_List *l;
   Ea_Event_Callback *callback;
   void *data;

   //Check the validation
   event_mgr = evas_object_data_get(obj, EA_OBJ_KEY_EVENT_MGR);
   obj_event = evas_object_data_get(obj, EA_OBJ_KEY_OBJ_EVENT);

   if (!event_mgr || !obj_event)
     {
        LOGW("This object(%p) hasn't been registered before", obj);
        return NULL;
     }

   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _ea_object_del_cb);

   //Remove the callback data
   EINA_LIST_REVERSE_FOREACH(obj_event->callbacks, l, callback)
     {
        if ((callback->func == func) && (callback->type == type)) break;
     }

   data = callback->data;
   obj_event->callbacks = eina_list_remove_list(obj_event->callbacks, l);
   free(callback);

   //This object is not managed anymore.
   if (!obj_event->callbacks)
     {
        evas_object_data_set(obj, EA_OBJ_KEY_OBJ_EVENT, NULL);
        evas_object_data_set(obj, EA_OBJ_KEY_EVENT_MGR, NULL);
        Eina_List *l = eina_list_data_find_list(event_mgr->obj_events,
                                                obj_event);
        if (l)
          event_mgr->obj_events = eina_list_remove_list(event_mgr->obj_events,
                                                        l);
        if (obj_event->on_callback) obj_event->delete_me = EINA_TRUE;
        else free(obj_event);
     }

   _ea_event_mgr_del(event_mgr);

   return data;
}

EAPI void
ea_object_event_callback_add(Evas_Object *obj, Ea_Callback_Type type, Ea_Event_Cb func, void *data)
{
   Ea_Event_Mgr *event_mgr;
   Evas *e;
   Ea_Object_Event *obj_event = NULL;
   Eina_List *l;
   Ea_Event_Callback *callback;
   Eina_Bool new_event_mgr = EINA_TRUE;

   //Check the registered event manager for this Evas.
   e = evas_object_evas_get(obj);

   EINA_LIST_FOREACH(event_mgrs, l, event_mgr)
     {
        if (event_mgr->e == e)
          {
             new_event_mgr = EINA_FALSE;
             break;
          }
     }

   //New Evas comes. Create new event manager for this Evas.
   if (new_event_mgr)
     {
        if (!(event_mgr = _ea_event_mgr_new(e))) return;
        event_mgrs = eina_list_append(event_mgrs, event_mgr);
     }

   obj_event = evas_object_data_get(obj, EA_OBJ_KEY_OBJ_EVENT);

   //New Object Event. Probably user adds ea_object_event_callback first time.
   if (!obj_event)
     {
        obj_event = calloc(1, sizeof(Ea_Object_Event));
        if (!obj_event)
          {
             LOGE("Failed to allocate object event");
             return;
          }
        evas_object_data_set(obj, EA_OBJ_KEY_OBJ_EVENT, obj_event);
        evas_object_data_set(obj, EA_OBJ_KEY_EVENT_MGR, event_mgr);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _ea_object_del_cb,
                                       obj_event);
        event_mgr->obj_events = eina_list_append(event_mgr->obj_events,
                                                 obj_event);
        obj_event->obj = obj;
     }

   //Append this callback.
   callback = calloc(1, sizeof(Ea_Event_Callback));
   if (!callback)
     {
        LOGE("Failed to allocate event callback");
        return;
     }
   callback->type = type;
   callback->func = func;
   callback->data = data;

   obj_event->callbacks = eina_list_append(obj_event->callbacks, callback);
}
