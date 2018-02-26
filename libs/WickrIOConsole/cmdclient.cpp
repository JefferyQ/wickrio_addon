#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include "cmdclient.h"

#include "wickrIOCommon.h"
#include "wickrIOServerCommon.h"
#include "wickrbotsettings.h"
#include "consoleserver.h"
#include "wickrIOConsoleClientHandler.h"

CmdClient::CmdClient(CmdOperation *operation) :
    m_operation(operation)
{
}

bool CmdClient::processCommand(QStringList cmdList)
{
    bool retVal = true;
    bool bForce = false;

    QString cmd = cmdList.at(0).toLower();
    int clientIndex;

    // Convert the second argument to an integer, for the client index commands
    if (cmdList.size() > 1) {
        bool ok;
        clientIndex = cmdList.at(1).toInt(&ok);
        if (!ok) {
            qDebug() << "CONSOLE:Client Index is not a number!";
            return true;
        }

        // See if the force option is set
        if (cmdList.size() == 3) {
            if (cmdList.at(2) == "-force" || cmdList.at(2) == "force") {
                bForce = true;
            }
        }
    } else {
        clientIndex = -1;
    }

    if (cmd == "?" || cmd == "help") {
        qDebug() << "CONSOLE:Commands:";
        qDebug() << "CONSOLE:  add        - adds a new client";
        qDebug() << "CONSOLE:  back       - leave the clients setup";
        qDebug() << "CONSOLE:  delete <#> - deletes client with the specific index";
        qDebug() << "CONSOLE:  help or ?  - shows supported commands";
        qDebug() << "CONSOLE:  list       - shows a list of clients";
        qDebug() << "CONSOLE:  modify <#> - modifies a client with the specified index";
        qDebug() << "CONSOLE:  pause <#>  - pauses the client with the specified index";
        qDebug() << "CONSOLE:  start <#>  - starts the client with the specified index";
        qDebug() << "CONSOLE:  quit       - leaves this program";
    } else if (cmd == "add") {
        addClient();
    } else if (cmd == "back") {
        retVal = false;
    } else if (cmd == "delete") {
        if (clientIndex == -1) {
            qDebug() << "CONSOLE:Usage: delete <index>";
        } else {
            deleteClient(clientIndex);
        }
    } else if (cmd == "list") {
        listClients();
    } else if (cmd == "modify") {
        if (clientIndex == -1) {
            qDebug() << "CONSOLE:Usage: modify <index>";
        } else {
            modifyClient(clientIndex);
        }
    } else if (cmd == "pause") {
        if (clientIndex == -1) {
            qDebug() << "CONSOLE:Usage: pause <index>";
        } else {
            pauseClient(clientIndex, bForce);
        }
    } else if (cmd == "quit") {
        retVal = false;
    } else if (cmd == "start") {
        if (clientIndex == -1) {
            qDebug() << "CONSOLE:Usage: start <index>";
        } else {
            startClient(clientIndex, bForce);
        }
    } else {
        qDebug() << "CONSOLE:" << cmd << "is not a known command!";
    }
    return retVal;
}

/**
 * @brief CmdClient::runCommands
 * This function handles the setup of the different clients running on the system.
 * There can be multiple clients on a system. The user add new clients or modify
 * the configuration of existing clients.  Only clients in the paused state can be
 * modified. The user can start or pause a client as well from this function.
 */
bool CmdClient::runCommands(QString commands)
{
    QTextStream input(stdin);

    // If the database location is not set then get it
    if (! m_operation->openDatabase()) {
        qDebug() << "CONSOLE:Cannot open database!";
        return true;
    }

    // Get the current SSL Settings
    if (m_operation->m_settings->childGroups().contains(WBSETTINGS_SSL_HEADER)) {
        // Get the email settings from the settings file
        m_operation->m_settings->beginGroup(WBSETTINGS_SSL_HEADER);
        m_sslSettings.readFromSettings(m_operation->m_settings);
        m_operation->m_settings->endGroup();
    } else {
        m_sslSettings.sslKeyFile = "";
        m_sslSettings.sslCertFile = "";
    }

    // Get the data from the database
    m_clients = m_operation->m_ioDB->getClients();

    if (commands.isEmpty()) {
        while (true) {
            qDebug() << "CONSOLE:Enter client command:";
            QString line = input.readLine();

            line = line.trimmed();
            if (line.length() > 0) {
                QStringList args = line.split(" ");
                if (!processCommand(args)) {
                    break;
                }

            }
        }
    } else {
        QStringList cmds = commands.trimmed().split(" ");
        processCommand(cmds);
    }
    return true;
}

void CmdClient::status()
{
    listClients();
}

