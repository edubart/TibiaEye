TARGET = TibiaEye
TEMPLATE = app
CONFIG += warn_on \
    qt \
    precompile_header
CONFIG(debug, debug|release) { 
    win32:CONFIG += console
    DEFINES += __DEBUG__
}
win32:RC_FILE = resource.rc
QT += network
PRECOMPILED_HEADER = headers.h
HEADERS += mainwindow.h \
    headers.h \
    fancytabwidget.h \
    watchmoviesview.h \
    recordmoviesview.h \
    optionsview.h \
    aboutdialog.h \
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
    protocollogin.h \
    protocolgame.h \
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
    aboutdialog.cpp \
    moviefile.cpp \
    util.cpp \
    modemanager.cpp \
    memoryinjection.cpp \
    clientproxy.cpp \
    networkmessage.cpp \
    protocol.cpp \
    rsa.cpp \
    protocollogin.cpp \
    protocolgame.cpp \
    serverproxy.cpp \
    servervirtual.cpp \
    tibiasocket.cpp \
    protocolgamerecord.cpp \
    protocolgameplay.cpp
FORMS += ui/optionsview.ui \
    ui/recordmoviesview.ui \
    ui/watchmoviesview.ui
RESOURCES += tibiaeye.qrc
LIBS += -lgmp
OTHER_FILES += resource.rc \
    TODO.txt
