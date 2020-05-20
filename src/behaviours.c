#define _GNU_SOURCE
#include <time.h>
#include <inttypes.h>
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
  if(ln) object_property_set(o, property, 0);
  return false;
}

bool evaluate_light_logic(object* o, void* d)
{
  if(object_property_is(o, "Timer", "0")){
    object_property_set(o, "Timer", "");
    object_property_set(o, "light", "off");
    return true;
  }
  if(object_property_is(o, "button:state",   "down") ||
     object_property_is(o, "touch:action:1", "down") ||
     object_property_is(o, "motion:gesture", "view-screen")){
    object_property_set(o, "light", "on");
  }
  if( object_property_is(o, "light", "on") &&
     !object_property(o, "Timer")             )  {
    char* timeout=object_property(o, "timeout");
    if(!timeout) timeout="2000";
    object_property_set(o, "Timer", timeout);
  }
  if(!object_property(o, "touch:is") &&
     !object_property(o, "motion:is")   ){
    if(!discover_io_peer(o, "button", "button")) return true;
    if(object_property_is(o, "button:state", "up"  )){
      object_property_set(o, "light", "off");
    }
  }
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
  if(object_property_contains(o, "Alerted:is", "edit") &&
     object_property_contains(o, "Alerted:is", "rule")   ){

    apply_edit(o);
  }
  return true;
}

bool evaluate_device_logic(object* o, void* d)
{
  if(object_property_contains(o, "Alerted:is", "device")){
    char* devuid=object_property(o, "Alerted");
    if(!object_property_contains(o, "connected-devices", devuid)){
      object_property_add(o, "connected-devices", devuid);
    }
  }
  return true;
}

bool evaluate_clock_sync(object* o, void* d)
{
  if(!discover_io_peer(o, "sync-clock", "clock")) return true;
  char* sync_ts=object_property(o, "sync-clock:ts");
  if(sync_ts && !object_property_is(o, "sync-ts", sync_ts)){
    object_property_set(o, "sync-ts",  sync_ts);
    object_property_set(o, "tz",  object_property_values(o, "sync-clock:tz"));
    char* e; uint64_t sync_clock_ts=strtoull(sync_ts,&e,10);
    if(!*e && sync_clock_ts) time_es_set(sync_clock_ts);
  }
  return true;
}

bool evaluate_clock(object* o, void* d)
{
  uint64_t es=time_es();

  char ess[16];
#if defined(NRF5)
  if(es>>32){
    // sort this out in 2038
    log_write("timestamp overflow\n");
    uint32_t lo=es & 0xffffffff;
    uint32_t hi=(es>>32);
    snprintf(ess, 16, "%lu:%lu", hi, lo);
  }
  else snprintf(ess, 16, "%"PRIu32, (uint32_t)es);
#else
  snprintf(ess, 16, "%"PRIu64, es);
#endif

  if(object_property_is(o, "ts", ess)) return true;

  object_property_set(o, "ts", ess);

#if !defined(NRF5)
  time_t est = (time_t)es;
  struct tm tms={0};
  localtime_r(&est, &tms);
  char t[32]; snprintf(t, 32, "%s %ld", tms.tm_zone, tms.tm_gmtoff);
  object_property_set(o, "tz", t);
#endif

  return true;
}

