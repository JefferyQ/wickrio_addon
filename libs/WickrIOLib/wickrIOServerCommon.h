#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QList>

#include "wickrbotclients.h"
#include "wickrIOParsers.h"

/**
 * @brief The WBIOBotTypes class
 * This class is used to identify the types of Bots supported (i.e. hubot, supportbot)
 * Custom bots will use this class as well.
 */
class WBIOBotTypes
{
public:
    WBIOBotTypes(const QString& name, const QString& type, bool customBot, bool useHttpApi, const QString& msgIface,
                 const QString& swLoc,
                 const QString& installer, const QString& configure,
                 const QString& startCmd, const QString& stopCmd, const QString& upgradeCmd) :
        m_name(name),
        m_type(type),
        m_customBot(customBot),
        m_useHttpApi(useHttpApi),
        m_msgIface(msgIface),
        m_swLocation(swLoc),
        m_installer(installer),
        m_configure(configure),
        m_startCmd(startCmd),
        m_stopCmd(stopCmd),
        m_upgradeCmd(upgradeCmd) {}

    QString m_name;
    QString m_type;
    bool    m_customBot;
    bool    m_useHttpApi;
    QString m_msgIface;
    QString m_swLocation;
    QString m_installer;
    QString m_configure;
    QString m_startCmd;
    QString m_stopCmd;
    QString m_upgradeCmd;

    QString name()       { return m_name; }
    QString type()       { return m_type; }
    bool    customBot()  { return m_customBot; }
    bool    useHttpApi() { return m_useHttpApi; }
    QString swLocation() { return m_swLocation; }
    QString installer()  { return m_installer; }
    QString configure()  { return m_configure; }
    QString startCmd()   { return m_startCmd; }
    QString stopCmd()    { return m_stopCmd; }
    QString upgradeCmd() { return m_upgradeCmd; }
};


/**
 * @brief The WBIOClientApps class
 * This class is used to identify the applications associated with the known client apps
 */
class WBIOClientApps
{
public:
    WBIOClientApps(const QString& bot, const QString& provision, const QString& parser, bool pwRequired, bool isMotherBot) :
        m_botApp(bot),
        m_provisionApp(provision),
        m_parserApp(parser),
        m_pwRequired(pwRequired),
        m_isMotherBot(isMotherBot) {}

    QString m_botApp;
    QString m_provisionApp;
    QString m_parserApp;
    bool    m_pwRequired;
    bool    m_isMotherBot;

    QList<WBIOBotTypes *> m_supportedBots;

    QString bot()       { return m_botApp; }
    QString provision() { return m_provisionApp; }
    QString parser()    { return m_parserApp; }
    bool pwRequired()   { return m_pwRequired; }
    bool isMotherBot()  { return m_isMotherBot; }

    QList<WBIOBotTypes *> supportedBots(bool customOnly);
    void addBot(WBIOBotTypes *bot) { m_supportedBots.append(bot); }
};

/**
 * @brief The WBIOServerCommon class
 * This class identifies common WBIO server static functions
 */
class WBIOServerCommon
{
public:
    WBIOServerCommon() {}

    static QSettings *getSettings();
    static QString getDBLocation();

    static void initClientApps();
    static void updateIntegrations();
    static QString getClientProcessName(WickrBotClients *client);
    static QStringList getAvailableClientApps();
    static QString getProvisionApp(const QString& clientApp);
    static QString getParserApp(const QString& clientApp);
    static QString getParserApp();
    static bool isValidClientApp(const QString& binaryName);
    static bool isPasswordRequired(const QString& binaryName);
    static WBIOClientApps *getClientApp(const QString& binaryName);

    static QString getParserProcessName(WickrIOParsers * parser);
    static QString getParserProcessName();
    static QStringList getAvailableParserApps();

    static QStringList getAvailableMotherClients();

    static QList<WBIOBotTypes *> getBotsSupported(const QString& clientApp, bool customOnly);
    static QString getBotSoftwarePath(const QString& botType);
    static QString getBotInstaller(const QString& botType);
    static QString getBotConfigure(const QString& botType);
    static QString getBotStartCmd(const QString& botType);
    static QString getBotStopCmd(const QString& botType);
    static QString getBotUpgradeCmd(const QString& botType);
    static QString getBotMsgIface(const QString& botType);

    static QStringList getIntegrationsList(const QString& integrationDirectory);
    static void addCustomIntegration(const QString& customBot, bool httpIface=false);

private:
    static bool                     m_initialized;
    static QList<WBIOClientApps *>  m_botApps;
    static QStringList              m_bots;
    static QStringList              m_parsers;

    static QList<WBIOBotTypes *>    m_supportedBots;
};


#endif // SERVER_COMMON_H
