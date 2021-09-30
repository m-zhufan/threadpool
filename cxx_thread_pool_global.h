#ifndef CXX_THREAD_POOL_GLOBAL_H
#define CXX_THREAD_POOL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CXX_THREAD_POOL_LIBRARY)
#  define CXX_THREAD_POOLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CXX_THREAD_POOLSHARED_EXPORT Q_DECL_IMPORT
#endif

#define CXX_THREAD_POOL_BEGIN  namespace _cxx_thread {
#define CXX_THREAD_POOL_END    }


#endif // CXX_THREAD_POOL_GLOBAL_H
