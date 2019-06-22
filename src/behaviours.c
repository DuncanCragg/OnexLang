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
      if(!arrow) continue;
      if(arrow+1>s) continue;
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
  }
}

bool evaluate_object_setter(object* o, void* d)
{
  apply_update(o, (properties*)d);
  return true;
}

