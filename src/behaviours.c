#include <onf.h>
#include <behaviours.h>

bool evaluate_light_logic(object* light, void* d)
{
  bool buttonpressed=object_property_is(light, "button:state", "down");
  char* s=(char*)(buttonpressed? "on": "off");
  object_property_set(light, "light", s);
  return true;
}
