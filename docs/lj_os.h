/**
 * @defgroup os os
 * @brief System dialogs, apps, battery, power & system information (extends Lua's os library)
 *
 * These functions are added to Lua's standard `os` library, so `os.time()`, `os.date()`, `os.clock()`, etc. still work
 * like in normal Lua (see the [Lua manual](https://www.lua.org/manual/5.1/manual.html#5.8)).
 *
 * ## System dialogs
 * The dialog functions (osꓸkeyboard(), osꓸmessage(), osꓸimportphoto(), ...) show one of the PS Vita's own dialogs
 * and **wait until it's closed**, then return the result. While a dialog is open your main loop doesn't run,
 * so the screen behind it is empty. To draw something behind a dialog (or to update it, like a progress bar),
 * define the dialog's callback function. It's called once per frame while that dialog is open:
 *
 * | Dialog function | Callback function |
 * |-----------------|-------------------|
 * | osꓸkeyboard() | `LifeLuaIMEDialog()` |
 * | osꓸmessage() | `LifeLuaMessageDialog()` |
 * | osꓸsystemmessage() | `LifeLuaSystemMessageDialog()` |
 * | osꓸerrormessage() | `LifeLuaErrorCodeDialog()` |
 * | osꓸprogressmessage() | `LifeLuaProgressMessageDialog()` |
 * | osꓸimportphoto() | `LifeLuaPhotoImportDialog()` |
 * | osꓸimportvideo() | `LifeLuaVideoImportDialog()` |
 * | osꓸphotoreview() | `LifeLuaPhotoReviewDialog()` |
 * | osꓸcameraimport() | `LifeLuaCameraImportDialog()` |
 *
 * Don't call drawꓸswapbuffers() inside these callbacks, LifeLua does it for you.
 * @code
 * function LifeLuaIMEDialog()
 *     background:display(0, 0) -- keep the app's background visible behind the keyboard
 * end
 * local name = os.keyboard("What's your name?", "Harommel")
 * if name then
 *     os.message("Hello, "..name.."!")
 * end
 * @endcode
 * @{
*/
/** @name Dialogs
 * @{ */
