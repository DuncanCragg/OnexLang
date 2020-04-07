
#if defined(NRF5)
#include <boards.h>
#include <onex-kernel/gpio.h>
#include <onex-kernel/serial.h>
#include <onex-kernel/blenus.h>
#endif
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>
#include <assert.h>
#include <onf.h>

extern void run_light_tests();
extern void run_evaluate_object_setter_tests();
extern void run_evaluate_edit_rule_tests();
extern void run_device_tests();
extern void run_clock_tests();

static volatile bool run_tests=false;

void on_recv(unsigned char* buf, size_t size)
{
  if(!size) return;
  log_write(">%c\n", buf[0]);
  if(buf[0]=='t') run_tests=true;
}

static void run_tests_maybe();

int main(void)
{
  log_init();
  time_init();
#if defined(NRF5)
  gpio_init();
  serial_init((serial_recv_cb)on_recv,0);
  blenus_init(0);
  time_ticker(serial_loop, 1);
  while(1) run_tests_maybe();
#else
  on_recv((unsigned char*)"t", 1);
  run_tests_maybe();
#endif
}

void run_tests_maybe()
{
  if(!run_tests) return;
  run_tests=false;

  log_write("-----------------ONR tests------------------------\n");

  onex_init("");

  run_light_tests();
  run_evaluate_object_setter_tests();
  run_evaluate_edit_rule_tests();
  run_device_tests();
  run_clock_tests();

  onex_assert_summary();
}

