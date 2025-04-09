
#if defined(NRF5)
#include <boards.h>
#include <onex-kernel/gpio.h>
#if defined(HAS_SERIAL)
#include <onex-kernel/serial.h>
#endif
#endif
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>
#include <tests.h>
#include <onn.h>

extern void run_light_tests();
extern void run_evaluate_edit_rule_tests();
extern void run_device_tests();
extern void run_clock_tests();

#if defined(NRF5)
#if defined(BOARD_PCA10059)
const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;
#endif

static void set_up_gpio(void)
{
#if defined(BOARD_PCA10059)
  for(uint8_t l=0; l< LEDS_NUMBER; l++){ gpio_mode(leds_list[l], OUTPUT); gpio_set(leds_list[l], 1); }
  gpio_set(leds_list[0], 0);
#endif
}
#endif

static volatile bool run_tests=false;

void on_recv(unsigned char* chars, size_t size)
{
  if(!size) return;
  log_write(">%c<----------\n", chars[0]);
  if(chars[0]=='t') run_tests=true;
}

void run_tests_maybe()
{
  if(!run_tests) return;
  run_tests=false;

  log_write("ONR tests\n");

  onex_init(0);

  run_light_tests();
  run_device_tests();
  run_clock_tests();
  run_evaluate_edit_rule_tests();

#if defined(NRF5)
  int failures=onex_assert_summary();
#if defined(BOARD_PCA10059)
  if(failures) gpio_set(leds_list[1], 0);
  else         gpio_set(leds_list[2], 0);
#endif
#else
  onex_assert_summary();
#endif
}

int main(void)
{
  log_init();
  time_init();
#if defined(NRF5)
  gpio_init();
#if defined(HAS_SERIAL)
  serial_init((serial_recv_cb)on_recv,0);
  set_up_gpio();
  time_ticker((void (*)())serial_loop, 1);
  while(1) run_tests_maybe();
#else
  set_up_gpio();
  while(1){
    log_loop();
    run_tests_maybe();
  }
#endif
#else
  on_recv((unsigned char*)"t", 1);
  run_tests_maybe();
  time_end();
#endif
}