/**
 * Shows the system keyboard and waits for the user to type something.
 *
 * osꓸime() is the same function under another name.
 * @param title Title shown above the text box
 * @param default_text Text already in the text box *(optional, default: empty)*
 * @param type One of the keyboard type constants *(optional, default: SCE_IME_TYPE_DEFAULT)*
 * @param mode One of the text box mode constants *(optional, default: SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT)*
 * @param option Keyboard option constants, added together *(optional, default: none)*
 * @param dialog_mode One of the dialog mode constants *(optional, default: SCE_IME_DIALOG_DIALOG_MODE_DEFAULT)*
 * @param enter_label One of the enter label constants *(optional, default: SCE_IME_ENTER_LABEL_DEFAULT)*
 * @param length Maximum number of characters *(optional, default: the system maximum)*
 * @return The typed text, or `nil` if the user canceled
 * @par Example:
 * @code
 * local pin = os.keyboard("Enter your PIN", "", SCE_IME_TYPE_NUMBER, SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD)
 * @endcode
*/
string osꓸkeyboard(string title, string default_text, SceUInt32 type, SceUInt32 mode, SceUInt32 option, SceUInt32 dialog_mode, SceUInt32 enter_label, number length) {}
/**
 * Same as osꓸkeyboard()
 * @param title Title shown above the text box
 * @param default_text Text already in the text box *(optional)*
 * @param type One of the keyboard type constants *(optional)*
 * @param mode One of the text box mode constants *(optional)*
 * @param option Keyboard option constants, added together *(optional)*
 * @param dialog_mode One of the dialog mode constants *(optional)*
 * @param enter_label One of the enter label constants *(optional)*
 * @param length Maximum number of characters *(optional)*
 * @return The typed text, or `nil` if the user canceled
*/
string osꓸime(string title, string default_text, SceUInt32 type, SceUInt32 mode, SceUInt32 option, SceUInt32 dialog_mode, SceUInt32 enter_label, number length) {}
/**
 * Closes the keyboard (the typed text is kept)
*/
nil osꓸcloseime() {}
/**
 * Aborts the keyboard dialog
*/
nil osꓸabortime() {}
/**
 * Shows a message dialog and waits until it's closed
 *
 * With SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS, 6 more parameters are required: the text and font size of each button.
 * @param text The message
 * @param buttons One of the message dialog button constants *(optional, default: SCE_MSG_DIALOG_BUTTON_TYPE_OK)*
 * @param ... The 3 button texts and font sizes, only for SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS
 * @return `false` if the user chose "No" (or the dialog was invalid), otherwise `true`.
 * With 3 buttons: the text of the chosen button, or `nil` if none was chosen.
 * @par Example:
 * @code
 * if os.message("Delete the save file?", SCE_MSG_DIALOG_BUTTON_TYPE_YESNO) then
 *     io.delete("ux0:data/myapp/save.txt")
 * end
 *
 * local size = SCE_MSG_DIALOG_FONT_SIZE_DEFAULT
 * local level = os.message("Choose a difficulty", SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS, "Easy", size, "Normal", size, "Hard", size)
 * -- level is "Easy", "Normal" or "Hard"
 * @endcode
*/
any osꓸmessage(string text, SceUInt32 buttons, ...) {}
/**
 * Shows one of the system's built-in messages and waits until it's closed
 * @param type One of the system message type constants
 * @return `false` if the user chose "No", otherwise `true`
*/
boolean osꓸsystemmessage(SceUInt32 type) {}
/**
 * Shows the system's error message for an error code (like `C1-2345-6`) and waits until it's closed
 * @param errorcode The error code number (e.g. `0x80010002`)
 * @return `false` if the user chose "No", otherwise `true`
*/
boolean osꓸerrormessage(number errorcode) {}
/**
 * Shows a message with a progress bar and waits until it's closed.
 * Because this waits, the progress bar is updated from the `LifeLuaProgressMessageDialog()` callback,
 * using osꓸsetprogressmessage(), osꓸincprogressmessage() and osꓸprogressmessagetext(), then closed with osꓸclosemessage().
 * @param text The message
 * @return `true` when it's closed
 * @par Example:
 * @code
 * local progress = 0
 * function LifeLuaProgressMessageDialog()
 *     progress = progress + 1
 *     os.setprogressmessage(progress)
 *     if progress >= 100 then os.closemessage() end
 * end
 * os.progressmessage("Loading...")
 * @endcode
*/
boolean osꓸprogressmessage(string text) {}
/**
 * Changes the text of the open progress message
 * @param text The new text
*/
nil osꓸprogressmessagetext(string text) {}
/**
 * Sets the value of the open progress message's bar
 * @param value Progress from 0 to 100
*/
nil osꓸsetprogressmessage(number value) {}
/**
 * Increases the value of the open progress message's bar
 * @param amount How much to add
*/
nil osꓸincprogressmessage(number amount) {}
/**
 * Closes the open message dialog (like pressing its button)
*/
nil osꓸclosemessage() {}
/**
 * Aborts the open message dialog
*/
nil osꓸabortmessage() {}
/**
 * Lets the user pick a photo from the Photos app
 * @return The path of the chosen photo, or `nil` if the user canceled
 * @par Example:
 * @code
 * local path = os.importphoto()
 * if path then picture = image.load(path) end
 * @endcode
*/
string osꓸimportphoto() {}
/**
 * Aborts the photo import dialog
*/
nil osꓸabortphotoimport() {}
/**
 * Lets the user pick a video from the Videos app
 * @return The path of the chosen video, or `nil` if the user canceled
*/
string osꓸimportvideo() {}
/**
 * Aborts the video import dialog
*/
nil osꓸabortimportvideo() {}
/**
 * Opens a photo in the system's photo viewer
 * @param path Path of the photo
*/
nil osꓸphotoreview(string path) {}
/**
 * Aborts the photo review dialog
*/
nil osꓸabortphotoreview() {}
/**
 * Opens the system's camera dialog so the user can take a photo
 * @return The photo as an image, or `nil` if the user canceled
*/
image osꓸcameraimport() {}
/**
 * Aborts the camera dialog
*/
nil osꓸabortcameraimport() {}
/**
 * Shows a notification (the kind that slides in from the top of the screen)
 * @param text The notification text
*/
nil osꓸnotification(string text) {}
/** @} */

