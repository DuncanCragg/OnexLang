#include <onf.h>
#include <onr.h>

bool evaluate_light_logic(object* o, void* d)
{
  bool buttonpressed=object_property_is(o, "button:state", "down");
  char* s=(char*)(buttonpressed? "on": "off");
  object_property_set(o, "light", s);
  return true;
}
