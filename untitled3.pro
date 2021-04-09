QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Rectangle.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Rectangle.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Application/opencv/build/x64/vc15/lib/ -lopencv_world3410
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Application/opencv/build/x64/vc15/lib/ -lopencv_world3410d

INCLUDEPATH += $$PWD/../Application/opencv/build/include
DEPENDPATH += $$PWD/../Application/opencv/build/include
