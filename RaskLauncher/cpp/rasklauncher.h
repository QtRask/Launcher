#pragma once

#include <QObject>
#include <QVariantList>
#include "applications.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif

class RaskLauncher : public QObject
{
    Q_OBJECT
public:
    explicit RaskLauncher(QObject *parent = nullptr);

    void registerMethods();

public slots:
    void retrievePackages();

    static void launchApplication(const QString &application);
    static void openApplicationDetailsSettings(const QString &application);
    void uninstallApplication(const QString &application);

    void getSystemResources();
    void newApplication(const QString &packageName);
    void removedApplication(const QString &packageName);

private:
    Applications &applications;

#ifdef Q_OS_ANDROID
    QAndroidJniObject m_activityLauncher;
#endif

    void registerNativeMethods();
};

