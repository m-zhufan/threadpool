QT       -= qt

TARGET = cxx_thread_pool

DEFINES += CXX_THREAD_POOL_LIBRARY
include($$PWD/../libconfig.pri)

SOURCES += \
        cxx_thread_pool.cpp \
    cxx_thread.cpp

HEADERS += \
        cxx_thread_pool.h \
        cxx_thread_pool_global.h \ 
    cxx_thread.h