/**
 * @brief CmdClient::listClients
 * This funciton will print out a list of the current clients from the database.
 */
void CmdClient::listClients()
{
    // Update the list of clients
    m_clients = m_operation->m_ioDB->getClients();

    int cnt=0;
    for (WickrBotClients *client : m_clients) {
        QString processName = WBIOServerCommon::getClientProcessName(client);
        QString clientState = WickrIOConsoleClientHandler::getActualProcessState(processName, m_operation->m_ioDB);
        WickrIOConsoleUser cUser;
        QString consoleUser;

        if (m_operation->m_ioDB->getConsoleUser(client->console_id, &cUser)) {
            consoleUser = cUser.user;
        } else {
            consoleUser = "Not set";
        }

        QString data = QString("CONSOLE: client[%1] %2, APIKey=%3, User=%4, Port=%5, Binary=%6, State=%7, ConsoleUser=%8")
            .arg(cnt++)
            .arg(client->name)
            .arg(client->apiKey)
            .arg(client->user)
            .arg(client->port)
            .arg(client->binary)
            .arg(clientState)
            .arg(consoleUser);
        qDebug() << qPrintable(data);
    }
}

/**
 * @brief CmdClient::chkClientsNameExists
 * Check if the input name is already used by one of the client records
 * @param name
 * @return
 */
bool CmdClient::chkClientsNameExists(const QString& name)
{
    for (WickrBotClients *client : m_clients) {
        if (client->name == name) {
            qDebug() << "CONSOLE:The input name is NOT unique!";
            return true;
        }
    }
    return false;
}

/**
 * @brief CmdClient::chkClientsUserExists
 * Check if the input user is already used by one of the client records
 * @param name
 * @return
 */
bool CmdClient::chkClientsUserExists(const QString& user)
{
#if 0
    for (WickrBotClients *client : m_clients) {
        if (client->user == user) {
            qDebug() << "CONSOLE:The input user name is NOT unique!";
            return true;
        }
    }
    return false;
#else
    // Allow duplicates for now
    return false;
#endif
}

/**
 * @brief CmdClient::chkClientsInterfaceExists
 * Check if the input interface and port combination is already used
 * by one of the client records
 * @param name
 * @return
 */
bool CmdClient::chkClientsInterfaceExists(const QString& iface, int port)
{
    // TODO: Make sure not using the same port as the console server

    for (WickrBotClients *client : m_clients) {
        if (client->port == port && (client->iface == iface || client->iface == "localhost" || iface == "localhost" )) {
            qDebug() << "CONSOLE:The input interface and port are NOT unique!";
            return true;
        }
    }
    return false;
}

