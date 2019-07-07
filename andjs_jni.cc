#include <memory>
#include "base/memory/weak_ptr.h"
#include "base/android/jni_weak_ref.h"
#include "base/android/jni_android.h"
#include "base/android/jni_utils.h"
#include "base/android/jni_string.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/android/build_info.h"
#include "base/system/sys_info.h"
#include "base/threading/thread.h"
#include "base/run_loop.h"

#include "base/command_line.h"
#include "base/no_destructor.h"
#include "chrome/app/android/chrome_jni_onload.h"
#include "base/android/library_loader/library_loader_hooks.h"

#include "andjs/andjs_core.h"

#include "andjs/android/andjs_jni_registration.h"
#include "jni/AndJS_jni.h"

namespace andjs {

using base::android::JavaParamRef;

static jlong JNI_AndJS_InitAndJS(JNIEnv* env,
                                 const base::android::JavaParamRef<jobject>& jcaller) {
  AndJSCore* jscore = NULL;
  jscore = new AndJSCore();
  LOG(INFO) << "BuildInfo.device " << base::android::BuildInfo::GetInstance()->device();
  jscore->Init();
  return reinterpret_cast<intptr_t>(jscore);
}

} //namespace andjs

static bool NativeInit(base::android::LibraryProcessType) {
  // Setup a working test environment for the network service in case it's used.
  // Only create this object in the utility process, so that its members don't
  // interfere with other test objects in the browser process.
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  LOG(INFO) << " NativeInit command_line.argv size = " << command_line->argv().size();

  return base::android::OnJNIOnLoadInit();
}

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  // By default, all JNI methods are registered. However, since render processes
  // don't need very much Java code, we enable selective JNI registration on the
  // Java side and only register a subset of JNI methods.
  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();

  if (!base::android::IsSelectiveJniRegistrationEnabled(env) &&
      !RegisterNonMainDexNatives(env)) {
    return -1;
  }

  if (!RegisterMainDexNatives(env)) {
    return -1;
  }

  base::android::SetNativeInitializationHook(NativeInit);
  //NativeInit(base::android::PROCESS_CHILD);

  LOG(INFO) << " JNI_OnLoad OK!";

  return JNI_VERSION_1_4;
}
