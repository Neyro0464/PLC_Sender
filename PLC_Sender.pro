QT = core
QT += network
CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = $$PWD


SOURCES += \
        ScheduleSaver/DatabaseNoradScheduleSaver.cpp \
        ScheduleSaver/FileNoradScheduleSaver.cpp \
        LIBS/Include/libsgp4/CoordGeodetic.cc \
        LIBS/Include/libsgp4/CoordTopocentric.cc \
        LIBS/Include/libsgp4/DateTime.cc \
        LIBS/Include/libsgp4/DecayedException.cc \
        LIBS/Include/libsgp4/Eci.cc \
        LIBS/Include/libsgp4/Globals.cc \
        LIBS/Include/libsgp4/Observer.cc \
        LIBS/Include/libsgp4/OrbitalElements.cc \
        LIBS/Include/libsgp4/SGP4.cc \
        LIBS/Include/libsgp4/SatelliteException.cc \
        LIBS/Include/libsgp4/SolarPosition.cc \
        LIBS/Include/libsgp4/TimeSpan.cc \
        LIBS/Include/libsgp4/Tle.cc \
        LIBS/Include/libsgp4/Util.cc \
        LIBS/Include/libsgp4/Vector.cc \
        NoradProcessor.cpp \
        SenderPLC/SnmpSender.cpp \
        TleProcessor.cpp \
        main.cpp

# INCLUDEPATH += $$PWD/LIBS/Include/
# DEPENDPATH += $$PWD/LIBS/Include//

# unix:!macx: LIBS += -L$$PWD/LIBS/Bin/ -lsgp4s

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
    ScheduleSaverINoradScheduleSaver.h \
    SenderPLC/ISenderPLC.h \
    LIBS/Include/libsgp4/CoordGeodetic.h \
    LIBS/Include/libsgp4/CoordTopocentric.h \
    LIBS/Include/libsgp4/DateTime.h \
    LIBS/Include/libsgp4/DecayedException.h \
    LIBS/Include/libsgp4/Eci.h \
    LIBS/Include/libsgp4/Globals.h \
    LIBS/Include/libsgp4/Observer.h \
    LIBS/Include/libsgp4/OrbitalElements.h \
    LIBS/Include/libsgp4/SGP4.h \
    LIBS/Include/libsgp4/SatelliteException.h \
    LIBS/Include/libsgp4/SolarPosition.h \
    LIBS/Include/libsgp4/TimeSpan.h \
    LIBS/Include/libsgp4/Tle.h \
    LIBS/Include/libsgp4/TleException.h \
    LIBS/Include/libsgp4/Util.h \
    LIBS/Include/libsgp4/Vector.h \
    NoradProcessor.h \
    SenderPLC/SnmpSender.h \
    TleProcessor.h