bool CmdClient::getClientValues(WickrBotClients *client)
{
    bool quit = false;
    QString temp;

    // Get a unique client name
    do {
        temp = getNewValue(client->name, tr("Enter an unique Name for this client (this is a local name, not the Wickr ID)"));

        // Check if the user wants to quit the action
        if (handleQuit(temp, &quit) && quit) {
            return false;
        }

        // Allow the user to re-use the same name if it was previously set
        if (! client->name.isEmpty()  && client->name == temp) {
            break;
        }
    } while (chkClientsNameExists(temp));
    client->name = temp;

    // Determine the client type
    QStringList possibleClientTypes = WBIOServerCommon::getAvailableClientApps();
    QString binary;
    if (possibleClientTypes.length() > 1) {
        temp = getNewValue(client->binary, tr("Enter the client type"), CHECK_LIST, possibleClientTypes);
        // Check if the user wants to quit the action
        if (handleQuit(temp, &quit) && quit) {
            return false;
        }
        binary = temp;
    } else {
        binary = possibleClientTypes.at(0);
    }
    client->binary = binary;

    // Time to configure the BOT's username

    // Determine if the BOT has an executable that will create/provision the user

    QString provisionApp = WBIOServerCommon::getProvisionApp(binary);
    if (provisionApp != nullptr) {
        QString clientDbDir;
        QString command = client->binary;
        QStringList arguments;

        QString configFileName = QString(WBIO_CLIENT_SETTINGS_FORMAT).arg(m_operation->m_dbLocation).arg(client->name);
        clientDbDir = QString("%1/clients/%2/client").arg(m_operation->m_dbLocation).arg(client->name);

        arguments.append(QString("-config=%1").arg(configFileName));
        arguments.append(QString("-clientdbdir=%1").arg(clientDbDir));
        arguments.append(QString("-processname=%1").arg(WBIOServerCommon::getClientProcessName(client)));

        m_exec = new QProcess();

        connect(m_exec, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotCmdFinished));
        connect(m_exec, SIGNAL(finished(int, QProcess::readyReadStandardOutput)), this, SLOT(slotCmdOutputRx));

        m_exec->start(command, arguments);

        if (m_exec->waitForStarted(-1)) {
            m_exec->waitForFinished(-1);
        } else {
            QByteArray errorout = m_exec->readAllStandardError();
            if (!errorout.isEmpty()) {
                qDebug() << "ERRORS" << errorout;
            }
            qDebug() << "Exit code=" << m_exec->exitCode();
        }
        m_exec->close();

    } else {
        // Get a unique user name
        do {
            temp = getNewValue(client->user, tr("Enter the user name"));

            // Check if the user wants to quit the action
            if (handleQuit(temp, &quit) && quit) {
                return false;
            }

            // Allow the user to re-use the same name if it was previously set
            if (! client->user.isEmpty()  && client->user == temp) {
                break;
            }
        } while (chkClientsUserExists(temp));
        client->user = temp;

        // Get the password
        while (true) {
            client->password = getNewValue(client->password, tr("Enter the password"));

            // Check if the user wants to quit the action
            if (handleQuit(client->password, &quit) && quit) {
                return false;
            }

            if (client->password.isEmpty() || client->password.length() < 4) {
                qDebug() << "CONSOLE:Password should be at least 4 characters long!";
            } else {
                break;
            }
        }
    }

    // Get the API Key value.  Does not currently have to be unique
    while (true) {
        client->apiKey = getNewValue(client->apiKey, tr("Enter the API Key"));

        // Check if the user wants to quit the action
        if (handleQuit(client->apiKey, &quit) && quit) {
            return false;
        }

        if (client->apiKey.isEmpty() || client->apiKey.length() < 4) {
            qDebug() << "CONSOLE:API Key should be at least 4 characters long!";
        } else {
            break;
        }
    }

    // Get the list of possible interfaces
    QStringList ifaceList = WickrIOConsoleClientHandler::getNetworkInterfaceList();

    // Get a unique interface and port pair
    while (true) {
        QString ifaceinput = getNewValue(client->iface, tr("Enter the interface (list to see possible interfaces)"));

        // Check if the user wants to quit the action
        if (handleQuit(ifaceinput, &quit) && quit) {
            return false;
        }

        if (ifaceinput.toLower() == "list" || ifaceinput == "?") {
            foreach (QString iface, ifaceList) {
                qDebug() << "CONSOLE:" << iface;
            }
            continue;
        }

        if (!ifaceList.contains(ifaceinput)) {
            qDebug() << "CONSOLE:" << ifaceinput << "not found in the list of interfaces!";
            continue;
        }


        QString port = getNewValue(QString::number(client->port), tr("Enter the IP port number"), CHECK_INT);

        // Check if the user wants to quit the action
        if (handleQuit(port, &quit) && quit) {
            return false;
        }

        int portinput = port.toInt();

        // Check if used the same values, if so continue on to the next field
        if (!client->iface.isEmpty() && (client->iface == ifaceinput && client->port == portinput)) {
            break;
        }

        if (chkClientsInterfaceExists(ifaceinput, portinput)) {
            qDebug() << "CONSOLE:Interface and port combination used already!";
            continue;
        }

        client->iface = ifaceinput;
        client->port = portinput;

        break;
    }

    // Get the interface type (HTTP or HTTPS)
    while (true) {
        QString ifacetype;
        ifacetype = getNewValue(client->getIfaceTypeStr(), tr("Enter the interface type"));
        if (ifacetype.toLower() == "https") {
            client->isHttps = true;
            if (m_sslSettings.sslKeyFile.isEmpty() || m_sslSettings.sslCertFile.isEmpty()) {
                qDebug() << "CONSOLE:WARNING: SSL has not been setup! Go to the Advanced settings first!";
            }
        } else if (ifacetype.toLower() == "http") {
            client->isHttps = false;
        } else {
            qDebug() << "CONSOLE:Invalid interface type, enter either HTTP or HTTPS";
            continue;

        }
        break;
    }

    // Get the SSL Key and Cert values
    if (client->isHttps) {
        client->sslKeyFile = m_sslSettings.sslKeyFile;
        client->sslCertFile = m_sslSettings.sslCertFile;
    }

    QList<WickrIOConsoleUser *> cusers = m_operation->m_ioDB->getConsoleUsers();
    if (cusers.length() > 0) {
        int curindex;
        if (client->console_id != 0) {

        } else {
            curindex = -1;
        }
        QString temp;
        temp = getNewValue(curindex == -1 ? "n" : "y", tr("Associate with a console user?"));
        if (handleQuit(temp, &quit) && quit) {
        } else if (temp == "y") {
            // Associate with a console user
            while (true) {
                qDebug() << "CONSOLE:Possible console user choices:";
                int cnt=0;
                for (WickrIOConsoleUser *cuser : cusers) {
                    QString data = QString("CONSOLE:  index=%1, Name=%2").arg(cnt++).arg(cuser->user);
                    qDebug() << qPrintable(data);
                }
                temp = getNewValue(curindex == -1 ? "" : QString::number(curindex), tr("Enter the index of the console user"), CHECK_INT);
                if (handleQuit(temp, &quit) && quit) {
                    break;
                }
                int inputIndex = temp.toInt();
                if (inputIndex < 0 || inputIndex >= cusers.length()) {
                    qDebug() << "CONSOLE:Invalid index value entered!";
                } else {
                    curindex = inputIndex;
                    break;
                }
            }
        } else {
            curindex = -1;
        }

        // If the user is NOT quitting then update the console ID
        if (!quit) {
            if (curindex == -1) {
                client->console_id = 0;
            } else {
                client->console_id = cusers.at(curindex)->id;
            }
        }

        // Cleanup the allocated memory
        for (WickrIOConsoleUser *cuser : cusers) {
            delete cuser;
        }
    }

    // Get the Handle Inbox messages setting
    while (true) {
        QString handleInbox;
        QStringList trueFalseChoices;
        trueFalseChoices << "true" << "false";
        handleInbox = getNewValue(client->getHandleInboxStr(), tr("Does client support inbox handling?"), CHECK_LIST, trueFalseChoices);
        if (handleInbox.toLower() == "true") {
            client->m_handleInbox = true;
        } else if (handleInbox.toLower() == "false") {
            client->m_handleInbox = false;
        } else {
            qDebug() << "CONSOLE:Invalid input, enter either true or false";
            continue;

        }
        break;
    }


    /*
     * setup an integration bot, if the users desires to use one
     */
    QList<WBIOBotTypes *>botTypes = WBIOServerCommon::getBotsSupported(binary);
    if (botTypes.length() > 0) {
        // If the user wants to connect the client to an integration bot
        temp = getNewValue("no", tr("Do you want to connect to a integration bot?"), CHECK_BOOL);
        if (temp == "yes") {
            QStringList possibleBotTypes;
            for (WBIOBotTypes *bt : botTypes) {
                possibleBotTypes.append(bt->m_name);
            }
            possibleBotTypes.append("none");

            temp = getNewValue(client->botType, tr("Enter the bot type"), CHECK_LIST, possibleBotTypes);
            // Check if the user wants to quit the action
            if (handleQuit(temp, &quit) && quit) {
                return false;
            }
            if (temp != "none")
                client->botType = temp;
            else
                client->botType = QString();
        }
    }


    /*
     * Need to configure the bot if one was selected
     */
    if (! client->botType.isEmpty()) {
        // Get the software into the appropriate location
        QString swPath = WBIOServerCommon::getBotSoftwarePath(client->botType);
        if (!swPath.isEmpty()) {
            qDebug() << "CONSOLE:**********************************************************************";
            qDebug().noquote().nospace() << "CONSOLE:Begin setup of " << client->botType << " software for " << client->name;

            QString destPath = QString(WBIO_CLIENT_BOTDIR_FORMAT)
                    .arg(WBIO_DEFAULT_DBLOCATION)
                    .arg(client->name)
                    .arg(client->botType);

            // Create the directory for the integration software
            QDir destDir(destPath);
            if (!destDir.exists()) {
                if (!destDir.mkpath(destPath)) {
                    qDebug() << "CONSOLE:Failed to create directory for integration bot software!";
                    return false;
                }
            }

            // First copy the software to the client directory
            qDebug().noquote().nospace() << "CONSOLE:Copying " << client->botType << " software for " << client->name;

            {
                // Create a process to extract the software
                QProcess *unarchive = new QProcess(this);

                QString command = QString("tar -xf %1 -C %2").arg(swPath).arg(destPath);
                unarchive->setProcessChannelMode(QProcess::MergedChannels);
                unarchive->start(command, QIODevice::ReadWrite);

                // Wait for it to start
                if(!unarchive->waitForStarted()) {
                    qDebug() << QString("CONSOLE:Failed to install %1 software!").arg(client->botType);
                    return false;
                }

                // Continue reading the data until EOF reached
                QByteArray data;

                while(unarchive->waitForReadyRead())
                    data.append(unarchive->readAll());

                // Output the data
                qDebug().noquote().nospace() << "CONSOLE:" << data;
            }

            // Second peform the installer if one does exist
            qDebug().noquote().nospace() << "CONSOLE:Installing " << client->botType << " software for " << client->name;
            {
                QStringList installOutput;
                QString installer = WBIOServerCommon::getBotInstaller(client->botType);
                if (!installer.isEmpty()){
                    QString installerFullPath = QString("%1/%2").arg(destPath).arg(installer);
                    // Create a process to run the installer
                    QProcess *runInstaller = new QProcess(this);
                    runInstaller->setProcessChannelMode(QProcess::MergedChannels);
                    runInstaller->setWorkingDirectory(destPath);
                    runInstaller->start(installerFullPath, QIODevice::ReadWrite);

                    // Wait for it to start
                    if(!runInstaller->waitForStarted()) {
                        qDebug() << QString("CONSOLE:Failed to run %1").arg(installer);
                        return false;
                    }

                    while(runInstaller->waitForReadyRead()) {
                        QString bytes = QString(runInstaller->readAll());
                        installOutput.append(bytes);
                    }
                }
            }

            // Third peform the configure if one does exist
            qDebug().noquote().nospace() << "CONSOLE:Begin configuration of " << client->botType << " software for " << client->name;
            QString configure = WBIOServerCommon::getBotConfigure(client->botType);
            if (!configure.isEmpty()){
                QString configureFullpath = QString("%1/%2").arg(destPath).arg(configure);
                if (!runBotScript(destPath, configureFullpath, client)) {
                    qDebug().noquote().nospace() << "CONSOLE:Failed to configure " << client->botType;
                    return false;
                }
            }

            qDebug().noquote().nospace() << "CONSOLE:End of setup of " << client->botType << " software for " << client->name;
            qDebug() << "CONSOLE:**********************************************************************";

        }
    }


    return !quit;
}

