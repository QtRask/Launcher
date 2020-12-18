#include "applications.h"
#include "imageprovider.h"
#include "singleton.h"

#include <QJsonDocument>
#include <QHash>
#include <QDebug>

Applications::Applications(QObject *parent) :
    QObject(parent),
    jsonModel(QStringLiteral("applications")),
    m_fields({ QStringLiteral("name"), QStringLiteral("packageName"), QStringLiteral("visible"), QStringLiteral("adaptativeIcon") }),
    m_modifiedList(false),
    m_fieldApplications(QStringLiteral("applications")),
    m_fieldHidden(QStringLiteral("hidden")),
    m_fieldDock(QStringLiteral("dock"))
{
    QVariantMap map = jsonModel.getJSONData().toMap();

    m_applications << map[m_fieldApplications].toList();
    m_applicationsHidden << map[m_fieldHidden].toList();
    m_applicationsDock << map[m_fieldDock].toList();

    connect(this, &Applications::listChanged, this, &Applications::searchListChanged);
}

void Applications::addApplications(QVariantList &list)
{
    findAndRemove(list, m_applicationsHidden);
    findAndRemove(list, m_applications);

    for (const auto &application : qAsConst(list)) {
        newApplication(application.toMap());
    }

    refreshApplicationsList();
}

void Applications::addApplication(const QVariantMap &application)
{
    newApplication(application);
    refreshApplicationsList();
}

void Applications::removeApplication(const QString &packageName)
{
    if (!findAndRemove(packageName, m_applicationsHidden))
        findAndRemove(packageName, m_applications);

    refreshApplicationsList();
}

void Applications::sort(QVariantList &value, int column, Qt::SortOrder order)
{
    std::sort(value.begin(), value.end(), [=](const QVariant &a, const QVariant &b) -> bool
    {
        return order == Qt::AscendingOrder ?
                    a.toMap()[m_fields[column - Qt::UserRole]].toString().toLower() < b.toMap()[m_fields[column - Qt::UserRole]].toString().toLower() :
                    a.toMap()[m_fields[column - Qt::UserRole]].toString().toLower() > b.toMap()[m_fields[column - Qt::UserRole]].toString().toLower();
    });
}

QVariantList Applications::getList() const
{
    return m_applications;
}

QVariantList Applications::getHidden() const
{
    return m_applicationsHidden;
}

QVariantList Applications::getDock() const
{
    return m_applicationsDock;
}

QVariantList Applications::getSearchList()
{
    QVariantList searchList = m_applications;
    std::copy_n(m_applicationsHidden.begin(), m_applicationsHidden.size(), std::back_inserter(searchList));

    sort(searchList, Name, Qt::AscendingOrder);
    return searchList;
}

void Applications::hideApplication(const QString &packageName)
{
    qDebug() << "Hide Application" << packageName;

    auto app = find(m_applications, packageName);
    if (app == std::end(m_applications)) {
        qCritical() << "Application Not Found" << packageName;
        return;
    }

    m_applicationsHidden << app->toMap();
    m_applications.erase(app);

    m_modifiedList = true;
    refreshApplicationsList();
}

void Applications::showApplication(const QString &packageName)
{
    qDebug() << "Show Application in grid" << packageName;

    auto app = find(m_applicationsHidden, packageName);
    if (app == std::end(m_applicationsHidden)) {
        qCritical() << "Application Not Found" << packageName;
        return;
    }

    m_applications << app->toMap();
    m_applicationsHidden.erase(app);

    m_modifiedList = true;
    refreshApplicationsList();
}

void Applications::addToDock(const QString &packageName)
{
    qDebug() << "Add Application in dock" << packageName;
    auto app = find(m_applicationsDock, packageName);
    if (app != std::end(m_applicationsDock)) {
        qDebug() << "Application has already been added to the dock" << packageName;
        return;
    }

    app = find(m_applicationsHidden, packageName);
    if (app == std::end(m_applicationsHidden)) {
        app = find(m_applications, packageName);
    }

    m_applicationsDock << app->toMap();
    m_modifiedList = true;
    refreshApplicationsList();
}

void Applications::removeFromDock(const QString &packageName)
{
    qDebug() << "Remove Application from dock" << packageName;

    auto app = find(m_applicationsDock, packageName);
    if (app == std::end(m_applicationsDock)) {
        qCritical() << "Application Not Found" << packageName;
        return;
    }

    m_applicationsDock.erase(app);
    m_modifiedList = true;
    refreshApplicationsList();
}

bool Applications::isOnTheDock(const QString &packageName)
{
    qDebug() << "Application is on the dock" << packageName;
    return find(m_applicationsDock, packageName) != std::end(m_applicationsDock);
}

void Applications::newApplication(const QVariantMap &application)
{
    if (find(m_applicationsHidden, application[QStringLiteral("packageName")].toString()) != std::end(m_applicationsHidden)) {
        return;
    }

    if (find(m_applications, application[QStringLiteral("packageName")].toString()) != std::end(m_applications)) {
        return;
    }

    qDebug() << "New Application" << application[QStringLiteral("packageName")].toString();
    m_applications << application;
    m_modifiedList = true;
}

void Applications::refreshApplicationsList()
{
    sort(m_applications, Name, Qt::AscendingOrder);
    sort(m_applicationsHidden, Name, Qt::AscendingOrder);

    emit listChanged();
    emit hiddenChanged();
    emit dockChanged();

    if (!m_modifiedList)
        return;

    QVariantMap applications = QVariantMap({
                                               { m_fieldApplications, m_applications },
                                               { m_fieldHidden, m_applicationsHidden },
                                               { m_fieldDock, m_applicationsDock }
                                           });

    //qDebug().noquote() << QJsonDocument::fromVariant(m_applications).toJson(QJsonDocument::Indented);

    jsonModel.setJSONData(applications);
    m_modifiedList = false;
}

QVariantList::iterator Applications::find(QVariantList &list, const QString &searchFor, Applications::ModelRoles field)
{
    return std::find_if(std::begin(list), std::end(list), [=](const QVariant &it)
    {
        return it.toMap()[m_fields[field - Qt::UserRole]].toString() == searchFor;
    });
}

bool Applications::findAndRemove(QVariantList &listSource, QVariantList &listTarget)
{
    ImageProvider &imageProvider = Singleton<ImageProvider>::getInstance();

    bool atLeastOne = false;
    for (const auto &itApp: qAsConst(listTarget)) {
        const QVariantMap &app = itApp.toMap();

        const auto &finded = find(listSource, app[QStringLiteral("packageName")].toString());
        if (finded == std::end(listSource)) {
            qDebug() << "App removed" << app[QStringLiteral("packageName")].toString();
            imageProvider.removeImageCache(app[QStringLiteral("packageName")].toString());

            listTarget.removeOne(itApp);
            atLeastOne = true;
            m_modifiedList = true;
        }
    }
    return atLeastOne;
}

bool Applications::findAndRemove(const QString &packageName, QVariantList &listTarget)
{
    const auto &it = find(listTarget, packageName);
    if (it == std::end(listTarget))
        return false;

    ImageProvider &imageProvider = Singleton<ImageProvider>::getInstance();
    imageProvider.removeImageCache(packageName);
    listTarget.erase(it);

    qDebug() << "Application Removed" << packageName;
    m_modifiedList = true;

    return true;
}
