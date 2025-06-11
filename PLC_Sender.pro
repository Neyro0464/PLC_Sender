QT = core
QT += network
CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = $$PWD
INCLUDEPATH += $$PWD/LIBS/Include/
DEPENDPATH += $$PWD/LIBS/Include//

unix:!macx: LIBS += -L$$PWD/LIBS/Bin/ -lsgp4s

SOURCES += \
        ScheduleSaver/DatabaseNoradScheduleSaver.cpp \
        ScheduleSaver/FileNoradScheduleSaver.cpp \
        NoradProcessor.cpp \
        SenderPLC/SnmpSender.cpp \
        TleProcessor.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    # LIBS/Bin/libsgp4s.so \
    TLE.txt \
    settings.ini

HEADERS += \
    ScheduleSaver/DatabaseNoradScheduleSaver.h \
    ScheduleSaver/FileNoradScheduleSaver.h \
    ScheduleSaver/INoradScheduleSaver.h \
    SenderPLC/ISenderPLC.h \
    NoradProcessor.h \
    SenderPLC/SnmpSender.h \
    TleProcessor.h