bool
CmdClient::getAuthToken(WickrBotClients *client, QString& authToken)
{
    WickrIOTokens token;

    // get the id of the console user to use for authentication
    if (client->console_id == 0) {
        QList<WickrIOConsoleUser *> cusers;
        cusers = m_operation->m_ioDB->getConsoleUsers();

        if (cusers.length() == 0) {
            qDebug() << "CONSOLE:There are not console users defined.  Please create a console user and a token.";
            return false;
        }

        qDebug() << "CONSOLE:You will need to select a console user to use their authentication token";
        qDebug() << "CONSOLE:Please select from the following list of console users:";

        int curindex = -1;
        QString temp;
        bool quit=false;
        // Associate with a console user
        while (true) {
            qDebug() << "CONSOLE:Possible console user choices:";
            int cnt=0;
            for (WickrIOConsoleUser *cuser : cusers) {
                QString data = QString("CONSOLE:  index=%1, Name=%2").arg(cnt++).arg(cuser->user);
                qDebug() << qPrintable(data);
            }
            temp = getNewValue(curindex == -1 ? "" : QString::number(curindex), tr("Enter the index of the console user"), CHECK_INT);
            if (handleQuit(temp, &quit) && quit) {
                break;
            }
            int inputIndex = temp.toInt();
            if (inputIndex < 0 || inputIndex >= cusers.length()) {
                qDebug() << "CONSOLE:Invalid index value entered!";
            } else {
                curindex = inputIndex;
                break;
            }
        }

        if (quit || curindex == -1)
            return false;

        client->console_id = cusers.at(curindex)->id;

        // Cleanup the allocated memory
        for (WickrIOConsoleUser *cuser : cusers) {
            delete cuser;
        }
    }

    if (! m_operation->m_ioDB->getConsoleUserToken(client->console_id, &token)) {
        qDebug() << "CONSOLE:There is no token associated with that console user!";
        return false;
    }

    authToken = token.token;
    return true;
}

