/**
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
SceUInt32 SCE_IME_TYPE_DEFAULT;
SceUInt32 SCE_IME_TYPE_BASIC_LATIN;
SceUInt32 SCE_IME_TYPE_NUMBER;
SceUInt32 SCE_IME_TYPE_EXTENDED_NUMBER;
SceUInt32 SCE_IME_TYPE_URL;
SceUInt32 SCE_IME_TYPE_MAIL;
SceUInt32 SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT;
SceUInt32 SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD;
SceUInt32 SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR;
SceUInt32 SCE_IME_DIALOG_DIALOG_MODE_DEFAULT;
SceUInt32 SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
SceUInt32 SCE_IME_ENTER_LABEL_DEFAULT;
SceUInt32 SCE_IME_ENTER_LABEL_SEND;
SceUInt32 SCE_IME_ENTER_LABEL_SEARCH;
SceUInt32 SCE_IME_ENTER_LABEL_GO;
SceUInt32 SCE_IME_OPTION_MULTILINE;
SceUInt32 SCE_IME_OPTION_NO_AUTO_CAPITALIZATION;
SceUInt32 SCE_IME_OPTION_NO_ASSISTANCE;
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_OK;
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_YESNO;
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_NONE;
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_OK_CANCEL;
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL;
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS;
SceUInt32 SCE_MSG_DIALOG_FONT_SIZE_DEFAULT;
SceUInt32 SCE_MSG_DIALOG_FONT_SIZE_SMALL;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_NOSPACE;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_MAGNETIC_CALIBRATION;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT_SMALL;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT_CANCEL;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_NEED_MC_CONTINUE;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_NEED_MC_OPERATION;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_MIC_DISABLED;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_OPERATION;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_APPLICATION;
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_EMPTY_STORE;
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DEFAULT;
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND;
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF;
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DISABLE_OLED_DIMMING;
SceInt32 SCE_AVCONFIG_COLOR_SPACE_MODE_DEFAULT;
SceInt32 SCE_AVCONFIG_COLOR_SPACE_MODE_HIGH_CONTRAST;
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
 * Delay the app for a certain amount of time and handle any callbacks
 * @param seconds The number of seconds to delay (defaults to 0)
 */
