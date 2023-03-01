
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

  object* target=object_new(0, "evaluate_edit_rule", "editable fruit tracker", 3);
  char* targetuid=object_property(target, "UID");

  object* edit=object_new(0, 0, "fruit tracker edit rule", 3);
  object_property_add(edit, "Notifying", targetuid);

  // ----- set and append --------------
  object_property_set(edit, "fruits", "=> mango");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits"), "mango",      "evaluate_edit_rule set fruits to 'mango'");

  object_property_set(edit, "fruits", "=> @. orange");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",    "evaluate_edit_rule set fruits to 'mango orange'");
  onex_assert_equal(object_property(target, "fruits:2"), "orange",   "evaluate_edit_rule set fruits to 'mango orange'");

  object_property_set(edit, "fruits", "=> @. fig");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",    "evaluate_edit_rule set fruits to 'mango orange fig'");
  onex_assert_equal(object_property(target, "fruits:2"), "orange",   "evaluate_edit_rule set fruits to 'mango orange fig'");
  onex_assert_equal(object_property(target, "fruits:3"), "fig",      "evaluate_edit_rule set fruits to 'mango orange fig'");

  // ----- delete from list -------

  object_property_set(edit, "fruits", 0);
  object_property_set(edit, "fruits\\:2", "=>");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",    "evaluate_edit_rule set fruits to 'mango fig'");
  onex_assert_equal(object_property(target, "fruits:2"), "fig",      "evaluate_edit_rule set fruits to 'mango fig'");

  object_property_set(edit, "fruits", "=>");
  object_property_set(edit, "fruits\\:2", 0);
  onex_loop();
  onex_assert(     !object_property(target, "fruits"),               "evaluate_edit_rule set fruits to nothing");

  // ----- set in list -------

  object_property_set(edit, "fruits", "=> @. fig orange mango banana papaya");
  onex_loop();
  object_property_set(edit, "fruits", 0);
  object_property_set(edit, "fruits\\:1", "=> potato");
  object_property_set(edit, "fruits\\:3", "=>");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "potato", "evaluate_edit_rule sets fruits:1 to potato");
  onex_assert_equal(object_property(target, "fruits:2"), "orange", "evaluate_edit_rule leaves value");
  onex_assert_equal(object_property(target, "fruits:3"), "banana", "evaluate_edit_rule has deleted mango");
  onex_assert_equal(object_property(target, "fruits:4"), "papaya", "evaluate_edit_rule leaves value");
}


