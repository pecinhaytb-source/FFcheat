#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <cstring>
#include <sys/mman.h>

namespace zygisk {
    struct Api {
        void (*plt_hook_register)(void*, void*, void*) = nullptr;
    };
    struct AppSpecializeArgs {
        jstring *nice_name;
        jstring *dir;
    };
    class ModuleBase {
    public:
        virtual void onLoad(Api *api, JNIEnv *env) {}
        virtual void preAppSpecialize(AppSpecializeArgs *args) {}
        virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
        virtual void preServerSpecialize(AppSpecializeArgs *args) {}
        virtual void postServerSpecialize(const AppSpecializeArgs *args) {}
    };
}

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(void* api, void* env) { \
        static clazz instance; \
        instance.onLoad((zygisk::Api*)api, (JNIEnv*)env); \
    }

using zygisk::Api;
using zygisk::AppSpecializeArgs;

class FFModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->env = env;
    }
    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (args->nice_name) {
            const char *process = env->GetStringUTFChars(*args->nice_name, nullptr);
            if (strstr(process, "com.dts.freefiremax") != nullptr || strstr(process, "com.dts.freefireth") != nullptr) {
                dlopen("/data/adb/modules/ffbypass/system/lib64/libffhook.so", RTLD_NOW);
            }
            env->ReleaseStringUTFChars(*args->nice_name, process);
        }
    }
private:
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(FFModule)