/**
 * @brief CmdClient::runBotScript
 * This function will run the input script. The script has commands in it to identify if input
 * is required.
 * @param destPath
 * @param configure
 * @return
 */
bool
CmdClient::runBotScript(const QString& destPath, const QString& configure, WickrBotClients *client)
{
    // Values associated with the CallbackURL
    QString cbackEndPoint;
    QString cbackPort;

    // Create a process to run the configure
    QProcess *runScript = new QProcess(this);

    connect(runScript, &QProcess::errorOccurred, [=](QProcess::ProcessError error) {
        qDebug() << "CONSOLE:error enum val = " << error;
    });
    runScript->setProcessChannelMode(QProcess::MergedChannels);
    runScript->setWorkingDirectory(destPath);
    runScript->start(configure, QIODevice::ReadWrite);

    // Wait for it to start
    if(!runScript->waitForStarted()) {
        qDebug() << QString("CONSOLE:Failed to run %1").arg(configure);
        return false;
    }

    while(runScript->waitForReadyRead(-1)) {
        while(runScript->canReadLine()) {
            QString bytes = QString(runScript->readLine());
            if (!bytes.isEmpty()) {
                QStringList inputList = bytes.split(":");

                if (inputList.size() > 1 && inputList[0].toLower() == "prompt") {
                    bool promptForValue = false;
                    if (inputList.size() == 3) {
                        QString prompt = inputList[2].remove(QRegExp("[\\n\\t\\r]")).replace(" ", "");

                        if (prompt == "WICKRIO_AUTH_TOKEN") {
                            QString authToken;
                            if (!getAuthToken(client, authToken)) {
                                runScript->close();
                                return false;
                            }

                            authToken.append("\n");
                            runScript->write(authToken.toLatin1());
                        } else if (prompt == "WICKRIO_SERVER") {
                            QString server = QString("%1://%2:%3/Apps/%5\n")
                                    .arg(client->isHttps ? "https" : "http")
                                    .arg(client->iface)
                                    .arg(client->port)
                                    .arg(client->apiKey);
                            runScript->write(server.toLatin1());
                        } else if (prompt == "HUBOT_NAME") {
                            QString botName = QString("%1_hubot\n").arg(client->name);
                            runScript->write(botName.toLatin1());
                        } else if (prompt == "HUBOT_URL_ENDPOINT") {
                            cbackEndPoint = QString("Apps/%1").arg(client->port);
                            QString endpoint = QString("%1\n").arg(cbackEndPoint);
                            runScript->write(endpoint.toLatin1());
                        } else if (prompt == "HUBOT_URL_PORT") {
                            QString cbackPortPrompt = QString("Enter the port the %1 integration will listen on").arg(client->botType);
                            cbackPort = getNewValue(cbackPort, cbackPortPrompt);
                            QString outString = QString("%1\n").arg(cbackPort);
                            runScript->write(outString.toLatin1());
                        } else {
                            promptForValue = true;
                        }
                    } else {
                        promptForValue = true;
                    }

                    // If there was no known token asked for then prompt to the user
                    if (promptForValue) {
                        QString prompt = bytes.right(bytes.length()-7).remove(QRegExp("[\\n\\t\\r]"));     // size of string - sizeof "PROMPT:"
                        QString curVal;
                        prompt = prompt.remove(QRegExp("[\\n\\t\\r]"));
//                        prompt = QString("Enter value for %1").arg(prompt.toLower());
                        QString input = getNewValue(curVal, prompt);
                        input.append("\n");
                        runScript->write(input.toLatin1());
                    }
                } else {
                    bytes = bytes.remove(QRegExp("[\\n\\t\\r]"));
                    qDebug().noquote().nospace() << "CONSOLE:" << bytes;
                }
            }
        }
    }

    // If the callback valuee are set then create the callback url for this client
    if (!cbackEndPoint.isEmpty() && !cbackPort.isEmpty()) {
        client->m_callbackString = QString("http://localhost:%1/%2").arg(cbackPort).arg(cbackEndPoint);
    }
    return true;
}

