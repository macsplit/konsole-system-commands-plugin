/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "systemcommandsplugin.h"
#include "session/SessionController.h"
#include "session/Session.h"
#include "Emulation.h"
#include "Screen.h"
#include "MainWindow.h"
#include <QAction>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QPointer>
#include <KActionCollection>
#include <KLocalizedString>
#include <QDebug>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

K_PLUGIN_CLASS_WITH_JSON(SystemCommandsPlugin, "konsole_systemcommands.json")

struct SystemCommandsPluginPrivate {
    QList<SystemCommand> commands;
    QPointer<Konsole::SessionController> currentController;
    
    void loadCommands() {
        commands.clear();
        
        // Try user-specific location first
        QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        QStringList searchPaths;
        
        for (const QString &path : dataPaths) {
            searchPaths << path + QStringLiteral("/konsole");
        }
        
        // Add system-wide location
        searchPaths << QStringLiteral("/usr/share/konsole");
        
        for (const QString &searchPath : searchPaths) {
            QString filePath = searchPath + QStringLiteral("/system-commands.json");
            QFile file(filePath);
            
            if (file.exists() && file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
                
                if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                    QJsonObject obj = doc.object();
                    
                    // New format with categories and root commands
                    if (obj.contains(QStringLiteral("categories")) || obj.contains(QStringLiteral("rootCommands"))) {
                        // Handle categories
                        if (obj.contains(QStringLiteral("categories")) && obj[QStringLiteral("categories")].isObject()) {
                            QJsonObject categories = obj[QStringLiteral("categories")].toObject();
                            for (auto it = categories.begin(); it != categories.end(); ++it) {
                                QString categoryName = it.key();
                                if (it.value().isArray()) {
                                    QJsonArray categoryCommands = it.value().toArray();
                                    for (const QJsonValue &cmdValue : categoryCommands) {
                                        if (cmdValue.isObject()) {
                                            QJsonObject cmdObj = cmdValue.toObject();
                                            SystemCommand cmd;
                                            cmd.name = cmdObj[QStringLiteral("name")].toString();
                                            cmd.command = cmdObj[QStringLiteral("command")].toString();
                                            cmd.category = categoryName;
                                            if (!cmd.name.isEmpty() && !cmd.command.isEmpty()) {
                                                commands << cmd;
                                            }
                                        } else if (cmdValue.isString()) {
                                            SystemCommand cmd;
                                            cmd.name = cmdValue.toString();
                                            cmd.command = cmdValue.toString();
                                            cmd.category = categoryName;
                                            commands << cmd;
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Handle root commands
                        if (obj.contains(QStringLiteral("rootCommands")) && obj[QStringLiteral("rootCommands")].isArray()) {
                            QJsonArray rootCommands = obj[QStringLiteral("rootCommands")].toArray();
                            for (const QJsonValue &cmdValue : rootCommands) {
                                if (cmdValue.isObject()) {
                                    QJsonObject cmdObj = cmdValue.toObject();
                                    SystemCommand cmd;
                                    cmd.name = cmdObj[QStringLiteral("name")].toString();
                                    cmd.command = cmdObj[QStringLiteral("command")].toString();
                                    cmd.category = QString(); // No category for root commands
                                    if (!cmd.name.isEmpty() && !cmd.command.isEmpty()) {
                                        commands << cmd;
                                    }
                                } else if (cmdValue.isString()) {
                                    SystemCommand cmd;
                                    cmd.name = cmdValue.toString();
                                    cmd.command = cmdValue.toString();
                                    cmd.category = QString(); // No category for root commands
                                    commands << cmd;
                                }
                            }
                        }
                        
                        qDebug() << "SystemCommandsPlugin: Loaded" << commands.size() << "commands from" << filePath;
                        return;
                    }
                    // Legacy format with simple commands array
                    else if (obj.contains(QStringLiteral("commands")) && obj[QStringLiteral("commands")].isArray()) {
                        QJsonArray commandsArray = obj[QStringLiteral("commands")].toArray();
                        
                        for (const QJsonValue &value : commandsArray) {
                            if (value.isString()) {
                                SystemCommand cmd;
                                cmd.name = value.toString();
                                cmd.command = value.toString();
                                cmd.category = QString(); // No category for legacy format
                                commands << cmd;
                            }
                        }
                        
                        qDebug() << "SystemCommandsPlugin: Loaded" << commands.size() << "commands from (legacy format)" << filePath;
                        return;
                    }
                }
                file.close();
                qDebug() << "SystemCommandsPlugin: Invalid JSON format in" << filePath;
            }
        }
        
        // Fallback to default commands if no JSON file found
        QStringList defaultCommands = {
            QStringLiteral("ls"),
            QStringLiteral("history"),
            QStringLiteral("df"),
            QStringLiteral("du"),
            QStringLiteral("htop"),
            QStringLiteral("sudo nethogs"),
            QStringLiteral("sudo fdisk -l")
        };
        
        for (const QString &cmd : defaultCommands) {
            SystemCommand syscmd;
            syscmd.name = cmd;
            syscmd.command = cmd;
            syscmd.category = QString(); // No category for default commands
            commands << syscmd;
        }
        qDebug() << "SystemCommandsPlugin: Using default commands (no JSON file found)";
    }
};

SystemCommandsPlugin::SystemCommandsPlugin(QObject *object, const QVariantList &args)
    : Konsole::IKonsolePlugin(object, args)
    , d(std::make_unique<SystemCommandsPluginPrivate>())
{
    setName(QStringLiteral("SystemCommands"));
    d->loadCommands();
}

SystemCommandsPlugin::~SystemCommandsPlugin() = default;

void SystemCommandsPlugin::createWidgetsForMainWindow(Konsole::MainWindow *mainWindow)
{
    Q_UNUSED(mainWindow);
    // No widgets needed for this plugin
}

QList<QAction *> SystemCommandsPlugin::menuBarActions(Konsole::MainWindow *mainWindow) const
{
    QList<QAction *> actions;
    
    // Group commands by category
    QMap<QString, QList<SystemCommand>> categorizedCommands;
    QList<SystemCommand> rootCommands;
    
    for (const SystemCommand &cmd : d->commands) {
        if (cmd.category.isEmpty()) {
            rootCommands << cmd;
        } else {
            categorizedCommands[cmd.category] << cmd;
        }
    }
    
    // Add root level commands as individual actions
    for (const SystemCommand &cmd : rootCommands) {
        auto *action = new QAction(cmd.name, mainWindow);
        connect(action, &QAction::triggered, this, [this, cmd]() {
            const_cast<SystemCommandsPlugin*>(this)->executeCommand(cmd.command);
        });
        actions << action;
    }
    
    // Add separator if we have both root commands and categories
    if (!rootCommands.isEmpty() && !categorizedCommands.isEmpty()) {
        actions << new QAction(mainWindow);
        actions.last()->setSeparator(true);
    }
    
    // Add each category as a separate top-level menu
    for (auto it = categorizedCommands.begin(); it != categorizedCommands.end(); ++it) {
        const QString &categoryName = it.key();
        const QList<SystemCommand> &categoryCommands = it.value();
        
        auto *categoryMenu = new QMenu(categoryName, mainWindow);
        
        for (const SystemCommand &cmd : categoryCommands) {
            auto *action = new QAction(cmd.name, mainWindow);
            connect(action, &QAction::triggered, this, [this, cmd]() {
                const_cast<SystemCommandsPlugin*>(this)->executeCommand(cmd.command);
            });
            categoryMenu->addAction(action);
        }
        
        actions << categoryMenu->menuAction();
    }
    
    return actions;
}

void SystemCommandsPlugin::activeViewChanged(Konsole::SessionController *controller)
{
    d->currentController = QPointer(controller);
}

void SystemCommandsPlugin::executeCommand(const QString &command)
{
    if (!d->currentController) {
        qDebug() << "SystemCommandsPlugin: No current controller available";
        return;
    }
    
    auto session = d->currentController->session();
    if (!session) {
        qDebug() << "SystemCommandsPlugin: No current session found";
        return;
    }
    
    auto emulation = session->emulation();
    if (!emulation) {
        qDebug() << "SystemCommandsPlugin: No emulation found";
        return;
    }
    
    // Fallback to sendTextToTerminal for compatibility
    session->sendTextToTerminal(command, QChar::fromLatin1('\r'));
    
    qDebug() << "SystemCommandsPlugin: Sent command:" << command;
}


#include "systemcommandsplugin.moc"