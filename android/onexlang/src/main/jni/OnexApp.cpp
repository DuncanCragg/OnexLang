#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include <sys/system_properties.h>

extern "C" {
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <onf.h>
#include <assert.h>
extern void run_light_tests();
extern void run_evaluate_object_setter_tests();
extern void run_evaluate_edit_rule_tests();
extern void run_device_tests();
}

android_app* androidApp;

extern "C" {

void serial_send(char* b)
{
  log_write("serial_send %s\n", b);
}

}

class OnexApp
{
public:

  OnexApp()
  {
  }

  ~OnexApp()
  {
  }

  static void handleAppCommand(android_app* app, int32_t cmd)
  {
    OnexApp* onexapp = reinterpret_cast<OnexApp*>(app->userData);
    switch (cmd)
    {
    case APP_CMD_SAVE_STATE:
      log_write("APP_CMD_SAVE_STATE");
      break;
    case APP_CMD_INIT_WINDOW:
      log_write("APP_CMD_INIT_WINDOW");
      break;
    case APP_CMD_LOST_FOCUS:
      log_write("APP_CMD_LOST_FOCUS");
      break;
    case APP_CMD_GAINED_FOCUS:
      log_write("APP_CMD_GAINED_FOCUS");
      break;
    case APP_CMD_TERM_WINDOW:
      log_write("APP_CMD_TERM_WINDOW");
      break;
    }
  }

  static int32_t handleAppInput(struct android_app* app, AInputEvent* event)
  {
    OnexApp* onexapp = reinterpret_cast<OnexApp*>(app->userData);
    return 0;
  }

  void run()
  {
    log_init();
    time_init();

    log_write("---------------OnexLang tests----------------------\n");

    extern char* sprintExternalStorageDirectory(char* buf, int buflen, const char* format);
    char dbpath[128]; sprintExternalStorageDirectory(dbpath, 128, "%s/Onex/onex.ondb");

    onex_init("");

    run_light_tests();
    run_evaluate_object_setter_tests();
    run_evaluate_edit_rule_tests();
    run_device_tests();

    int failures=onex_assert_summary();

    log_write("---------------%d failures---------------------------\n", failures);

  }
};

OnexApp* onexapp;

void android_main(android_app* state)
{
  onexapp = new OnexApp();
  state->userData = onexapp;
  state->onAppCmd = OnexApp::handleAppCommand;
  state->onInputEvent = OnexApp::handleAppInput;
  androidApp = state;
  onexapp->run();
  delete(onexapp);
}

