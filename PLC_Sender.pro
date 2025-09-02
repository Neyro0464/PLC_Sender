QT = core
QT += network
CONFIG += c++17 cmdline

DESTDIR = $$PWD
INCLUDEPATH += $$PWD/LIBS/Include/
DEPENDPATH += $$PWD/LIBS/Include//

unix:!macx: LIBS += -L$$PWD/LIBS/Bin/ -lsgp4s
LIBS += -lssh

SOURCES += \
        FileSender/UdpSender.cpp \
        Receiver/QueryHandler.cpp \
        Receiver/UdpListener.cpp \
        ScheduleSaver/FileNoradScheduleSaver.cpp \
        NoradProcessor.cpp \
        # FileSender/SftpFileSender.cpp \
        ServerConfig.cpp \
        TleProcessor.cpp \
        # Utils/UtilResponseParser.cpp \
        Utils/Utility.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    # LIBS/Bin/libsgp4s.so \
    ServiceFiles/settings.ini \
    setup.sh

HEADERS += \
    FileSender/UdpSender.h \
    Receiver/QueryHandler.h \
    Receiver/UdpListener.h \
    ScheduleSaver/FileNoradScheduleSaver.h \
    ScheduleSaver/INoradScheduleSaver.h \
    NoradProcessor.h \
    ServerConfig.h \
    TleProcessor.h \
    # Utils/UtilResponseParser.h \
    Utils/Utility.h