void CmdClient::slotCmdFinished(int, QProcess::ExitStatus)
{
    qDebug() << "process completed";
    m_exec->deleteLater();
    m_exec = nullptr;
}

void CmdClient::slotCmdOutputRx()
{
    QByteArray output = m_exec->readAll();

    qDebug() << output;

    QTextStream s(stdin);
    QString lineInput = s.readLine();
    QByteArray input(lineInput.toUtf8());
    m_exec->write(input);
}


/**
 * @brief CmdClient::addClient
 * This function will add a new client. The user will be prompted for the
 * appropriate fields for the new user.
 */
void CmdClient::addClient()
{
    WickrBotClients *client = new WickrBotClients();

    while (true) {
        if (!getClientValues(client)) {
            break;
        }

        // Add the new record to the database
        QString errorMsg = WickrIOConsoleClientHandler::addClient(m_operation->m_ioDB, client);
        if (errorMsg.isEmpty()) {
            if (!client->m_callbackString.isEmpty()) {
                WickrIOAppSettings appSetting;
                appSetting.clientID = client->id;
                appSetting.type = DB_APPSETTINGS_TYPE_MSGRECVCALLBACK;
                appSetting.value = client->m_callbackString;
                if (!m_operation->m_ioDB->updateAppSetting(&appSetting)) {
                    qDebug() << "CONSOLE:Failed to create callback connection!";
                }
            }
            qDebug() << "CONSOLE:Successfully added record to the database!";
            break;
        } else {
            qDebug() << "CONSOLE:" << errorMsg;
            // If the record was not added to the database then ask the user to try again
            QString response = getNewValue("", tr("Failed to add record, try again?"));
            if (response.isEmpty() || response.toLower() == "n") {
                delete client;
                return;
            }
        }
    }
    delete client;

    // Update the list of clients
    m_clients = m_operation->m_ioDB->getClients();
}



