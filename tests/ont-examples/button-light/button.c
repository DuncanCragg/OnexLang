
#include <time.h>
#include <stdlib.h>
#if defined(NRF5)
#include <boards.h>
#include <onex-kernel/gpio.h>
#if defined(HAS_SERIAL)
#include <onex-kernel/serial.h>
#endif
#include <onex-kernel/blenus.h>
#endif
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>
#include <onf.h>

#if defined(NRF5)
#define SYNC_TO_PEER_CLOCK
#endif

char* buttonuid;
char* clockuid;

void button_1_change_cb(int);
bool evaluate_button(object* button, void* pressed);

static void every_second()
{
  onex_run_evaluators(clockuid, 0);
}

bool evaluate_clock(object* oclock, void* d)
{
#if defined(SYNC_TO_PEER_CLOCK)
  if(!object_property_contains(oclock, "sync-clock:is", "clock")){
    int ln=object_property_length(oclock, "device:connected-devices:io");
    for(int i=1; i<=ln; i++){
      char* uid=object_property_get_n(oclock, "device:connected-devices:io", i);
      if(!is_uid(uid)) continue;
      object_property_set(oclock, "sync-clock", uid);
      if(object_property_contains_peek(oclock, "sync-clock:is", "clock")) break;
    }
  }
  char* sync_clock_ts_str=object_property(oclock, "sync-clock:timestamp");
  if(sync_clock_ts_str && !object_property_is(oclock, "sync-ts", sync_clock_ts_str)){
    object_property_set(oclock, "sync-ts", sync_clock_ts_str);
    char* e; uint64_t sync_clock_ts=strtoull(sync_clock_ts_str,&e,10);
    if(sync_clock_ts) time_es_set(sync_clock_ts);
  }
#endif

  uint64_t es=time_es();
  char ess[16];
#if defined(NRF5)
  if(es>>32) snprintf(ess, 16, "%lu%lu", ((uint32_t)(es>>32)),(uint32_t)es);
  else       snprintf(ess, 16,    "%lu",                      (uint32_t)es);
#else
  if(es>>32) snprintf(ess, 16, "%u%u", ((uint32_t)(es>>32)),(uint32_t)es);
  else       snprintf(ess, 16,   "%u",                      (uint32_t)es);
#endif

  if(object_property_is(oclock, "timestamp", ess)) return true;

  object_property_set(oclock, "timestamp", ess);

  time_t est = (time_t)es;
  struct tm* tms = localtime(&est);
  char ts[32];

  strftime(ts, 32, "%Y/%m/%d", tms);
  object_property_set(oclock, "date", ts);

  strftime(ts, 32, "%H:%M:%S", tms);
  object_property_set(oclock, "time", ts);

  return true;
}

// Copied from ONR Behaviours
bool evaluate_device_logic(object* device, void* d)
{
  if(object_property_contains(device, (char*)"Alerted:is", (char*)"device")){
    char* devuid=object_property(device, (char*)"Alerted");
    if(!object_property_contains(device, (char*)"connected-devices", devuid)){
      object_property_add(device, (char*)"connected-devices", devuid);
    }
  }
  return true;
}

int main()
{
  log_init();
  time_init();
#if defined(NRF5)
  gpio_init();
#if defined(HAS_SERIAL)
  serial_init(0,0);
#endif
  blenus_init(0);
#endif
  onex_init("");

#if defined(BOARD_PCA10059)
  gpio_mode_cb(BUTTON_1, INPUT_PULLUP, button_1_change_cb);
#elif defined(BOARD_PINETIME)
  gpio_mode_cb(BUTTON_1, INPUT_PULLDOWN, button_1_change_cb);
  gpio_mode(   BUTTON_ENABLE, OUTPUT);
  gpio_set(    BUTTON_ENABLE, 1);
#else
  log_write("\n------Starting Button Test-----\n");
#endif

  onex_set_evaluators("evaluate_device", evaluate_device_logic, 0);
  onex_set_evaluators("evaluate_button", evaluate_button, 0);
  onex_set_evaluators("evaluate_clock",  evaluate_clock, 0);

  object_set_evaluator(onex_device_object, (char*)"evaluate_device");
#if defined(SYNC_TO_PEER_CLOCK)
  char* deviceuid=object_property(onex_device_object, "UID");
#endif

  object* button=object_new(0, "evaluate_button", "editable button", 4);
  object_property_set(button, "name", "£€§");
  buttonuid=object_property(button, "UID");

  object* oclock=object_new(0, "evaluate_clock", "clock event", 12);
  object_property_set(oclock, "title", "OnexOS Clock");
  object_property_set(oclock, "timestamp", "1585045750");
  object_property_set(oclock, "timezone", "GMT");
  object_property_set(oclock, "daylight", "BST");
  object_property_set(oclock, "date", "2020-03-24");
  object_property_set(oclock, "time", "12:00:00");
#if defined(SYNC_TO_PEER_CLOCK)
  object_property_set(oclock, "device", deviceuid);
#endif
  clockuid =object_property(oclock, "UID");

  object_property_add(onex_device_object, (char*)"io", buttonuid);
  object_property_add(onex_device_object, (char*)"io", clockuid);

  time_ticker(every_second, 1000);

#if !defined(NRF5)
  uint32_t lasttime=0;
  bool button_pressed=false;
#endif

  while(1){

    onex_loop();

#if !defined(NRF5)
    if(time_ms() > lasttime+1200u){
      lasttime=time_ms();
      button_pressed=!button_pressed;
      button_1_change_cb(button_pressed);
    }
#endif
  }
}

void button_1_change_cb(int button_pressed)
{
  onex_run_evaluators(buttonuid, (void*)(bool)button_pressed);
}

bool evaluate_button(object* button, void* pressed)
{
  char* s=(char*)(pressed? "down": "up");
  object_property_set(button, "state", s);
#if !defined(NRF5)
  log_write("evaluate_button: "); object_log(button);
#endif
  return true;
}


