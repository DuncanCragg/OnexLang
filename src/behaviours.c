#include <string.h>
#include <onex-kernel/log.h>
#include <items.h>
#include <onf.h>
#include <onr.h>

bool evaluate_light_logic(object* o, void* d)
{
  bool buttonpressed=object_property_is(o, "button:state", "down");
  char* s=(char*)(buttonpressed? "on": "off");
  object_property_set(o, "light", s);
  return true;
}

void apply_update(object* o, properties* update)
{
  uint16_t n=properties_size(update);
  for(int i=1; i<=n; i++){
    value* key=properties_key_n(update, i);
    item* upd=properties_get_n(update, i);
    if(item_is_type(upd,ITEM_LIST)){
      list* li=(list*)upd;
      uint16_t s=list_size(li);
      uint16_t arrow=list_find(li, (item*)value_new("=>"));
      uint16_t arrowdel=list_find(li, (item*)value_new("(=>)"));
      if(arrow){
        if(arrow+1>s){
          object_property_set(o, value_string(key), 0);
          continue;
        }
        if(arrow+1==s){
          item* r = list_get_n(li, arrow+1);
          if(item_is_type(r, ITEM_VALUE)){
            object_property_set(o, value_string(key), value_string((value*)r));
          }
          continue;
        }
        item* r1 = list_get_n(li, arrow+1);
        item* r2 = list_get_n(li, arrow+2);
        if(item_is_type(r1, ITEM_VALUE) && item_is_type(r2, ITEM_VALUE)){
          if(item_equal(r1, value_new("@."))){
            object_property_add(o, value_string(key), value_string((value*)r2));
          }
        }
      }
      if(arrowdel){
        char delkey[128]; snprintf(delkey, 128, "%s:%d", value_string(key), arrowdel);
        object_property_set(o, delkey, 0);
      }
    }
  }
  properties_free(update, true);
}

bool evaluate_object_setter(object* o, void* d)
{
  if(d) apply_update(o, (properties*)d);
  return true;
}

void apply_edit(object* o)
{
  uint16_t n=object_property_size(o, "Alerted:");
  for(int i=1; i<=n; i++){
    char* key=object_property_key(o, "Alerted:", i);
    if(!strcmp(key, "is")) continue;
    char pathkey[128]; snprintf(pathkey, 128, "Alerted:%s", key);
    uint16_t ln = object_property_length(o, pathkey);
    int arrowindex=0;
    int atdotindex=0;
    int j=1;
    for(; j<=ln; j++){
      char* val=object_property_get_n(o, pathkey, j);
      if(!arrowindex){
        if(!strcmp(val, "=>")){
          arrowindex=j;
        }
        if(!strcmp(val, "(=>)")){
          char delkey[128]; snprintf(delkey, 128, "%s:%d", key, j);
          object_property_set(o, delkey, 0);
          return;
        }
        continue;
      }
      else{
        if(!atdotindex){
          if(!strcmp(val, "@.")){
            if(j==arrowindex+1) atdotindex=j;
            continue;
          }
          object_property_set(o, key, val);
          atdotindex=j;
        }
        else object_property_add(o, key, val);
      }
    }
    if(arrowindex==ln){
      object_property_set(o, key, 0);
    }
  }
}

bool evaluate_edit_rule(object* o, void* d)
{
  if(object_property_contains(o, (char*)"Alerted:is", (char*)"edit") &&
     object_property_contains(o, (char*)"Alerted:is", (char*)"rule")   ){

    apply_edit(o);
  }
  return true;
}

bool evaluate_device_logic(object* o, void* d)
{
  if(object_property_contains(o, (char*)"Alerted:is", (char*)"device")){
    char* devuid=object_property(o, (char*)"Alerted");
    if(!object_property_contains(o, (char*)"connected-devices", devuid)){
      object_property_add(o, (char*)"connected-devices", devuid);
    }
  }
  return true;
}

