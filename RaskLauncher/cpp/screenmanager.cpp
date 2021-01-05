#include "screenmanager.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#endif
#include <QDebug>

ScreenManager::ScreenManager(QObject *parent):
    QObject(parent),
    m_density(0),
    m_statusBarHeight(0),
    m_navigationBarHeight(0),
    m_navigationBarHeightLandscape(0),
    m_navigationBarVisible(false)
{
    setDensity(getDensity());
    setStatusBarHeight(getResourceSize(QStringLiteral("status_bar_height")));
    setNavigationBarHeight(getResourceSize(QStringLiteral("navigation_bar_height")));
    setNavigationBarHeightLandscape(getResourceSize(QStringLiteral("navigation_bar_height_landscape")));

    qDebug() << "Screen Values" << m_statusBarHeight << m_navigationBarHeight << m_navigationBarHeightLandscape;
    updateScreenValues();
}

void ScreenManager::updateScreenValues()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");

    QAndroidJniObject windowManager = activity.callObjectMethod("getWindowManager", "()Landroid/view/WindowManager;");
    QAndroidJniObject display = windowManager.callObjectMethod("getDefaultDisplay", "()Landroid/view/Display;");

    QAndroidJniObject realSize = QAndroidJniObject("android/graphics/Point");
    display.callMethod<void>("getRealSize", "(Landroid/graphics/Point;)V", realSize.object());

    QAndroidJniObject displayFrame = QAndroidJniObject("android/graphics/Rect");
    QAndroidJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
    QAndroidJniObject view = window.callObjectMethod("getDecorView", "()Landroid/view/View;");
    QAndroidJniObject rootView = view.callObjectMethod("getRootView", "()Landroid/view/View;");
    rootView.callMethod<void>("getWindowVisibleDisplayFrame", "(Landroid/graphics/Rect;)V", displayFrame.object());

    int y = std::floor(static_cast<int>(realSize.getField<jint>("y")) / m_density);
    int height = std::floor(static_cast<int>(displayFrame.callMethod<jint>("height")) / m_density);

    if ((y - height - m_statusBarHeight) == 0)
        setNavigationBarVisible(false);
    else
        setNavigationBarVisible(true);
#endif
}

bool ScreenManager::getNavigationBarVisible() const
{
    return m_navigationBarVisible;
}

void ScreenManager::setNavigationBarVisible(bool navigationBarVisible)
{
    m_navigationBarVisible = navigationBarVisible;
    emit navigationBarVisibleChanged();
}

float ScreenManager::getDensity() const
{
    return m_density;
}

void ScreenManager::setDensity(float density)
{
    m_density = density;
}

int ScreenManager::getNavigationBarHeightLandscape() const
{
    return m_navigationBarHeightLandscape;
}

void ScreenManager::setNavigationBarHeightLandscape(int value)
{
#ifdef Q_OS_ANDROID
    int navigationBarHeightLandscape = static_cast<int>(std::floor(value / m_density));
    if (m_navigationBarHeightLandscape == navigationBarHeightLandscape)
        return;

    m_navigationBarHeightLandscape = navigationBarHeightLandscape;
    emit navigationBarHeightLandscapeChanged();
#else
    m_navigationBarHeightLandscape = value;
    emit navigationBarHeightLandscapeChanged();
#endif
}

int ScreenManager::getNavigationBarHeight() const
{
    return m_navigationBarHeight;
}

void ScreenManager::setNavigationBarHeight(int value)
{
#ifdef Q_OS_ANDROID
    int navigationBarHeight = static_cast<int>(std::floor(value / m_density));
    if (m_navigationBarHeight == navigationBarHeight)
        return;

    m_navigationBarHeight = navigationBarHeight;
    emit navigationBarHeightChanged();
#else
    m_navigationBarHeight = value;
    emit navigationBarHeightChanged();
#endif
}

int ScreenManager::getStatusBarHeight() const
{
    return m_statusBarHeight;
}

void ScreenManager::setStatusBarHeight(int value)
{
#ifdef Q_OS_ANDROID
    int statusBarHeight = static_cast<int>(std::floor(value / m_density));
    if (m_statusBarHeight == statusBarHeight)
        return;

    m_statusBarHeight = statusBarHeight;
    emit statusBarHeightChanged();
#else
    m_statusBarHeight = value;
    emit statusBarHeightChanged();
#endif
}

int ScreenManager::getResourceSize(const QString &value)
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    QAndroidJniObject resources = activity.callObjectMethod("getResources", "()Landroid/content/res/Resources;");

    QAndroidJniObject name = QAndroidJniObject::fromString(value);
    QAndroidJniObject defType = QAndroidJniObject::fromString("dimen");
    QAndroidJniObject defPackage = QAndroidJniObject::fromString("android");

    jint identifier = resources.callMethod<jint>("getIdentifier",
                                                 "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",
                                                 name.object<jstring>(),
                                                 defType.object<jstring>(),
                                                 defPackage.object<jstring>());

    jint size = resources.callMethod<jint>("getDimensionPixelSize", "(I)I", identifier);

    qDebug() << "Resource" << value << size;
    return size;
#else
    Q_UNUSED(value)
    return 0;
#endif
}

float ScreenManager::getDensity()
{
    float d = 0;
#ifdef Q_OS_ANDROID
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    QAndroidJniObject resources = activity.callObjectMethod("getResources", "()Landroid/content/res/Resources;");
    QAndroidJniObject displayMetrics = resources.callObjectMethod("getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
    jfloat density = displayMetrics.getField<jfloat>("density");
    d = density;
#endif
    return d;
}
