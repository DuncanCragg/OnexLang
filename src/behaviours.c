#define _GNU_SOURCE
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <items.h>
#include <onn.h>
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
    if(object_property_contains(o, ispath, is)) return true;
  }
  if(ln) object_property_set(o, property, 0);
  return false;
}

bool evaluate_light_logic(object* o, void* d)
{
  bool light_on=object_property_is(o, "light", "on");

  if(light_on && object_property_is(o, "Timer", "0")){

    object_property_set(o, "Timer", "");
    object_property_set(o, "light", "off");

    return true;
  }

  bool changed=false;

  if(!light_on && (
       object_property_is(o, "button:state",   "down") ||
       object_property_is(o, "touch:action",   "down") ||
       object_property_is(o, "motion:gesture", "view-screen"))){

    light_on=true;
    object_property_set(o, "light", "on");
    changed=true;
  }

  if(light_on /* && !object_property(o, "Timer") */ )  {

    char* timeout=object_property(o, "timeout");
    if(timeout){
      object_property_set(o, "Timer", timeout);
      changed=true;
    }
  }

  ;  object_property(o, "button:is"); // observe the button
  if(object_property(o, "touch:is") ||
     object_property(o, "motion:is")   ) return changed;

  if(!discover_io_peer(o, "button", "button")) return changed;

  if(light_on && object_property_is(o, "button:state", "up")){

    object_property_set(o, "light", "off");
    changed=true;
  }
  return changed;
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
    object_property_set(o, "ts",  sync_ts);
    object_property_set(o, "tz",  object_property(o, "sync-clock:tz:1"));
    object_property_add(o, "tz",  object_property(o, "sync-clock:tz:2"));
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

  if(object_property_is(o, "ts", ess)) return true; // XXX remove when no-change OK

  object_property_set(o, "ts", ess);

  if(!object_property(o, "sync-clock")){
    time_t est = (time_t)es;
    struct tm tms={0};
    localtime_r(&est, &tms);
    char t[32];
#if !defined(NRF5)
    snprintf(t, 32, "%s %ld", tms.tm_zone, tms.tm_gmtoff);
#else
    snprintf(t, 32, "GMT 0");
#endif
    object_property_set(o, "tz", t);
  }

  return true;
}