/** @name Apps
 * @{ */
/**
 * Gets your app's title (as shown in the LiveArea)
 * @return The title
*/
string osꓸtitle() {}
/**
 * Gets your app's title ID
 * @return The title ID, e.g. `"LIFELUAJT"`
*/
string osꓸtitleid() {}
/**
 * Checks if an app is installed
 * @param titleid The app's title ID
 * @return `true` if it's installed
*/
boolean osꓸappexists(string titleid) {}
/**
 * Uninstalls an app. Waits until it's done.
 * @param titleid The app's title ID
*/
nil osꓸappdelete(string titleid) {}
/**
 * Installs an app from a folder (the extracted contents of a .VPK). Waits until it's done.
 * @param path Path of the folder
 * @param has_headbin `true` if the folder already has a `sce_sys/package/head.bin`, `false` to have one made *(optional, default: false)*
*/
nil osꓸinstalldir(string path, boolean has_headbin) {}
/**
 * Installs a .VPK file. It's extracted to `ux0:data/ll_temp` first, then installed with osꓸinstalldir().
 * @param path Path of the .VPK
 * @param has_headbin `true` if the .VPK already has a `sce_sys/package/head.bin` *(optional, default: false)*
*/
nil osꓸinstallvpk(string path, boolean has_headbin) {}
/**
 * Gets the title IDs of the apps that are running
 * @param max Maximum number of apps to list *(optional, default: 100)*
 * @return A list of title IDs
*/
table osꓸrunningapps(number max) {}
/**
 * Closes a running app
 * @param titleid The app's title ID
*/
nil osꓸcloseapp(string titleid) {}
/**
 * Closes all the other running apps
*/
nil osꓸcloseotherapps() {}
/**
 * Opens a URI, which can launch other apps or open a website in the browser
 * @param uri The URI (e.g. `"psgm:play?titleid=VITASHELL"` or `"https://example.com"`)
 * @param flag Launch flags *(optional, default: 0xFFFFF)*
*/
nil osꓸuri(string uri, number flag) {}
/**
 * Starts another program (eboot.bin/SELF) in place of your app
 * @param path Path of the SELF, it must be inside the app's partition (`app0:`)
 * @param ... Strings passed to the program as its arguments *(optional)*
*/
nil osꓸexecute(string path, ...) {}
/**
 * Gets the parameters your app was launched with (e.g. from a URI)
 * @return The parameters as a string
*/
string osꓸlaunchparams() {}
/**
 * Unmounts a mount point
 * @param mountpoint The mount point
*/
nil osꓸunmountmountpoint(string mountpoint) {}
/**
 * Loads and starts a user module (.SUPRX), giving it the Lua state so it can add its own Lua functions
 * @param path Path of the module
*/
nil osꓸrequiremodule(string path) {}
/**
 * Exits the app (standard Lua function)
*/
nil osꓸexit() {}
/** @} */

/** @name Timing
 * @{ */
/**
 * Pauses the app for some time
 * @param seconds How long to wait, in seconds (decimals allowed) *(optional, default: 0)*
*/
nil osꓸdelay(number seconds) {}
/**
 * Pauses the app for some time, while letting system callbacks run
 * @param seconds How long to wait, in seconds (decimals allowed) *(optional, default: 0)*
*/
nil osꓸdelaycb(number seconds) {}
/** @} */

/** @name System UI
 * @{ */
