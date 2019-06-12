#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>

extern android_app* androidApp;

void sprintExternalStorageDirectory(char* buf, int buflen, const char* format)
{   
  JNIEnv* env; androidApp->activity->vm->AttachCurrentThread(&env, 0);

  jclass osEnvClass = env->FindClass("android/os/Environment");
  jmethodID getExternalStorageDirectoryMethod = env->GetStaticMethodID(osEnvClass, "getExternalStorageDirectory", "()Ljava/io/File;");
  jobject extStorage = env->CallStaticObjectMethod(osEnvClass, getExternalStorageDirectoryMethod);

  jclass extStorageClass = env->GetObjectClass(extStorage);
  jmethodID getAbsolutePathMethod = env->GetMethodID(extStorageClass, "getAbsolutePath", "()Ljava/lang/String;");
  jstring extStoragePath = (jstring)env->CallObjectMethod(extStorage, getAbsolutePathMethod);

  const char* extStoragePathString=env->GetStringUTFChars(extStoragePath, 0);
  snprintf(buf, buflen, format, extStoragePathString);
  env->ReleaseStringUTFChars(extStoragePath, extStoragePathString);

  androidApp->activity->vm->DetachCurrentThread();
}

