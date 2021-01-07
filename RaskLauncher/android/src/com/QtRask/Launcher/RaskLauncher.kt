package com.QtRask.Launcher

import android.app.ActivityManager
import 	android.view.Window
import android.app.WallpaperManager
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.provider.Settings
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.drawable.AdaptiveIconDrawable
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.graphics.drawable.LayerDrawable
import android.net.ConnectivityManager
import android.net.Uri
import android.net.Uri.fromParts
import android.os.Build
import android.os.Bundle
import android.os.Vibrator
import android.os.VibrationEffect
import android.util.Log
import android.view.View
import android.view.WindowManager
import java.io.ByteArrayOutputStream
import android.content.res.Configuration
import android.graphics.Color

open class RaskLauncher : org.qtproject.qt5.android.bindings.QtActivity() {
    init {
        instance = this
    }

    private val packageBroadcastReceiver = PackageBroadcast()

    override fun onCreate(savedInstanceState: Bundle?) {
        Log.d(TAG, "Kotlin onCreate")

        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR)

        super.onCreate(savedInstanceState)
    }

    override fun onStart() {
        Log.d(TAG, "Kotlin onStart")
        super.onStart()
        instance = this

        m_activityManager = instance!!.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        m_iconDpi = m_activityManager!!.launcherLargeIconDensity
        m_packageManager = instance!!.getPackageManager() as PackageManager
        m_vibratorService = instance!!.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
        m_wallpaperManager = WallpaperManager.getInstance(this)

        getApplicationsRunning()

        val filterPackage: IntentFilter = IntentFilter()
        filterPackage.addAction(PackageBroadcast.intentActionAdded)
        filterPackage.addAction(PackageBroadcast.intentActionRemoved)
        filterPackage.addDataScheme(PackageBroadcast.intentActionPackage)
        registerReceiver(packageBroadcastReceiver, filterPackage)
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        newIntent()
    }

    companion object {
        private const val TAG = "RaskLauncher"

        var instance: RaskLauncher? = null
            private set

        private var m_iconDpi: Int = 0
        private var m_wallpaperManager: WallpaperManager? = null
        private var m_packageManager: PackageManager? = null
        private var m_vibratorService: Vibrator? = null
        private var m_theme: Int? = null
        private var m_activityManager: ActivityManager? = null

        @JvmStatic
        fun isLaunchableApplication(packageName: String): Boolean {
            return m_packageManager!!.getLaunchIntentForPackage(packageName) != null
        }

        @JvmStatic
        fun getListApplications(): Array<Application> {
            Log.d(TAG, "Get Applications")

            val intent = Intent(Intent.ACTION_MAIN, null)
            intent.addCategory(Intent.CATEGORY_LAUNCHER)
            val availableActivities = m_packageManager!!.queryIntentActivities(intent, 0)

            val applications = mutableListOf<Application>()

            for (i in availableActivities.indices) {
                applications.add(Application(
                        availableActivities[i].loadLabel(m_packageManager).toString(),
                        availableActivities[i].activityInfo.packageName,
                        getIconType(availableActivities[i].activityInfo.packageName)
                    )
                )
            }

            for (i in applications.indices) {
                if (applications[i].name.isNotEmpty() &&
                    (Character.isSpaceChar(applications[i].name[0]) ||
                     Character.isWhitespace(applications[i].name[0]))) {
                    val charToReplace = applications[i].name[0]
                    applications[i].name = applications[i].name.replace(charToReplace, ' ').trim { it <= ' ' }
                    applications[i].name = applications[i].name.trim { it <= ' ' }
                }
            }

            applications.sort()
            return applications.toTypedArray()
        }

        @JvmStatic
        fun getApplicationData(packageName: String): Application {
            Log.d(TAG, "Get Application data: " + packageName)

            try {
                val app = m_packageManager!!.getApplicationInfo(packageName, 0)
                val resources = m_packageManager!!.getResourcesForApplication(app)
                val resolveInfo = m_packageManager!!.resolveActivity(m_packageManager!!.getLaunchIntentForPackage(packageName), 0)

                val application = Application(
                    resolveInfo.loadLabel(m_packageManager).toString(),
                    packageName,
                    getIconType(packageName)
                )

                if (application.name.isNotEmpty() &&
                    (Character.isSpaceChar(application.name[0]) ||
                     Character.isWhitespace(application.name[0]))) {
                    val charToReplace = application.name[0]
                    application.name = application.name.replace(charToReplace, ' ').trim { it <= ' ' }
                    application.name = application.name.trim { it <= ' ' }
                }

                return application
            } catch (e: Exception) {
                Log.e(TAG, "getApplicationData", e)
                return Application("", "", "")
            }
        }

        @JvmStatic
        fun getApplicationIcon(packageName: String): ByteArray {
            Log.d(TAG, "getApplicationIcon")

            var icon: Drawable?

            try {
                val app = m_packageManager!!.getApplicationInfo(packageName, 0)
                val resources = m_packageManager!!.getResourcesForApplication(app)
                val resolveInfo = m_packageManager!!.resolveActivity(m_packageManager!!.getLaunchIntentForPackage(packageName), 0)
                icon = resources.getDrawableForDensity(resolveInfo.activityInfo.iconResource, m_iconDpi)
            } catch (e: Exception) {
                Log.e(TAG, "exception getApplicationIcon for $packageName", e)
                icon = null
            }

            if (icon == null)
                icon = defaultApplicationIcon

            val stream = ByteArrayOutputStream()
            var bitmap: Bitmap? = null

            if (icon is BitmapDrawable) {
                bitmap = icon.bitmap
            } else {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    if (icon is AdaptiveIconDrawable) {
                        val arrayOfDrawables = arrayOf(icon.background, icon.foreground)
                        val layerDrawable = LayerDrawable(arrayOfDrawables)

                        bitmap = Bitmap.createBitmap(layerDrawable.intrinsicWidth, layerDrawable.intrinsicHeight, Bitmap.Config.ARGB_8888)

                        val canvas = Canvas(bitmap)

                        layerDrawable.setBounds(0, 0, canvas.width, canvas.height)
                        layerDrawable.draw(canvas)
                    } else {
                        bitmap = (defaultApplicationIcon as BitmapDrawable).bitmap
                    }
                } else {
                    bitmap = (defaultApplicationIcon as BitmapDrawable).bitmap
                }
            }

            bitmap?.compress(Bitmap.CompressFormat.PNG, 100, stream)
            return stream.toByteArray()
        }

        fun getIconType(packageName: String): String {
            var icon: Drawable?

            try {
                val app = m_packageManager!!.getApplicationInfo(packageName, 0)
                val resources = m_packageManager!!.getResourcesForApplication(app)
                val resolveInfo = m_packageManager!!.resolveActivity(m_packageManager!!.getLaunchIntentForPackage(packageName), 0)
                icon = resources.getDrawableForDensity(resolveInfo.activityInfo.iconResource, m_iconDpi)
            } catch (e: Exception) {
                Log.e(TAG, "exception getApplicationIcon for $packageName", e)
                icon = null
            }
            if (icon is AdaptiveIconDrawable) {
                return "Adaptative"
            }
            return "Normal"
        }

        @JvmStatic
        fun getWallpaperImage(): ByteArray {
            Log.d(TAG, "Get Wallpaper Image from Android")

            var wallpaper: Drawable?
            try {
                wallpaper = m_wallpaperManager!!.getDrawable()
            } catch (e: Exception) {
                Log.e(TAG, "exception getWallpaperImage", e)
                wallpaper = null
            }

            var bitmap: Bitmap? = null
            if (wallpaper == null) {
                Log.e(TAG, "Error to get wallpaper image")
            }
            if (wallpaper is BitmapDrawable) {
                Log.d(TAG, "Wallpaper is BitmapDrawable")
                bitmap = wallpaper.bitmap
            }
            else
            {
                Log.d(TAG, "Wallpaper is not BitmapDrawable")
            }

            val stream = ByteArrayOutputStream()
            bitmap?.compress(Bitmap.CompressFormat.PNG, 100, stream)
            return stream.toByteArray()
        }

        val defaultApplicationIcon: Drawable
            get() = instance!!.getResources().getDrawable(android.R.mipmap.sym_def_app_icon)

        fun isAppLaunchable(packageName: String): Boolean {
            return m_packageManager!!.getLaunchIntentForPackage(packageName) != null
        }

        @JvmStatic
        fun launchApplication(packageName: String) {
            val intent = m_packageManager!!.getLaunchIntentForPackage(packageName)
            if (intent != null) {
                instance!!.startActivity(intent)
            } else {
                Log.e(TAG, "Error trying launch application $packageName")
            }
        }

        @JvmStatic
        fun applicationDetailsSettings(packageName: String) {
            val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS, fromParts("package", packageName, null))
            if (intent != null) {
                instance!!.startActivity(intent)
            } else {
                Log.e(TAG, "Error trying open Application Details Settings $packageName")
            }
        }

        @JvmStatic
        fun uninstallApplication(packageName: String) {
            val intent = Intent(Intent.ACTION_DELETE, fromParts("package", packageName, null))
            if (intent != null) {
                instance!!.startActivity(intent)
            } else {
                Log.e(TAG, "Error trying delete Application $packageName")
            }
        }

        @JvmStatic
        fun vibrate(vibe: Long = 80, amplitude: Int = -1) {
            Log.d(TAG, "Vibration service: " + vibe + ", " + amplitude)
            val amplitudeEffect = when (amplitude) {
                1 -> VibrationEffect.EFFECT_DOUBLE_CLICK
                2 -> VibrationEffect.EFFECT_TICK
                5 -> VibrationEffect.EFFECT_HEAVY_CLICK
                else -> VibrationEffect.DEFAULT_AMPLITUDE
            }

            if (Build.VERSION.SDK_INT >= 26) {
                m_vibratorService!!.vibrate(VibrationEffect.createOneShot(vibe, amplitudeEffect))
            }
        }

        @JvmStatic
        fun identifySystemTheme() {
            val theme: Int = getSystemDefaultTheme()
            Log.d(TAG, "Identify system theme: " + theme)

            if (theme != m_theme) {
                m_theme = theme
                systemTheme(theme)
            }
        }

        @JvmStatic
        fun getSystemDefaultTheme(): Int {
            if (instance!!.getResources().getConfiguration().uiMode and Configuration.UI_MODE_NIGHT_MASK == Configuration.UI_MODE_NIGHT_YES)
                return 1
           return 0
        }

        @JvmStatic
        fun getApplicationsRunning() {
            Log.d(TAG, "Get Applications running")

            val applicationsRunning = m_activityManager!!.getRunningTasks(1)
            Log.d(TAG, "Total applications running: " + applicationsRunning.indices)
            for (i in applicationsRunning.indices) {
                Log.d(TAG, "Application: " + applicationsRunning[i])
            }
        }

        @JvmStatic
        private external fun newIntent()

        @JvmStatic
        private external fun activated()

        @JvmStatic
        external fun packageAdded(packageName: String)

        @JvmStatic
        external fun packageRemoved(packageName: String)

        @JvmStatic
        external fun systemTheme(theme: Int)
    }
}