/**
 * Shows, hides or changes the info bar (the bar at the top of the screen with the clock and battery)
 * @param visibility SCE_APPMGR_INFOBAR_VISIBILITY_VISIBLE or SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE *(optional, default: invisible)*
 * @param color SCE_APPMGR_INFOBAR_COLOR_BLACK or SCE_APPMGR_INFOBAR_COLOR_WHITE *(optional, default: black)*
 * @param transparency SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE or SCE_APPMGR_INFOBAR_TRANSPARENCY_TRANSLUCENT *(optional, default: opaque)*
 * @par Example:
 * @code
 * os.infobar(INFOBAR_VISIBILITY_VISIBLE, INFOBAR_COLOR_WHITE, INFOBAR_TRANSPARENCY_TRANSLUCENT)
 * @endcode
*/
nil osꓸinfobar(SceAppMgrInfoBarVisibility visibility, SceAppMgrInfoBarColor color, SceAppMgrInfoBarTransparency transparency) {}
/**
 * Blocks or unblocks system features, like the PS button or the quick menu
 * @param lock `true` to block, `false` to unblock
 * @param type One or more shell lock type constants, as separate parameters
 * @param ... More shell lock type constants
 * @par Example:
 * @code
 * os.lock(true, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN, SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU)
 * -- ...
 * os.lock(false, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN, SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU)
 * @endcode
*/
nil osꓸlock(boolean lock, SceShellUtilLockType type, ...) {}
/**
 * Plays the camera shutter sound
 * @param type One of the shutter sound type constants
*/
nil osꓸshuttersound(SceShutterSoundType type) {}
/**
 * Gets the next system event, like waking up from sleep
 * @return One of the system event constants
*/
SceAppMgrSystemEventType osꓸsystemevent() {}
/** @} */

/** @name Screenshots
 * @{ */
/**
 * Allows or blocks taking screenshots (PS button + Start) in your app
 * @param enable `true` to allow, `false` to block
*/
nil osꓸscreenshot(boolean enable) {}
/**
 * Sets an image that's drawn over every screenshot taken in your app, like a watermark
 * @param path Path of the image (.PNG)
 * @param x X position of the image *(optional, default: 0)*
 * @param y Y position of the image *(optional, default: 0)*
*/
nil osꓸscreenshotoverlay(string path, number x, number y) {}
/**
 * Sets the information saved with screenshots taken in your app
 * @param photo_title Title of the photo *(optional)*
 * @param game_title Title of the game *(optional)*
 * @param game_comment A comment *(optional)*
*/
nil osꓸscreenshotinfo(string photo_title, string game_title, string game_comment) {}
/**
 * Shows or hides the notification that appears after taking a screenshot
 * @param enable `true` to show it, `false` to hide it
*/
nil osꓸscreenshotnotification(boolean enable) {}
/**
 * Takes a screenshot and saves it to the Photos app, like pressing PS button + Start
 * @param mode `0` for a normal screenshot, `1` to take it even if screenshots are blocked *(optional, default: 1)*
*/
nil osꓸscreenshotcapture(number mode) {}
/** @} */

/** @name Exporting to system apps
 * @{ */
/**
 * Copies a photo into the Photos app
 * @param path Path of the photo (.PNG, .JPG/.JPEG, .BMP)
 * @return `true` on success
*/
boolean osꓸphotoexport(string path) {}
/**
 * Copies a song into the Music app
 * @param path Path of the song (.MP3)
 * @return `true` on success
*/
boolean osꓸmusicexport(string path) {}
/**
 * Copies a video into the Videos app
 * @param path Path of the video (.MP4)
 * @return `true` on success
*/
boolean osꓸvideoexport(string path) {}
/** @} */

/** @name Audio & display
 * @{ */
/**
 * Gets or sets the system volume
 * @param volume New volume, from 0 to 30 *(optional)*
 * @return The current volume (0-30) if `volume` is omitted
*/
number osꓸvolume(number volume) {}
/**
 * Mutes the PS Vita
*/
nil osꓸmute() {}
/**
 * Takes or gives back the background music audio port
 * @param acquire `true` to take it, `false` to give it back
*/
nil osꓸacquirebgm(boolean acquire) {}
/**
 * Sets the screen's color mode
 * @param mode One of the color space mode constants *(optional, default: SCE_AVCONFIG_COLOR_SPACE_MODE_DEFAULT)*
*/
nil osꓸcolorspace(SceInt32 mode) {}
/**
 * Turns the screen on
*/
nil osꓸdisplayon() {}
/**
 * Turns the screen off (the app keeps running)
*/
nil osꓸdisplayoff() {}
/** @} */

