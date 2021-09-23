#pragma once

#include <QtCore/qglobal.h>

#if defined(LAYOUTWATCHER_LIBRARY)
#  define LAYOUTWATCHER_EXPORT Q_DECL_EXPORT
#else
#  define LAYOUTWATCHER_EXPORT Q_DECL_IMPORT
#endif
