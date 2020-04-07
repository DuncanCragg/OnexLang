
#include <onex-kernel/gpio.h>
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>
#include <assert.h>
#include <onf.h>

extern void run_light_tests();
extern void run_evaluate_object_setter_tests();
extern void run_evaluate_edit_rule_tests();
extern void run_device_tests();
extern void run_clock_tests();

int main(void) {

  log_init();

  log_write("-----------------ONR tests------------------------\n");

  onex_init("");

  run_light_tests();
  run_evaluate_object_setter_tests();
  run_evaluate_edit_rule_tests();
  run_device_tests();
  run_clock_tests();

  int failures=onex_assert_summary();
  return failures;
}

