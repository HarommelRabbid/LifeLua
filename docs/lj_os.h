/**
 * @file
 * @defgroup os os
 * @brief Lua OS library extension
 * @{
*/
SceShutterSoundType SCE_SHUTTER_SOUND_TYPE_SAVE_IMAGE;
SceShutterSoundType SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_START;
SceShutterSoundType SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_END;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK8;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK80;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK100;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK200;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER;
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2;
/** @brief `INFOBAR_VISIBILITY_INVISIBLE` for short */
SceAppMgrInfoBarVisibility SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE;
/** @brief `INFOBAR_VISIBILITY_VISIBLE` for short */
SceAppMgrInfoBarVisibility SCE_APPMGR_INFOBAR_VISIBILITY_VISIBLE;
/** @brief `INFOBAR_COLOR_BLACK` for short */
SceAppMgrInfoBarColor SCE_APPMGR_INFOBAR_COLOR_BLACK;
/** @brief `INFOBAR_COLOR_WHITE` for short */
SceAppMgrInfoBarColor SCE_APPMGR_INFOBAR_COLOR_WHITE;
/** @brief `INFOBAR_TRANSPARENCY_OPAQUE` for short */
SceAppMgrInfoBarTransparency SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE;
/** @brief `INFOBAR_TRANSPARENCY_TRANSLUCENT` for short */
SceAppMgrInfoBarTransparency SCE_APPMGR_INFOBAR_TRANSPARENCY_TRANSLUCENT;
/**
 * Delay the app for a certain amount of time
 * @param seconds The number of seconds to delay (defaults to 0)
 */
nil os․delay(number seconds) {}
/** 
 * Exits the app
*/
nil os․exit() {}
/** 
 * Opens an URI
 * @param flag: Optional, will default to `0xFFFFF` if not set
*/
nil os․uri(string uri, number flag) {}
/** 
 * Play a shutter sound
*/
nil os․shuttersound(SceShutterSoundType shuttertype) {}
/** 
 * Sets the state of the infobar
*/
nil os․infobar(SceAppMgrInfoBarVisibility state, SceAppMgrInfoBarColor color, SceAppMgrInfoBarTransparency transparency) {}
/** 
 * Locks some shell functionality (e. g. the PS button, the quick menu, the power off menu, USB connection etc.)
 * > [!note]
 * After `lock` has been defined as a boolean, you can add as many `SceShellUtilLockType` values as you want as the next arguments
 * @par Example:
 * @code
 * os.lock(true, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN, SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU, SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION)
 * @endcode
*/
nil os․lock(boolean lock, SceShellUtilLockType type, ...) {}
/** 
 * Sends a notification
*/
nil os․notification(string text) {}
/** 
 * Exports a video (.mp4) file
*/
nil os․videoexport(string path) {}
/** 
 * Exports a photo (.png, .jpg/.jpeg, .bmp) file
*/
nil os․photoexport(string path) {}
/** 
 * Exports an audio (.mp3) file
*/
nil os․musicexport(string path) {}
/** 
 * Turns off the PS Vita completely
 * > [!note]
 * `os.suspend` does the same thing
*/
nil os․shutdown() {}
/** 
 * Puts the PS Vita into standby
*/
nil os․standby() {}
/** 
 * Restarts the PS Vita
*/
nil os․restart() {}
/** 
 * Gets the app's title
*/
string os․title() {}
/** 
 * Gets the app's title ID
*/
string os․titleid() {}
/** @} */