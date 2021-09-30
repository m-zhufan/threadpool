TEMPLATE = lib

CONFIG += c++11 warn_on

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#INCLUDEPATH +=
#LIBS += -L

#DESTDIR =
