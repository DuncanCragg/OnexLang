
#include <stdio.h>
#include <assert.h>
#include <onex-kernel/log.h>
#include <items.h>
#include <onr.h>

// ---------------------------------------------------------------------------------

uint8_t evaluate_button_io_called=0;
bool evaluate_button_io(object* button, void* pressed)
{
  evaluate_button_io_called++;
  if(evaluate_button_io_called==1) onex_assert(!pressed, "evaluate_button_io arg is false the first time");
  if(evaluate_button_io_called==2) onex_assert(!!pressed, "evaluate_button_io arg is true the second time");
  char* s=(char*)(pressed? "down": "up");
  object_property_set(button, "state", s);
  return true;
}

uint8_t evaluate_light_io_called=0;
bool evaluate_light_io(object* light, void* d)
{
  evaluate_light_io_called++;
  if(evaluate_light_io_called==1) onex_assert(object_property_is(light, "light", "off"), "evaluate_light_io light is off the first time");
  if(evaluate_light_io_called==2) onex_assert(object_property_is(light, "light", "off"), "evaluate_light_io light is off the second time");
  if(evaluate_light_io_called==3) onex_assert(object_property_is(light, "light", "on"),  "evaluate_light_io light is on the third time");
  return true;
}

void run_light_tests()
{
  log_write("------light behaviour tests-----\n");

  onex_set_evaluators("evaluate_button", evaluate_button_io, 0);
  onex_set_evaluators("evaluate_light", evaluate_light_logic, evaluate_light_io, 0);

  object* button=object_new(0, "evaluate_button", "button", 4);
  object* light =object_new(0, "evaluate_light",  "light", 4);

  char* buttonuid=object_property(button, "UID");
  char* lightuid=object_property(light, "UID");

  object_property_set(light, "light", "off");
  object_property_set(light, "button", buttonuid);

  onex_run_evaluators(lightuid, 0);

  bool button_pressed=false;
  onex_run_evaluators(buttonuid, (void*)button_pressed);

  button_pressed=true;
  onex_run_evaluators(buttonuid, (void*)button_pressed);

  onex_assert_equal_num(evaluate_button_io_called, 2,  "evaluate_button_io was called");
  onex_assert_equal_num(evaluate_light_io_called, 3,  "evaluate_light_io was called");
}

void run_evaluate_object_setter_tests()
{
  log_write("------object setter behaviour tests-----\n");

  onex_set_evaluators("evaluate_setter", evaluate_object_setter, 0);

  object* target=object_new(0, "evaluate_setter", "thing", 4);
  char* targetuid=object_property(target, "UID");

  properties* update = properties_new(3);
  list*       li=list_new(3);
  list_add(li, value_new("fig"));
  list_add(li, value_new("=>"));
  list_add(li, value_new("mango"));
  properties_set(update, value_new("banana"), li);

  onex_run_evaluators(targetuid, update);
  onex_loop();
  onex_assert_equal(object_property(target, "banana"), "mango", "evaluate_object_setter set banana to mango");

  update = properties_new(3);
  li=list_new(3);
  list_add(li, value_new("=>"));
  list_add(li, value_new("@."));
  list_add(li, value_new("orange"));
  properties_set(update, value_new("banana"), li);

  onex_run_evaluators(targetuid, update);
  onex_loop();
  onex_assert_equal(object_property_values(target, "banana"), "mango orange", "evaluate_object_setter set banana to 'mango orange'");

  update = properties_new(3);
  li=list_new(3);
  list_add(li, value_new("=>"));
  list_add(li, value_new("@."));
  list_add(li, value_new("fig"));
  properties_set(update, value_new("banana"), li);

  onex_run_evaluators(targetuid, update);
  onex_loop();
  onex_assert_equal(object_property_values(target, "banana"), "mango orange fig", "evaluate_object_setter set banana to 'mango orange fig'");

  update = properties_new(3);
  li=list_new(3);
  list_add(li, value_new("something"));
  list_add(li, value_new("(=>)"));
  list_add(li, value_new("something"));
  properties_set(update, value_new("banana"), li);

  onex_run_evaluators(targetuid, update);
  onex_loop();
  onex_assert_equal(object_property_values(target, "banana"), "mango fig", "evaluate_object_setter set banana to 'mango fig'");

  update = properties_new(3);
  li=list_new(3);
  list_add(li, value_new("=>"));
  properties_set(update, value_new("banana"), li);

  onex_run_evaluators(targetuid, update);
  onex_loop();
  onex_assert(     !object_property_values(target, "banana"),              "evaluate_object_setter set banana to empty");
}

void run_evaluate_edit_rule_tests()
{
  log_write("------edit rule behaviour tests-----\n");

  onex_set_evaluators("evaluate_edit_rule", evaluate_edit_rule, 0);

  object* target=object_new(0, "evaluate_edit_rule", (char*)"editable object", 3);
  char* targetuid=object_property(target, "UID");

  object* edit=object_new(0, 0, (char*)"editable edit rule", 3);
  object_property_add(edit, (char*)"Notifying", targetuid);

  object_property_set(edit, "banana", "=> mango");
  onex_assert_equal(object_property_values(target, "banana"), "mango",            "evaluate_edit_rule set banana to 'mango'");

  object_property_set(edit, "banana", "=> @. orange");
  onex_assert_equal(object_property_values(target, "banana"), "mango orange",     "evaluate_edit_rule set banana to 'mango orange'");

  object_property_set(edit, "banana", "=> @. fig");
  onex_assert_equal(object_property_values(target, "banana"), "mango orange fig", "evaluate_edit_rule set banana to 'mango orange fig'");

  object_property_set(edit, "banana", "something (=>) something");
  onex_assert_equal(object_property_values(target, "banana"), "mango fig",        "evaluate_edit_rule set banana to 'mango fig'");

  object_property_set(edit, "banana", "=>");
  onex_assert(     !object_property_values(target, "banana"),                     "evaluate_edit_rule set banana to nothing");
}

void run_device_tests()
{
  log_write("------device behaviour tests-----\n");

  onex_set_evaluators("evaluate_device", evaluate_device_logic, 0);
  object_set_evaluator(onex_device_object, (char*)"evaluate_device");

  object_property_set(onex_device_object, "incoming", "uid-incomingdevice");
  object_property(onex_device_object, "incoming:UID");
  object* incomingdevice=onex_get_from_cache("uid-incomingdevice");
  object_property_set(incomingdevice, "is", "device");

  onex_assert_equal(object_property(onex_device_object, "connected-devices"), "uid-incomingdevice", "device evaluator adds incoming device to connected-devices");
}

