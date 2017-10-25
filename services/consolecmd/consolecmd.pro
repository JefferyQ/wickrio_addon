DEPTH = ../..
COMMON = $${DEPTH}/shared/common
CONSOLESRC = ../console

#
# Include the Wickr IO services base
#
include(../services.pri)

wickr_messenger {
    DEFINES += WICKR_MESSENGER=1
}
else:wickr_blackout {
    DEFINES += WICKR_BLACKOUT=1
}
else:wickr_enterprise {
    DEFINES += WICKR_ENTERPRISE=1
}
else:wickr_scif {
    DEFINES += WICKR_SCIF=1
}


CONFIG(release,release|debug) {
    wickr_beta {
        DEFINES += WICKR_BETA
    }
    else {
        DEFINES += WICKR_PRODUCTION
    }
}
else {
    wickr_beta {
        DEFINES += WICKR_BETA
    }
    else:wickr_qa {
        DEFINES += WICKR_QA
    }
    else {
        DEFINES += WICKR_ALPHA
    }

    DEFINES += WICKR_DEBUG
}

#
# Include the WickrIO common files
#
include($${COMMON}/common.pri)

CONFIG += c++11

CONFIG(release,release|debug) {
    message(*** WickrIO Console Release Build)
    TARGET = WickrIOConsoleCmd
    SOURCES += $${COMMON}/versiondebugNO.cpp

    macx {
        ICON = $$COMMON/Wickr_prod.icns
        QMAKE_INFO_PLIST = $$DEPTH/ConsumerProd.Info.plist
    }
    win32 {
        RC_FILE = $$COMMON/Wickr.rc
        ICON = $$COMMON/Wickr.ico
    }
}
else {
    message(*** WickrIO Console Beta Build)
    TARGET = WickrIOConsoleCmdDebug

    SOURCES += $${COMMON}/versiondebugYES.cpp
    DEFINES += VERSIONDEBUG

    macx {
        ICON = $$COMMON/Wickr_beta.icns
        QMAKE_INFO_PLIST = $$DEPTH/ConsumerBeta.Info.plist
    }
    win32 {
        RC_FILE = $$COMMON/Wickr-beta.rc
        ICON = $$COMMON/Wickr-beta.ico
    }
}

#
# Include the WickrIO common server files
#
SERVER_COMMON=../common
INCLUDEPATH += $${SERVER_COMMON}

#
# Include the Wickr Console library
#
include($${DEPTH}/libs/WickrIOConsole/WickrIOConsole.pri)

#
# Include the Wickr library
#
include($${DEPTH}/libs/WickrIOLib/WickrIOLib.pri)

INCLUDEPATH += $${CONSOLESRC}

QT -= gui
QT += network
QT += sql

HEADERS += \
    $${COMMON}/cmdbase.h

SOURCES += \
    $${COMMON}/cmdbase.cpp \
    main.cpp

TEMPLATE = app

CONFIG += depend_includepath

# qsqlcipher_wickr

win32 {
    CONFIG(debug, debug|release):LIBPATH += $$DEPTH/wickr-sdk/libs/qsqlcipher_wickr/debug
    else:LIBPATH += $$DEPTH/wickr-sdk/libs/qsqlcipher_wickr/release
} else {
    LIBPATH += $$DEPTH/wickr-sdk/libs/qsqlcipher_wickr/
}
LIBS += -lqsqlcipher_wickr

# sqlcipher

LIBS += -lsqlcipher
