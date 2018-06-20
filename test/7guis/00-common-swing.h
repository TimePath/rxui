#pragma once

#include "../../src/rxui-swing.h"
#include "../../src/jni/jni.h"

#include <unistd.h>

static int activate();

namespace javax {
    namespace swing {
        struct JFrame {
            static constexpr auto &fqn = const_string("javax/swing/JFrame");
        };
    }
}

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

    auto JFrame = env.FindClass<javax::swing::JFrame>();
    auto JFrame_new = JFrame.ctor<jstring>();
    auto JFrame_setVisible = JFrame.method<void(jboolean)>("setVisible");

    auto frame = JFrame_new(env->NewStringUTF("Window"));
    JFrame_setVisible(frame, true);

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