/** @name Power
 * @{ */
/**
 * Restarts the PS Vita
*/
nil osꓸrestart() {}
/**
 * Turns the PS Vita off
*/
nil osꓸstandby() {}
/**
 * Puts the PS Vita to sleep (rest mode)
*/
nil osꓸsuspend() {}
/**
 * Same as osꓸsuspend(): puts the PS Vita to sleep. Use osꓸstandby() to turn it off.
*/
nil osꓸshutdown() {}
/**
 * Checks if the system wants the app to let the PS Vita go to sleep
 * @return `true` if a sleep is needed
*/
boolean osꓸsuspendneeded() {}
/**
 * Resets power timers, so the screen doesn't dim/turn off and the PS Vita doesn't fall asleep.
 * Call it regularly (e.g. once per frame) while you want to keep the PS Vita awake, like during a video.
 * @param type One of the power tick type constants *(optional, default: SCE_KERNEL_POWER_TICK_DEFAULT)*
*/
nil osꓸpowertick(SceKernelPowerTickType type) {}
/**
 * Prevents the PS Vita from going to sleep until osꓸpowerunlock() is called
 * @param type One of the power tick type constants *(optional, default: SCE_KERNEL_POWER_TICK_DEFAULT)*
*/
nil osꓸpowerlock(SceKernelPowerTickType type) {}
/**
 * Allows the PS Vita to go to sleep again after osꓸpowerlock()
 * @param type One of the power tick type constants *(optional, default: SCE_KERNEL_POWER_TICK_DEFAULT)*
*/
nil osꓸpowerunlock(SceKernelPowerTickType type) {}
/**
 * Gets or sets the CPU clock speed
 * @param mhz New speed in MHz (e.g. 333 or 444) *(optional)*
 * @return The current speed in MHz if `mhz` is omitted
*/
number osꓸcpu(number mhz) {}
/**
 * Gets or sets the bus clock speed
 * @param mhz New speed in MHz *(optional)*
 * @return The current speed in MHz if `mhz` is omitted
*/
number osꓸbus(number mhz) {}
/**
 * Gets or sets the GPU clock speed
 * @param mhz New speed in MHz *(optional)*
 * @return The current speed in MHz if `mhz` is omitted
*/
number osꓸgpu(number mhz) {}
/**
 * Gets or sets the GPU crossbar (XBAR) clock speed
 * @param mhz New speed in MHz *(optional)*
 * @return The current speed in MHz if `mhz` is omitted
*/
number osꓸxbar(number mhz) {}
/** @} */

/** @name Battery
 * @{ */
/**
 * Gets the battery charge
 * @return The charge in percent (0-100)
*/
number osꓸbatterypercent() {}
/**
 * Checks if the battery is charging
 * @return `true` if it's charging
*/
boolean osꓸisbatterycharging() {}
/**
 * Checks if the battery is low
 * @return `true` if it's low
*/
boolean osꓸisbatterylow() {}
/**
 * Checks if a charger is connected
 * @return `true` if the PS Vita is plugged in
*/
boolean osꓸexternalbattery() {}
/**
 * Gets the estimated remaining battery time
 * @return Time in minutes
*/
number osꓸbatterylifetime() {}
/**
 * Gets the battery's health (State Of Health)
 * @return Health in percent
*/
number osꓸbatterySOH() {}
/**
 * Gets the battery voltage
 * @return Voltage in millivolts
*/
number osꓸbatteryvoltage() {}
/**
 * Gets how many times the battery has been fully charged
 * @return The number of charge cycles
*/
number osꓸbatterycyclecount() {}
/**
 * Gets the battery temperature
 * @return Temperature in degrees Celsius
*/
number osꓸbatterytemperature() {}
/**
 * Gets the battery's full capacity
 * @return Capacity in mAh
*/
number osꓸbatterycapacity() {}
/**
 * Gets the battery's remaining capacity
 * @return Remaining capacity in mAh
*/
number osꓸremainingbatterycapacity() {}
/** @} */

