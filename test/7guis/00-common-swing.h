#pragma once

#include "../../src/rxui-swing.h"
#include "../../src/jni/jni.h"

#include <unistd.h>

static int activate();

struct PrintStream_t;

struct System_t {
    static constexpr auto &fqn = const_string("java/lang/System");
};

struct PrintStream_t {
    static constexpr auto &fqn = const_string("java/io/PrintStream");
};

struct String_t {
    static constexpr auto &fqn = const_string("java/lang/String");
};

struct JFrame_t {
    static constexpr auto &fqn = const_string("javax/swing/JFrame");
};

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    (void) activate;

    JavaVMInitArgs args;
    args.version = JNI_VERSION_1_8;
    args.nOptions = 0;
    args.options = new JavaVMOption[1];
    // args.options[0].optionString = const_cast<char *>("-Xcheck:jni");
    args.ignoreUnrecognized = JNI_FALSE;

    JavaVM *jvm;
    jni::Env env{};
    JNI_CreateJavaVM(&jvm, reinterpret_cast<void **>(&env.env), &args);

    auto JFrame = env.FindClass<JFrame_t>();
    auto init = env.GetMethodID<void(String_t)>(JFrame, "<init>");
    auto setVisible = env.GetMethodID<void(jboolean)>(JFrame, "setVisible");
    auto frame = env->NewObject(JFrame, init, env->NewStringUTF("Window"));
    env->CallVoidMethod(frame, setVisible, true);

    sleep(-1);

    return 0;
}

static rxui::Element<> render(State &state, std::function<void()> &refresh);

static int activate()
{
    (void) render;

    /*
    window->setWindowTitle("Window");

    auto rootLayout = new QFormLayout;
    auto w = new QWidget;
    w->setLayout(rootLayout);
    window->setCentralWidget(w);

    auto state = new State;
    auto root = new QtRoot(rootLayout);
    auto refresh = new std::function<void()>{};
    *refresh = [=]() {
        auto element = render(*state, *refresh);
        rxui::render(element, *root);
    };
    (*refresh)();
    auto _refresh = new std::function<void()>{*refresh};
    *refresh = [=]() { QTimer::singleShot(0, *_refresh); };
    window->show();
    */
    return 0;
}
