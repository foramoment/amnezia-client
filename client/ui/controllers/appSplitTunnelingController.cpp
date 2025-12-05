#include "appSplitTunnelingController.h"

#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "core/defs.h"

AppSplitTunnelingController::AppSplitTunnelingController(const std::shared_ptr<Settings> &settings,
                                                         const QSharedPointer<AppSplitTunnelingModel> &appSplitTunnelingModel, QObject *parent)
    : QObject(parent), m_settings(settings), m_appSplitTunnelingModel(appSplitTunnelingModel)
{
}

void AppSplitTunnelingController::addApp(const QString &appPath)
{
    QFileInfo fileInfo(appPath);
    
    // If it's a directory, scan for all .exe files recursively
    if (fileInfo.isDir()) {
        QStringList addedApps;
        QStringList skippedApps;
        
        QDirIterator it(appPath, {"*.exe"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString exeName = it.fileName(); // Only the filename, e.g., "steam.exe"
            
            InstalledAppInfo appInfo { "", "", exeName };
            appInfo.appName = exeName;
            
            if (m_appSplitTunnelingModel->addApp(appInfo)) {
                addedApps.append(exeName);
            } else {
                skippedApps.append(exeName);
            }
        }
        
        if (addedApps.isEmpty() && skippedApps.isEmpty()) {
            emit errorOccurred(tr("No executable files found in folder"));
        } else if (addedApps.isEmpty()) {
            emit errorOccurred(tr("All %1 applications already added").arg(skippedApps.size()));
        } else {
            emit finished(tr("Added %1 applications from folder").arg(addedApps.size()));
        }
        return;
    }
    
    // For single files, add only the filename (not full path)
    QString fileName = fileInfo.fileName();
    InstalledAppInfo appInfo { "", "", fileName };
    appInfo.appName = fileName;

    if (m_appSplitTunnelingModel->addApp(appInfo)) {
        emit finished(tr("Application added: %1").arg(appInfo.appName));
    } else {
        emit errorOccurred(tr("The application has already been added"));
    }
}

void AppSplitTunnelingController::addApps(QVector<QPair<QString, QString>> apps)
{
    for (const auto &app : apps) {
        InstalledAppInfo appInfo { app.first, app.second, "" };

        m_appSplitTunnelingModel->addApp(appInfo);
    }
    emit finished(tr("The selected applications have been added"));
}

void AppSplitTunnelingController::removeApp(const int index)
{
    auto modelIndex = m_appSplitTunnelingModel->index(index);
    auto appPath = m_appSplitTunnelingModel->data(modelIndex, AppSplitTunnelingModel::Roles::AppPathRole).toString();
    m_appSplitTunnelingModel->removeApp(modelIndex);

    QFileInfo fileInfo(appPath);

    emit finished(tr("Application removed: %1").arg(fileInfo.fileName()));
}
