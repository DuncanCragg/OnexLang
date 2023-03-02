
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
  onex_assert_equal(object_property(target, "fruits:1"), "mango",    "evaluate_edit_rule deletes 'orange'");
  onex_assert_equal(object_property(target, "fruits:2"), "fig",      "evaluate_edit_rule deletes 'orange'");

  object_property_set(edit, "fruits", "=>");
  object_property_set(edit, "fruits\\:2", 0);
  onex_loop();
  onex_assert(     !object_property(target, "fruits"),               "evaluate_edit_rule deletes all fruit");

  // ----- set in list -------

  object_property_set(edit, "fruits", "=> mango @. banana");
  onex_loop();
  object_property_set(edit, "fruits", "=> fig orange @.");
  onex_loop();
  object_property_set(edit, "fruits", "=> @. papaya apple");
  onex_loop();

  onex_assert_equal(object_property(target, "fruits:1"), "fig",    "evaluate_edit_rule sets fruits:1 to fig");
  onex_assert_equal(object_property(target, "fruits:2"), "orange", "evaluate_edit_rule sets fruits:2 to orange");
  onex_assert_equal(object_property(target, "fruits:3"), "mango",  "evaluate_edit_rule sets fruits:3 to mango");
  onex_assert_equal(object_property(target, "fruits:4"), "banana", "evaluate_edit_rule sets fruits:4 to banana");
  onex_assert_equal(object_property(target, "fruits:5"), "papaya", "evaluate_edit_rule sets fruits:5 to papaya");
  onex_assert_equal(object_property(target, "fruits:6"), "apple",  "evaluate_edit_rule sets fruits:6 to apple");

  object_property_set(edit, "fruits", 0);
  object_property_set(edit, "fruits\\:1", "=> potato");
  object_property_set(edit, "fruits\\:3", "=>");
  onex_loop();

  onex_assert_equal(object_property(target, "fruits:1"), "potato", "evaluate_edit_rule sets fruits:1 to potato");
  onex_assert_equal(object_property(target, "fruits:2"), "orange", "evaluate_edit_rule leaves value");
  onex_assert_equal(object_property(target, "fruits:3"), "banana", "evaluate_edit_rule has deleted mango");
  onex_assert_equal(object_property(target, "fruits:4"), "papaya", "evaluate_edit_rule leaves value");

  // ----- prepend in list -------

  object_property_set(edit, "fruits\\:1", 0);
  object_property_set(edit, "fruits\\:3", 0);
  object_property_set(edit, "fruits", "=> tangerine @.");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "tangerine", "evaluate_edit_rule set fruits to 'tangerine potato'");
  onex_assert_equal(object_property(target, "fruits:2"), "potato",    "evaluate_edit_rule set fruits to 'tangerine potato'");

  object_property_set(edit, "fruits", "=> grapes @.");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "grapes",    "evaluate_edit_rule set fruits to 'grapes tangerine potato'");
  onex_assert_equal(object_property(target, "fruits:2"), "tangerine", "evaluate_edit_rule set fruits to 'grapes tangerine potato'");
  onex_assert_equal(object_property(target, "fruits:3"), "potato",    "evaluate_edit_rule set fruits to 'grapes tangerine potato'");

}


