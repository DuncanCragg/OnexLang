
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
}

