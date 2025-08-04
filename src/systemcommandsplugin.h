/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYSTEMCOMMANDSPLUGIN_H
#define SYSTEMCOMMANDSPLUGIN_H

#include "pluginsystem/IKonsolePlugin.h"
#include <memory>
#include <QObject>

namespace Konsole {
    class MainWindow;
    class SessionController;
}

struct SystemCommand {
    QString name;
    QString command;
    QString category;
};

struct SystemCommandsPluginPrivate;

class SystemCommandsPlugin : public Konsole::IKonsolePlugin
{
    Q_OBJECT

public:
    explicit SystemCommandsPlugin(QObject *object, const QVariantList &args);
    ~SystemCommandsPlugin() override;

    void createWidgetsForMainWindow(Konsole::MainWindow *mainWindow) override;
    QList<QAction *> menuBarActions(Konsole::MainWindow *mainWindow) const override;
    void activeViewChanged(Konsole::SessionController *controller) override;

private Q_SLOTS:
    void executeCommand(const QString &command);

private:
    std::unique_ptr<SystemCommandsPluginPrivate> d;
};

#endif // SYSTEMCOMMANDSPLUGIN_H