
#include <QCoreApplication>
#include <QtPlugin>

#include "cmdmain.h"
#include "wickrIOServerCommon.h"
#include "wickrIOIPCRuntime.h"

extern bool isVERSIONDEBUG();

void redirectedOutput(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    if (type == QtMsgType::QtDebugMsg) {
        if (str.startsWith("CONSOLE:")) {
            QString outstr = str.right(str.length()-8);
            QTextStream(stdout) << outstr << endl;
        }
    }
}

Q_IMPORT_PLUGIN(QSQLCipherDriverPlugin)

int main(int argc, char *argv[])
{
    qInstallMessageHandler(redirectedOutput);
    QCoreApplication coreapp(argc, argv);

    coreapp.setOrganizationName(WBIO_ORGANIZATION);
    coreapp.setApplicationName(WBIO_CONSOLE_TARGET);
    WickrIOIPCRuntime::init(WBIO_CONSOLE_TARGET, false);

    CmdMain cmdmain(WBIO_CONSOLE_TARGET);
    if (argc > 1) {
        QString commands(argv[1]);
        cmdmain.runCommands(commands);
    } else {
        cmdmain.runCommands();
    }

    WickrIOIPCRuntime::shutdown();
}