/** @name System information
 * @{ */
/**
 * Gets the PSN/system user name
 * @return The user name
*/
string osꓸusername() {}
/**
 * Gets the system language
 * @return One of the language constants
 * @par Example:
 * @code
 * local greeting = "Hello"
 * if os.language() == SCE_SYSTEM_PARAM_LANG_FRENCH then greeting = "Bonjour" end
 * @endcode
*/
SceSystemParamLang osꓸlanguage() {}
/**
 * Gets the system's date format
 * @return One of the date format constants
*/
SceSystemParamDateFormat osꓸdateformat() {}
/**
 * Gets the system's time format
 * @return One of the time format constants
*/
SceSystemParamTimeFormat osꓸtimeformat() {}
/**
 * Gets the real firmware version
 * @return The version, e.g. `"3.65"`
*/
string osꓸrealfirmware() {}
/**
 * Gets the firmware version the system reports (it can be spoofed by plugins)
 * @return The version, e.g. `"3.74"`
*/
string osꓸspoofedfirmware() {}
/**
 * Gets the firmware version the PS Vita came with from the factory
 * @return The version, e.g. `"3.60"`
*/
string osꓸfactoryfirmware() {}
/**
 * Checks if the app is running in unsafe mode (enabled in HENkaku's settings), which is needed for some features
 * @return `true` if unsafe mode is on
*/
boolean osꓸunsafe() {}
/**
 * Checks if the console is a PS Vita handheld
 * @return `true` on a PS Vita
*/
boolean osꓸisvita() {}
/**
 * Checks if the console is a PS TV
 * @return `true` on a PS TV
*/
boolean osꓸispstv() {}
/**
 * Checks if the console is a retail (CEX) unit
 * @return `true` on a retail unit
*/
boolean osꓸiscex() {}
/**
 * Checks if the console is a debug/testing (DEX) unit
 * @return `true` on a DEX unit
*/
boolean osꓸisdex() {}
/**
 * Checks if the console is a development kit (TOOL)
 * @return `true` on a development kit
*/
boolean osꓸistool() {}
/**
 * Checks if the console is a test unit
 * @return `true` on a test unit
*/
boolean osꓸistest() {}
/**
 * Reads a system registry key
 * @param category The key's category (e.g. `"/CONFIG/SYSTEM/"`)
 * @param name The key's name (e.g. `"username"`)
 * @return The value (a string or number), or `nil` if the key can't be read
*/
any osꓸgetregkey(string category, string name) {}
/**
 * Writes a system registry key
 * @warning Wrong registry values can break system settings. Only change keys you know.
 * @param category The key's category
 * @param name The key's name
 * @param value The new value, a number or a string
 * @param size Size of a string/binary value in bytes *(optional, default: the length of `value`)*
*/
nil osꓸsetregkey(string category, string name, any value, number size) {}
/** @} */

/** @name Shutter sound types
 * Used with osꓸshuttersound()
 * @{ */
SceShutterSoundType SCE_SHUTTER_SOUND_TYPE_SAVE_IMAGE; ///< Photo shutter sound
SceShutterSoundType SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_START; ///< Video recording start sound
SceShutterSoundType SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_END; ///< Video recording end sound
/** @} */
/** @name Shell lock types
 * Used with osꓸlock()
 * @{ */
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN; ///< The PS button (pressing it can then be detected with SCE_CTRL_PSBUTTON)
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU; ///< The quick menu (holding the PS button)
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU; ///< The power off menu (holding the power button)
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION; ///< USB connection
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED; ///< Memory card inserted message
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED; ///< Memory card removed message
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER; ///< Music player controls
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2; ///< The PS button, without showing the "not allowed" symbol
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK8; ///< Unknown
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK80; ///< Unknown
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK100; ///< Unknown
SceShellUtilLockType SCE_SHELL_UTIL_LOCK_TYPE_UNK200; ///< Unknown
/** @} */
/** @name Info bar settings
 * Used with osꓸinfobar(). Each one is also available without the `SCE_APPMGR_` prefix (e.g. `INFOBAR_COLOR_WHITE`).
 * @{ */