/**
 * @brief CmdClient::validateIndex
 * Validate the input index. If invalid output a message
 * @param clientIndex
 */
bool CmdClient::validateIndex(int clientIndex)
{
    if (clientIndex >= m_clients.length() || clientIndex < 0) {
        qDebug() << "CONSOLE:The input client index is out of range!";
        return false;
    }
    return true;
}

/**
 * @brief CmdClient::deleteClient
 * This function is used to delete a client.  The client must be in the paused
 * state first before it can be deleted.
 * @param clientIndex
 */
void CmdClient::deleteClient(int clientIndex)
{
    if (validateIndex(clientIndex)) {
        WickrBotClients *client = m_clients.at(clientIndex);

        // Make sure the user wants to continue
        QString prompt = QString(tr("Do you really want to remove the client with the name %1")).arg(client->name);
        QString response = getNewValue("", prompt);
        if (response.toLower() == "n") {
            return;
        }

        WickrBotProcessState state;
        QString processName = WBIOServerCommon::getClientProcessName(client);
        if (m_operation->m_ioDB->getProcessState(processName, &state)) {
            if (state.ipc_port == 0) {
                qDebug() << "CONSOLE:Client does not have an IPC port defined, will not be able to stop WickrIO Client process!";
                QString response = getNewValue("", "Do you want to continue? (y or n)");
                if (response.toLower() == "n") {
                    return;
                }
            } else if (state.state == PROCSTATE_RUNNING) {
                qDebug() << "CONSOLE:The client is running, you should first Pause the client, then delete!";
                QString response = getNewValue("", "Do you want to continue? (y or n)");
                if (response.toLower() == "n") {
                    return;
                }
            }

            qDebug() << "CONSOLE:Deleting client" << client->name;

            if (! m_operation->m_ioDB->deleteClientUsingName(m_clients.at(clientIndex)->name)) {
                qDebug() << "CONSOLE:There was a problem deleting the client!";
            }
            if (! m_operation->m_ioDB->deleteProcessState(processName)) {
                qDebug() << "CONSOLE:There was a problem deleting the client's process record!";
            }

            // TODO: Need to cleanup the clients directory and registry (for windows)

            // Update the list of clients
            m_clients = m_operation->m_ioDB->getClients();
        }
    }
}

/**
 * @brief CmdClient::modifyClient
 * This function will modify an exist client.
 * @param clientIndex
 */
void CmdClient::modifyClient(int clientIndex)
{
    if (validateIndex(clientIndex)) {
        WickrBotClients *client;
        WickrBotProcessState state;
        client = m_clients.at(clientIndex);

        QString processName = WBIOServerCommon::getClientProcessName(client);
        if (m_operation->m_ioDB->getProcessState(processName, &state)) {
            if (state.state == PROCSTATE_RUNNING) {
                qDebug() << "CONSOLE:Cannot modify a running client!";
            } else {
                while (true) {
                    if (!getClientValues(client)) {
                        break;
                    }

                    // update the new record to the database
                    QString errorMsg = WickrIOConsoleClientHandler::addClient(m_operation->m_ioDB, client);
                    if (errorMsg.isEmpty()) {
                        qDebug() << "CONSOLE:Successfully updated record to the database!";
                        break;
                    } else {
                        qDebug() << "CONSOLE:" << errorMsg;
                        // If the record was not updated to the database then ask the user to try again
                        QString response = getNewValue("", tr("Failed to update record, try again?"));
                        if (response.isEmpty() || response.toLower() == "n") {
                            delete client;
                            return;
                        }
                    }
                }
                m_clients = m_operation->m_ioDB->getClients();
            }
        }
    }
}

/**
 * @brief CmdClient::sendClientCmd
 * This function will send and wait for a successful sent of a message to a
 * client, with the input port number.
 * @param port
 * @param cmd
 * @return
 */
