#include <functional>

#if 0
#elif RXUI_BACKEND_EFL
#include "./00-common-efl.h"
#elif RXUI_BACKEND_GTK
#include "./00-common-gtk.h"
#elif RXUI_BACKEND_QT
#include "./00-common-qt.h"
#elif RXUI_BACKEND_SWING
#include "./00-common-swing.h"
#else
#error "no RXUI_BACKEND"
#endif
