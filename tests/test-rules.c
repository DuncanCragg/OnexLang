
#include <stdio.h>
#include <inttypes.h>
#include <tests.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <items.h>
#include <onr.h>

// ---------------------------------------------------------------------------------

void run_evaluate_edit_rule_tests() {

  log_write("------edit rule behaviour tests-----\n");

  onex_set_evaluators("evaluate_edit_rule", evaluate_edit_rule, 0);

  object* target=object_new(0, "evaluate_edit_rule", (char*)"editable object", 3);
  char* targetuid=object_property(target, "UID");

  object* edit=object_new(0, 0, (char*)"editable edit rule", 3);
  object_property_add(edit, (char*)"Notifying", targetuid);

  object_property_set(edit, "banana", "=> mango");
  onex_loop();
  onex_assert_equal(object_property(target, "banana"), "mango",      "evaluate_edit_rule set banana to 'mango'");

  object_property_set(edit, "banana", "=> @. orange");
  onex_loop();
  onex_assert_equal(object_property(target, "banana:1"), "mango",    "evaluate_edit_rule set banana to 'mango orange'");
  onex_assert_equal(object_property(target, "banana:2"), "orange",   "evaluate_edit_rule set banana to 'mango orange'");

  object_property_set(edit, "banana", "=> @. fig");
  onex_loop();
  onex_assert_equal(object_property(target, "banana:1"), "mango",    "evaluate_edit_rule set banana to 'mango orange fig'");
  onex_assert_equal(object_property(target, "banana:2"), "orange",   "evaluate_edit_rule set banana to 'mango orange fig'");
  onex_assert_equal(object_property(target, "banana:3"), "fig",      "evaluate_edit_rule set banana to 'mango orange fig'");

  object_property_set(edit, "banana", "something (=>) something");
  onex_loop();
  onex_assert_equal(object_property(target, "banana:1"), "mango",    "evaluate_edit_rule set banana to 'mango fig'");
  onex_assert_equal(object_property(target, "banana:2"), "fig",      "evaluate_edit_rule set banana to 'mango fig'");

  object_property_set(edit, "banana", "=>");
  onex_loop();
  onex_assert(     !object_property(target, "banana"),               "evaluate_edit_rule set banana to nothing");
}