nil os․delaycb(number seconds) {}
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
/** 
 * Closes a running app by its title ID
*/
nil os․closeapp(string titleid) {}
/** 
 * Closes all of the other running apps
*/
nil os․closeotherapps() {}
/** 
 * Launches a SELF executable (eboot.bin)
 * @attention SELF must be in the app partition
*/
nil os․execute(string path) {}
/** 
 * Gets if the battery is low
*/
boolean os․isbatterylow() {}
/** 
 * Checks if the app is in unsafe mode
*/
boolean os․unsafe() {}
/** 
 * Checks if an app exists
*/
boolean os․appexists(string titleid) {}
/** 
 * Installs a folder app (not a .VPK, but an app in a directory with what a .VPK includes)
*/
nil os․installdir(string path, boolean head) {}
/** 
 * Installs a .VPK
*/
nil os․installvpk(string path) {}
/** 
 * Deletes an app
*/
nil os․appdelete(string titleid) {}
/** 
 * Gets launch parameters
*/
string os․launchparams() {}
/** 
 * Opens the photo import dialog
 * @return Path of the photo
*/
string os․importphoto() {}
/** 
 * Aborts currently running photo import dialog
*/
nil os․abortimportphoto() {}
/** 
 * Opens the video import dialog
 * @return Path of the video
*/
string os․importvideo() {}
/** 
 * Aborts currently running video import dialog
*/
nil os․abortimportvideo() {}
/** 
 * Opens the photo review dialog
*/
nil os․photoreview(string path) {}
/** 
 * Aborts currently running photo review dialog
*/
nil os․abortphotoreview() {}
/** 
 * Opens the camera import dialog
*/
image os․cameraimport() {}
/** 
 * Aborts currently running camera import dialog
*/
nil os․abortcameraimport() {}
/** 
 * Gets or sets the CPU's clock speed
*/
number or nil os․cpu(number clock) {}
/** 
 * Gets or sets the BUS' clock speed
*/
number or nil os․bus(number clock) {}
/** 
 * Gets or sets the GPU's clock speed
*/
number or nil os․gpu(number clock) {}
/** 
 * Gets or sets the XBAR's clock speed
*/
number or nil os․xbar(number clock) {}
/** 
 * Turns the display on
*/
nil os․displayon() {}
/** 
 * Turns the display off
*/
nil os․displayoff() {}
/** 
 * Sets or gets the volume
 * @param vol optional, volume amount, between 0 to 30
*/
number or nil os․volume(number vol) {}
/** 
 * Mute the PS Vita
*/
nil os․mute() {}
/**
 * Gets the real firmware version.
*/
string os․realfirmware() {}
/**
 * Gets the spoofed firmware version.
*/
string os․spoofedfirmware() {}
/**
 * Gets the factory firmware version.
*/
string os․factoryfirmware() {}
/**
 * Shows a keyboard dialog.
 * (`os.ime` is the same as `os.keyboard`)
 * @param title The title of the dialog.
 * @param default_text The default text in the input box.
 * @param type The type of keyboard (e.g. `SCE_IME_TYPE_NUMBER`).
 * @param mode The textbox mode (e.g. `SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD`).
 * @param option Keyboard options (e.g. `SCE_IME_OPTION_MULTILINE`).
 * @param dialog_mode The dialog mode (e.g. `SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL`).
 * @param enter_label The label for the enter key (e.g. `SCE_IME_ENTER_LABEL_SEND`).
 * @param length The max length of the input text.
 * @return The text entered by the user, or nil if cancelled.
*/
string os․keyboard(string title, string default_text, SceUInt32 type, SceUInt32 mode, SceUInt32 option, SceUInt32 dialog_mode, SceUInt32 enter_label, SceUInt32 length) {}
/**
 * Shows a message dialog.
 * @param msg The message to display.
 * @param type The button type (e.g. `SCE_MSG_DIALOG_BUTTON_TYPE_YESNO`).
 * @return boolean or string depending on button type.
*/
boolean or string os․message(string msg, number type, ...) {}
/**
 * Shows a system message dialog.
 * @param type The system message type (e.g. `SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT`).
 * @return boolean indicating user choice.
*/
boolean os․systemmessage(number type) {}
/**
 * Shows an error message dialog from an error code.
 * @param errorcode The error code to display.
 * @return boolean indicating user choice.
*/
boolean os․errormessage(number errorcode) {}
/**
 * Shows a progress message dialog.
 * @param msg The message to display.
 * @return boolean indicating user choice.
*/
boolean os․progressmessage(string msg) {}
/**
 * Sets the text of the progress message dialog.
 * @param msg The new message.
*/
nil os․progressmessagetext(string msg) {}
/**
 * Sets the progress value of the progress bar.
 * @param value The value to set (0-100).
*/
nil os․setprogressmessage(number value) {}
/**
 * Increments the progress value of the progress bar.
 * @param inc The value to increment by.
*/
nil os․incprogressmessage(number inc) {}
/**
 * Closes the current message dialog.
*/
nil os․closemessage() {}
/**
 * Aborts the current message dialog.
*/
nil os․abortmessage() {}
/**
 * Closes the IME (Input Method Editor (keyboard)).
*/
nil os․closeime() {}
/**
 * Aborts the IME (keyboard) dialog.
*/
nil os․abortime() {}
/**
 * Gets a table of running applications' title IDs.
 * @param max Maximum number of apps to list (defaults to 100).
*/
table os․runningapps(number max) {}
/**
 * Checks if the battery is charging.
*/
boolean os․isbatterycharging() {}
/**
 * Gets the battery percentage.
*/
number os․batterypercent() {}
/**
 * Gets the battery State of Health (SOH).
*/
number os․batterySOH() {}
/**
 * Gets the remaining battery life time in minutes.
*/
number os․batterylifetime() {}
/**
 * Gets the battery voltage.
*/
number os․batteryvoltage() {}
/**
 * Gets the battery cycle count.
*/
number os․batterycyclecount() {}
/**
 * Gets the battery temperature in Celsius.
*/
number os․batterytemperature() {}
/**
 * Gets the battery's full capacity.
*/
number os․batterycapacity() {}
/**
 * Gets the battery's remaining capacity.
*/
number os․remainingbatterycapacity() {}
/**
 * Checks if external power is connected.
*/
boolean os․externalbattery() {}
/**
 * Prevents the system from certain power-saving states.
 * @param tick The power tick type (e.g. `SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND`).
*/
nil os․powertick(SceKernelPowerTickType tick) {}
/**
 * Locks a power state.
 * @param tick The power tick type to lock.
*/
nil os․powerlock(SceKernelPowerTickType tick) {}
/**
 * Unlocks a power state.
 * @param tick The power tick type to unlock.
*/
nil os․powerunlock(SceKernelPowerTickType tick) {}
/**
 * Checks if a suspend is required by the system.
*/
boolean os․suspendneeded() {}
/** @} */