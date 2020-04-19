#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <items.h>
#include <onf.h>
#include <onr.h>

static bool discover_io_peer(object* o, char* property, char* is)
{
  char ispath[32]; snprintf(ispath, 32, "%s:is", property);

  if(object_property_contains(o, ispath, is)) return true;

  int ln=object_property_length(o, "device:connected-devices:io");
  for(int i=1; i<=ln; i++){
    char* uid=object_property_get_n(o, "device:connected-devices:io", i);
    if(!is_uid(uid)) continue;
    object_property_set(o, property, uid);

    if(object_property_contains_peek(o, ispath, is)) return true;
  }
  object_property_set(o, property, 0);
  return false;
}

bool evaluate_light_logic(object* o, void* d)
{
  if(!discover_io_peer(o, "button", "button")) return true;
  if(object_property_is(o, "button:state", "up"  )) object_property_set(o, "light", (char*)"off");
  if(object_property_is(o, "button:state", "down")) object_property_set(o, "light", (char*)"on");
  return true;
}

void apply_update(object* o, properties* update)
{
  uint16_t n=properties_size(update);
  for(int i=1; i<=n; i++){
    char* key=properties_key_n(update, i);
    item* upd=properties_get_n(update, i);
    if(item_is_type(upd,ITEM_LIST)){
      list* li=(list*)upd;
      uint16_t s=list_size(li);
      uint16_t arrow=list_find(li, (item*)value_new("=>"));
      uint16_t arrowdel=list_find(li, (item*)value_new("(=>)"));
      if(arrow){
        if(arrow+1>s){
          object_property_set(o, key, 0);
          continue;
        }
        if(arrow+1==s){
          item* r = list_get_n(li, arrow+1);
          if(item_is_type(r, ITEM_VALUE)){
            object_property_set(o, key, value_string((value*)r));
          }
          continue;
        }
        item* r1 = list_get_n(li, arrow+1);
        item* r2 = list_get_n(li, arrow+2);
        if(item_is_type(r1, ITEM_VALUE) && item_is_type(r2, ITEM_VALUE)){
          if(item_equal(r1, value_new("@."))){
            object_property_add(o, key, value_string((value*)r2));
          }
        }
      }
      if(arrowdel){
        char delkey[128]; snprintf(delkey, 128, "%s:%d", key, arrowdel);
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

bool evaluate_clock_sync(object* o, void* d)
{
  if(!discover_io_peer(o, "sync-clock", "clock")) return true;
  char* sync_clock_ts_str=object_property(o, "sync-clock:timestamp");
  if(sync_clock_ts_str && !object_property_is(o, "sync-ts", sync_clock_ts_str)){
    object_property_set(o, "sync-ts", sync_clock_ts_str);
    char* e; uint64_t sync_clock_ts=strtoull(sync_clock_ts_str,&e,10);
    if(sync_clock_ts) time_es_set(sync_clock_ts);
  }
  return true;
}

bool evaluate_clock(object* o, void* d)
{
  uint64_t es=time_es();
  char ess[16];
#if defined(NRF5)
  if(es>>32) snprintf(ess, 16, "%lu%lu", ((uint32_t)(es>>32)),(uint32_t)es);
  else       snprintf(ess, 16,    "%lu",                      (uint32_t)es);
#else
  if(es>>32) snprintf(ess, 16, "%u%u", ((uint32_t)(es>>32)),(uint32_t)es);
  else       snprintf(ess, 16,   "%u",                      (uint32_t)es);
#endif

  if(object_property_is(o, "timestamp", ess)) return true;

  object_property_set(o, "timestamp", ess);

  time_t est = (time_t)es;
  struct tm* tms = localtime(&est);
  char ts[32];

  strftime(ts, 32, "%Y/%m/%d", tms);
  object_property_set(o, "date", ts);

  strftime(ts, 32, "%H:%M", tms);
  object_property_set(o, "time", ts);

  strftime(ts, 32, "%S", tms);
  object_property_set(o, "seconds", ts);

  return true;
}

