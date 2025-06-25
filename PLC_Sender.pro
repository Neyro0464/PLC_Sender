QT = core
QT += network
CONFIG += c++17 cmdline

DESTDIR = $$PWD
INCLUDEPATH += $$PWD/LIBS/Include/
DEPENDPATH += $$PWD/LIBS/Include//

unix:!macx: LIBS += -L$$PWD/LIBS/Bin/ -lsgp4s
LIBS += -lssh

SOURCES += \
        ScheduleSaver/DatabaseNoradScheduleSaver.cpp \
        ScheduleSaver/FileNoradScheduleSaver.cpp \
        NoradProcessor.cpp \
        FileSender/SftpFileSender.cpp \
        TleProcessor.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    # LIBS/Bin/libsgp4s.so \
    TLE.txt \
    settings.ini \

HEADERS += \
    ScheduleSaver/DatabaseNoradScheduleSaver.h \
    ScheduleSaver/FileNoradScheduleSaver.h \
    ScheduleSaver/INoradScheduleSaver.h \
    NoradProcessor.h \
    FileSender/IFileSenderPLC.h \
    FileSender/SftpFileSender.h \
    TleProcessor.h