SceAppMgrInfoBarVisibility SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE; ///< Hide the info bar
SceAppMgrInfoBarVisibility SCE_APPMGR_INFOBAR_VISIBILITY_VISIBLE; ///< Show the info bar
SceAppMgrInfoBarColor SCE_APPMGR_INFOBAR_COLOR_BLACK; ///< Black info bar
SceAppMgrInfoBarColor SCE_APPMGR_INFOBAR_COLOR_WHITE; ///< White info bar
SceAppMgrInfoBarTransparency SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE; ///< Solid info bar
SceAppMgrInfoBarTransparency SCE_APPMGR_INFOBAR_TRANSPARENCY_TRANSLUCENT; ///< See-through info bar
/** @} */
/** @name Keyboard types
 * Used in the `type` parameter of osꓸkeyboard()
 * @{ */
SceUInt32 SCE_IME_TYPE_DEFAULT; ///< Normal keyboard
SceUInt32 SCE_IME_TYPE_BASIC_LATIN; ///< Basic latin letters only
SceUInt32 SCE_IME_TYPE_NUMBER; ///< Number pad
SceUInt32 SCE_IME_TYPE_EXTENDED_NUMBER; ///< Number pad with extra symbols
SceUInt32 SCE_IME_TYPE_URL; ///< Keyboard for typing web addresses
SceUInt32 SCE_IME_TYPE_MAIL; ///< Keyboard for typing e-mail addresses
/** @} */
/** @name Keyboard text box modes
 * Used in the `mode` parameter of osꓸkeyboard()
 * @{ */
SceUInt32 SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT; ///< Normal text box
SceUInt32 SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD; ///< Hides the typed text, for passwords
SceUInt32 SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR; ///< Text box with a clear button
/** @} */
/** @name Keyboard options
 * Used in the `option` parameter of osꓸkeyboard(). They can be added together.
 * @{ */
SceUInt32 SCE_IME_OPTION_MULTILINE; ///< Allow several lines of text
SceUInt32 SCE_IME_OPTION_NO_AUTO_CAPITALIZATION; ///< Don't capitalize the first letter automatically
SceUInt32 SCE_IME_OPTION_NO_ASSISTANCE; ///< No word suggestions
/** @} */
/** @name Keyboard dialog modes
 * Used in the `dialog_mode` parameter of osꓸkeyboard()
 * @{ */
SceUInt32 SCE_IME_DIALOG_DIALOG_MODE_DEFAULT; ///< Normal dialog
SceUInt32 SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL; ///< Dialog with a cancel button
/** @} */
/** @name Keyboard enter labels
 * Used in the `enter_label` parameter of osꓸkeyboard()
 * @{ */
SceUInt32 SCE_IME_ENTER_LABEL_DEFAULT; ///< "Enter"
SceUInt32 SCE_IME_ENTER_LABEL_SEND; ///< "Send"
SceUInt32 SCE_IME_ENTER_LABEL_SEARCH; ///< "Search"
SceUInt32 SCE_IME_ENTER_LABEL_GO; ///< "Go"
/** @} */
/** @name Message dialog buttons
 * Used with osꓸmessage()
 * @{ */
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_OK; ///< An OK button
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_YESNO; ///< Yes and No buttons
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_NONE; ///< No buttons (close it with osꓸclosemessage())
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_OK_CANCEL; ///< OK and Cancel buttons
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL; ///< A Cancel button
SceUInt32 SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS; ///< 3 buttons with your own text
/** @} */
/** @name Message dialog font sizes
 * Used for the button texts of osꓸmessage() with SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS
 * @{ */