bool CmdClient::sendClientCmd(int port, const QString& cmd)
{
    m_clientMsgInProcess = true;
    m_clientMsgSuccess = false;

    if (! m_operation->m_ipc->sendMessage(port, cmd)) {
        return false;
    }

    QTimer timer;
    QEventLoop loop;

    loop.connect(m_operation->m_ipc, SIGNAL(signalSentMessage()), SLOT(quit()));
    loop.connect(m_operation->m_ipc, SIGNAL(signalSendError()), SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    int loopCount = 6;

    while (loopCount-- > 0) {
        timer.start(10000);
        loop.exec();

        if (timer.isActive()) {
            timer.stop();
            break;
        } else {
            qDebug() << "CONSOLE:Timed out waiting for stop client message to send!";
        }
    }
    return true;
}

/**
 * @brief CmdClient::pauseClient
 * This function is used to pause a running client
 * @param clientIndex
 */
void CmdClient::pauseClient(int clientIndex, bool force)
{
    if (validateIndex(clientIndex)) {
        WickrBotClients *client = m_clients.at(clientIndex);
        WickrBotProcessState state;
        QString processName = WBIOServerCommon::getClientProcessName(client);

        if (m_operation->m_ioDB->getProcessState(processName, &state)) {
            if (state.ipc_port == 0) {
                qDebug() << "CONSOLE:Client does not have an IPC port defined, cannot pause!";
            } else if (state.state == PROCSTATE_RUNNING) {
                QString prompt = QString(tr("Do you really want to pause the client with the name %1")).arg(client->name);
                QString response = getNewValue("", prompt);
                if (response.toLower() == "y" || response.toLower() == "yes") {
                    if (! sendClientCmd(state.ipc_port, WBIO_IPCCMDS_PAUSE)) {
                        qDebug() << "CONSOLE:Failed to send message to client!";
                    }
                }
            } else {
                if (!force) {
                    qDebug() << "CONSOLE:Client must be running to pause it!";
                } else {
                    if (! m_operation->m_ioDB->updateProcessState(processName, 0, PROCSTATE_PAUSED)) {
                        qDebug() << "CONSOLE:Failed to change start of client in database!";
                    } else {
                        qDebug() << "CONSOLE:Client state was force set to paused.";
                        qDebug() << "CONSOLE:Please verify the client process is not running.";
                    }
                }
            }
        } else {
            qDebug() << "CONSOLE:Could not get the clients state!";
        }
    }
}

/**
 * @brief CmdClient::startClient
 * This function is used to start a stopped or paused client
 * @param clientIndex
 */
void CmdClient::startClient(int clientIndex, bool force)
{
    if (validateIndex(clientIndex)) {
        WickrBotClients *client = m_clients.at(clientIndex);
        WickrBotProcessState state;
        QString processName = WBIOServerCommon::getClientProcessName(client);

        if (m_operation->m_ioDB->getProcessState(processName, &state)) {
            if (state.state == PROCSTATE_PAUSED || force) {
                QString prompt = QString(tr("Do you really want to start the client with the name %1")).arg(client->name);
                QString response = getNewValue("", prompt);
                if (response.toLower() == "y" || response.toLower() == "yes") {
                    // Check if the database password has been created.
                    // If not then will need the client's password to start.
                    QString clientDbDir = QString(WBIO_CLIENT_DBDIR_FORMAT).arg(m_operation->m_dbLocation).arg(client->name);
                    QString dbKeyFileName = QString("%1/dkd.wic").arg(clientDbDir);
                    QFile dbKeyFile(dbKeyFileName);
                    if (!dbKeyFile.exists()) {
                        QString configFileName = QString(WBIO_CLIENT_SETTINGS_FORMAT).arg(m_operation->m_dbLocation).arg(client->name);
                        QFile file(configFileName);
                        if (!file.exists()) {
                            qDebug() << "CONSOLE:Configuration file does not exist!";
                            return;
                        }

                        QString password;
                        do {
                            password = getNewValue("", "Enter password for this client:");
                            if (response == "quit") {
                                return;
                            }
                        } while (password.isEmpty());

                        QSettings * settings = new QSettings(configFileName, QSettings::NativeFormat);
                        settings->beginGroup(WBSETTINGS_USER_HEADER);
                        settings->setValue(WBSETTINGS_USER_PASSWORD, password);
                        settings->endGroup();
                        settings->sync();
                    }

                    if (! m_operation->m_ioDB->updateProcessState(processName, 0, PROCSTATE_DOWN)) {
                        qDebug() << "CONSOLE:Failed to change start of client in database!";
                    }
                }
            } else if (state.state == PROCSTATE_DOWN){
                qDebug() << "CONSOLE:Client is already waiting to start. The WickrIO Client Server should change the state to running.";
                qDebug() << "CONSOLE:If this is not happening, please check that the WickrIOSvr process is running!";
                //TODO: Check on the state of the WickrIO Server!
            } else {
                qDebug() << "CONSOLE:Client must be in paused state to start it!";
            }
        } else {
            qDebug() << "CONSOLE:Could not get the clients state!";
        }
    }
}
