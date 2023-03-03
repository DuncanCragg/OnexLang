
#include <stdio.h>
#include <stdlib.h>
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

  object_property_set(edit, "fruits", "=> mango orange fig");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",    "evaluate_edit_rule set fruits to 'mango orange fig'");
  onex_assert_equal(object_property(target, "fruits:2"), "orange",   "evaluate_edit_rule set fruits to 'mango orange fig'");
  onex_assert_equal(object_property(target, "fruits:3"), "fig",      "evaluate_edit_rule set fruits to 'mango orange fig'");

  object_property_set(edit, "fruits", "=> mango");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits"), "mango",      "evaluate_edit_rule set fruits to 'mango'");

  object_property_set(edit, "fruits", "=> @. orange");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",    "evaluate_edit_rule set fruits to 'mango orange'");
  onex_assert_equal(object_property(target, "fruits:2"), "orange",   "evaluate_edit_rule set fruits to 'mango orange'");

  object_property_set(edit, "fruits", "=> @. fig banana tangerine");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",     "evaluate_edit_rule set fruits to 'mango orange fig banana tangerine'");
  onex_assert_equal(object_property(target, "fruits:2"), "orange",    "evaluate_edit_rule set fruits to 'mango orange fig banana tangerine'");
  onex_assert_equal(object_property(target, "fruits:3"), "fig",       "evaluate_edit_rule set fruits to 'mango orange fig banana tangerine'");
  onex_assert_equal(object_property(target, "fruits:4"), "banana",    "evaluate_edit_rule set fruits to 'mango orange fig banana tangerine'");
  onex_assert_equal(object_property(target, "fruits:5"), "tangerine", "evaluate_edit_rule set fruits to 'mango orange fig banana tangerine'");

  object_property_set(edit, "fruits", 0);

  // ----- delete from list -------

  object_property_set(edit, "fruits\\:3", "=>");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "mango",     "evaluate_edit_rule deletes middle item: 'fig'");
  onex_assert_equal(object_property(target, "fruits:2"), "orange",    "evaluate_edit_rule deletes middle item: 'fig'");
  onex_assert_equal(object_property(target, "fruits:3"), "banana",    "evaluate_edit_rule deletes middle item: 'fig'");
  onex_assert_equal(object_property(target, "fruits:4"), "tangerine", "evaluate_edit_rule deletes middle item: 'fig'");
  object_property_set(edit, "fruits\\:3", 0);

  object_property_set(edit, "fruits\\:1", "=>");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "orange",    "evaluate_edit_rule deletes first item 'mango'");
  onex_assert_equal(object_property(target, "fruits:2"), "banana",    "evaluate_edit_rule deletes first item 'mango'");
  onex_assert_equal(object_property(target, "fruits:3"), "tangerine", "evaluate_edit_rule deletes first item 'mango'");
  object_property_set(edit, "fruits\\:1", 0);

  object_property_set(edit, "fruits\\:3", "=>");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "orange",    "evaluate_edit_rule deletes last item 'tangerine'");
  onex_assert_equal(object_property(target, "fruits:2"), "banana",    "evaluate_edit_rule deletes last item 'tangerine'");
  onex_assert_equal_num(object_property_length(target, "fruits"), 2,  "evaluate_edit_rule fruits is len 2 ");
  object_property_set(edit, "fruits\\:3", 0);

  object_property_set(edit, "fruits", "=>");
  onex_loop();
  onex_assert(         !object_property(       target, "fruits"),     "evaluate_edit_rule deletes all fruit");
  onex_assert_equal_num(object_property_length(target, "fruits"), 0,  "evaluate_edit_rule fruits is len 2 ");
  object_property_set(edit, "fruits", 0);

  // ----- set in list -------

  object_property_set(edit, "fruits", "=> mango @. banana");
  onex_loop();
  object_property_set(edit, "fruits", "=> orange @. papaya");
  onex_loop();
  object_property_set(edit, "fruits", "=> fig @. apple");
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
  onex_assert_equal(object_property(target, "fruits:5"), "apple",  "evaluate_edit_rule leaves value");
  object_property_set(edit, "fruits\\:1", 0);
  object_property_set(edit, "fruits\\:3", 0);

  // ----- prepend in list -------

  object_property_set(edit, "fruits", "=> tangerine @.");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "tangerine", "evaluate_edit_rule set fruits to 'tangerine potato ...'");
  onex_assert_equal(object_property(target, "fruits:2"), "potato",    "evaluate_edit_rule set fruits to 'tangerine potato ...'");

  object_property_set(edit, "fruits", "=> grapes @.");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "grapes",    "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert_equal(object_property(target, "fruits:2"), "tangerine", "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert_equal(object_property(target, "fruits:3"), "potato",    "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert_equal(object_property(target, "fruits:4"), "orange",    "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert_equal(object_property(target, "fruits:5"), "banana",    "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert_equal(object_property(target, "fruits:6"), "papaya",    "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert_equal(object_property(target, "fruits:7"), "apple",     "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");
  onex_assert(     !object_property(target, "fruits:8"),              "evaluate_edit_rule set fruits to 'grapes tangerine potato orange banana papaya apple'");

  object_property_set(edit, "fruits", 0);

  // ----- insert in list -------

  object_property_set(edit, "fruits\\:3", "=> carrot swedes");
  onex_loop();
  onex_assert_equal(object_property(target, "fruits:1"), "grapes",    "evaluate_edit_rule leaves value");
  onex_assert_equal(object_property(target, "fruits:2"), "tangerine", "evaluate_edit_rule leaves value");
  onex_assert_equal(object_property(target, "fruits:3"), "carrot",    "evaluate_edit_rule replaces banana with carrot");
  onex_assert_equal(object_property(target, "fruits:4"), "swedes",    "evaluate_edit_rule inserts swedes");
  onex_assert_equal(object_property(target, "fruits:5"), "orange",    "evaluate_edit_rule leaves value");
  onex_assert_equal(object_property(target, "fruits:6"), "banana",    "evaluate_edit_rule leaves value");
  object_property_set(edit, "fruits\\:3", 0);
}


