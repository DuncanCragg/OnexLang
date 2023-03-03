#define _GNU_SOURCE
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <onex-kernel/lib.h>
#include <onex-kernel/log.h>
#include <onex-kernel/mem.h>
#include <onex-kernel/time.h>
#include <items.h>
#include <onn.h>
#include <onr.h>

bool evaluate_edit_rule(object* o, void* d) {

  if(!(object_property_contains(o, "Alerted:is", "edit") &&
       object_property_contains(o, "Alerted:is", "rule")   )) return true;

  uint16_t n=object_property_size(o, "Alerted:");
  for(uint16_t i=1; i<=n; i++){

    char* key=object_property_key(o, "Alerted:", i);
    if(!strcmp(key, "is")) continue;

    char keyesc[128]; mem_strncpy(keyesc, key, 128);
    prefix_char_in_place(keyesc, '\\', ':');
    char pathkey[128]; snprintf(pathkey, 128, "Alerted:%s", keyesc);
    uint16_t ln = object_property_length(o, pathkey);

    uint16_t arrowindex=0;
    uint16_t atdotindex=0;

    uint16_t j=1;
    for(; j<=ln; j++){
      char* token=object_property_get_n(o, pathkey, j);
      if(!arrowindex){
        if(!strcmp(token, "=>")){
          arrowindex=j;
        }
        continue;
      }
      if(!strcmp(token, "@.")){
        atdotindex=j;
        break;
      }
    }
    if(!arrowindex) continue;

    if(arrowindex==ln){
      object_property_set(o, key, 0);
      continue;
    }
    if(!atdotindex){
      for(j=ln; j>arrowindex; j--){
        char* token=object_property_get_n(o, pathkey, j);
        if(j==ln) object_property_set(o, key, token);
        else      object_property_insert(o, key, token);
      }
      continue;
    }
    for(j=atdotindex-1; j>arrowindex; j--){
      char* token=object_property_get_n(o, pathkey, j);
      object_property_prepend(o, key, token);
    }
    for(j=atdotindex+1; j<=ln; j++){
      char* token=object_property_get_n(o, pathkey, j);
      object_property_append(o, key, token);
    }
  }
  return true;
}


