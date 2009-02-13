TARGET = TibiaEye
TEMPLATE = app
CONFIG += warn_on \
    qt \
    precompile_header
CONFIG(debug, debug|release) { 
    win32:CONFIG += console
    DEFINES += __DEBUG__
}
win32: RC_FILE = resource.rc
QT += network
PRECOMPILED_HEADER = headers.h
HEADERS += mainwindow.h \
    headers.h \
    fancytabwidget.h \
    watchmoviesview.h \
    recordmoviesview.h \
    optionsview.h \
    aboutview.h \
    constants.h \
    moviefile.h \
    definitions.h \
    util.h \
    modemanager.h \
    singleton.h \
    memoryinjection.h \
    clientproxy.h \
    networkmessage.h \
    consttibia.h \
    protocol.h \
    rsa.h \
    loginprotocol.h \
    gameprotocol.h \
    serverproxy.h \
    abstractserverproxy.h \
    servervirtual.h \
    tibiasocket.h
SOURCES += main.cpp \
    mainwindow.cpp \
    fancytabwidget.cpp \
    watchmoviesview.cpp \
    recordmoviesview.cpp \
    optionsview.cpp \
    aboutview.cpp \
    moviefile.cpp \
    util.cpp \
    modemanager.cpp \
    memoryinjection.cpp \
    clientproxy.cpp \
    networkmessage.cpp \
    protocol.cpp \
    rsa.cpp \
    loginprotocol.cpp \
    gameprotocol.cpp \
    serverproxy.cpp \
    servervirtual.cpp \
    tibiasocket.cpp
FORMS += ui/aboutview.ui \
    ui/optionsview.ui \
    ui/recordmoviesview.ui \
    ui/watchmoviesview.ui
RESOURCES += tibiaeye.qrc
LIBS += -lgmp
OTHER_FILES += resource.rc