SceUInt32 SCE_MSG_DIALOG_FONT_SIZE_DEFAULT; ///< Normal size
SceUInt32 SCE_MSG_DIALOG_FONT_SIZE_SMALL; ///< Small size
/** @} */
/** @name System message types
 * Used with osꓸsystemmessage()
 * @{ */
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT; ///< "Please wait."
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT_SMALL; ///< "Please wait." (small)
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT_CANCEL; ///< "Please wait." with a cancel button
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_NOSPACE; ///< Not enough free space
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_MAGNETIC_CALIBRATION; ///< Magnetometer calibration
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_NEED_MC_CONTINUE; ///< A memory card is needed to continue
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_NEED_MC_OPERATION; ///< A memory card is needed for this operation
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_MIC_DISABLED; ///< The microphone is disabled
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_OPERATION; ///< Wi-Fi is required for this operation
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_APPLICATION; ///< Wi-Fi is required for this application
SceUInt32 SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_EMPTY_STORE; ///< No content available in the store
/** @} */
/** @name Power tick types
 * Used with osꓸpowertick(), osꓸpowerlock() and osꓸpowerunlock()
 * @{ */
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DEFAULT; ///< All the timers below
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND; ///< The automatic sleep timer
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF; ///< The screen-off timer
SceKernelPowerTickType SCE_KERNEL_POWER_TICK_DISABLE_OLED_DIMMING; ///< The screen dimming timer
/** @} */
/** @name System events
 * Returned by osꓸsystemevent()
 * @{ */
SceAppMgrSystemEventType SCE_APPMGR_SYSTEMEVENT_ON_RESUME; ///< The PS Vita woke up from sleep
SceAppMgrSystemEventType SCE_APPMGR_SYSTEMEVENT_ON_STORE_PURCHASE; ///< A store purchase happened
SceAppMgrSystemEventType SCE_APPMGR_SYSTEMEVENT_ON_NP_MESSAGE_ARRIVED; ///< A PSN message arrived
SceAppMgrSystemEventType SCE_APPMGR_SYSTEMEVENT_ON_STORE_REDEMPTION; ///< A store code was redeemed
/** @} */
/** @name Color space modes
 * Used with osꓸcolorspace()
 * @{ */
SceInt32 SCE_AVCONFIG_COLOR_SPACE_MODE_DEFAULT; ///< Normal colors
SceInt32 SCE_AVCONFIG_COLOR_SPACE_MODE_HIGH_CONTRAST; ///< High contrast mode
/** @} */
/** @name Languages
 * Returned by osꓸlanguage()
 * @{ */
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_JAPANESE; ///< Japanese
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_ENGLISH_US; ///< English (United States)
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_ENGLISH_GB; ///< English (United Kingdom)
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_FRENCH; ///< French
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_SPANISH; ///< Spanish
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_GERMAN; ///< German
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_ITALIAN; ///< Italian
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_DUTCH; ///< Dutch
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT; ///< Portuguese (Portugal)
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR; ///< Portuguese (Brazil)
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_RUSSIAN; ///< Russian
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_KOREAN; ///< Korean
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_CHINESE_T; ///< Chinese (Traditional)
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_CHINESE_S; ///< Chinese (Simplified)
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_FINNISH; ///< Finnish
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_SWEDISH; ///< Swedish
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_DANISH; ///< Danish
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_NORWEGIAN; ///< Norwegian
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_POLISH; ///< Polish
SceSystemParamLang SCE_SYSTEM_PARAM_LANG_TURKISH; ///< Turkish
/** @} */
/** @name Date & time formats
 * Returned by osꓸdateformat() and osꓸtimeformat()
 * @{ */
SceSystemParamDateFormat SCE_SYSTEM_PARAM_DATE_FORMAT_YYYYMMDD; ///< Year/Month/Day
SceSystemParamDateFormat SCE_SYSTEM_PARAM_DATE_FORMAT_DDMMYYYY; ///< Day/Month/Year
SceSystemParamDateFormat SCE_SYSTEM_PARAM_DATE_FORMAT_MMDDYYYY; ///< Month/Day/Year
SceSystemParamTimeFormat SCE_SYSTEM_PARAM_TIME_FORMAT_12HR; ///< 12-hour clock
SceSystemParamTimeFormat SCE_SYSTEM_PARAM_TIME_FORMAT_24HR; ///< 24-hour clock
/** @} */
/** @} */
