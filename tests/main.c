
#if defined(NRF5)
#include <boards.h>
#if defined(BOARD_PINETIME)
#include <onex-kernel/gfx.h>
#endif
#include <onex-kernel/gpio.h>
#if defined(HAS_SERIAL)
#include <onex-kernel/serial.h>
#endif
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
#if defined(HAS_SERIAL)
  serial_init((serial_recv_cb)on_recv,0);
  blenus_init(0);
  time_ticker(serial_loop, 1);
#else
  blenus_init((blenus_recv_cb)on_recv);
#if defined(BOARD_PINETIME)
  gfx_reset();
  gfx_init();
  gfx_screen_colour(0x0);
  gfx_screen_fill();
  gfx_pos(10, 10);
  gfx_text_colour(GFX_BLUE);
  gfx_text("OnexLang");
  gpio_mode(LCD_BACKLIGHT_HIGH, OUTPUT);
  gpio_set(LCD_BACKLIGHT_HIGH, LEDS_ACTIVE_STATE);
#endif
#endif
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

#if defined(NRF5)
#if defined(BOARD_PCA10059)
  onex_assert_summary();
#elif defined(BOARD_PINETIME)
  int failures=onex_assert_summary();
  gfx_pos(10, 10);
  gfx_text(failures? "FAIL": "SUCCESS");
#endif
#else
  onex_assert_summary();
#endif
}

