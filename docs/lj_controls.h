/**
 * @defgroup controls controls
 * @brief Buttons, analog sticks, touch screens & motion sensors
 *
 * Call controlsꓸupdate() once at the start of every frame, then check the buttons and sensors.
 * The difference between the button functions:
 * * controlsꓸpressed(): `true` only on the frame the button goes down (good for menus)
 * * controlsꓸheld(): `true` on every frame while the button stays down, except the first one
 * * controlsꓸreleased(): `true` only on the frame the button is let go
 * * controlsꓸcheck(): `true` on every frame while the button is down
 *
 * @code
 * local x = 480
 * while true do
 *     controls.update()
 *     if controls.check(SCE_CTRL_LEFT) then x = x - 4 end
 *     if controls.check(SCE_CTRL_RIGHT) then x = x + 4 end
 *     if controls.pressed(SCE_CTRL_START) then break end -- exit the loop
 *     draw.circle(x, 272, 20, color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * @endcode
 *
 * You can check several buttons at once by adding them together, e.g. `controls.check(SCE_CTRL_L1 + SCE_CTRL_R1)`.
 * @{
*/
/** @name Buttons
 * @{ */
SceCtrlButtons SCE_CTRL_UP; ///< D-pad up
SceCtrlButtons SCE_CTRL_DOWN; ///< D-pad down
SceCtrlButtons SCE_CTRL_LEFT; ///< D-pad left
SceCtrlButtons SCE_CTRL_RIGHT; ///< D-pad right
SceCtrlButtons SCE_CTRL_CROSS; ///< Cross (✕) button
SceCtrlButtons SCE_CTRL_CIRCLE; ///< Circle (○) button
SceCtrlButtons SCE_CTRL_SQUARE; ///< Square (□) button
SceCtrlButtons SCE_CTRL_TRIANGLE; ///< Triangle (△) button
SceCtrlButtons SCE_CTRL_ACCEPT; ///< The "confirm" button set in the system settings (Cross or Circle, depending on the console's region/settings)
SceCtrlButtons SCE_CTRL_CANCEL; ///< The "back" button set in the system settings (Circle or Cross, the opposite of SCE_CTRL_ACCEPT)
SceCtrlButtons SCE_CTRL_LTRIGGER; ///< L button
SceCtrlButtons SCE_CTRL_RTRIGGER; ///< R button
SceCtrlButtons SCE_CTRL_L1; ///< L1 button (external controllers, or the L button when `bind` is used in controlsꓸupdate())
SceCtrlButtons SCE_CTRL_R1; ///< R1 button (external controllers, or the R button when `bind` is used in controlsꓸupdate())
SceCtrlButtons SCE_CTRL_L2; ///< L2 button (external controllers)
SceCtrlButtons SCE_CTRL_R2; ///< R2 button (external controllers)
SceCtrlButtons SCE_CTRL_L3; ///< L3 button, pressing the left stick (external controllers)
SceCtrlButtons SCE_CTRL_R3; ///< R3 button, pressing the right stick (external controllers)
SceCtrlButtons SCE_CTRL_START; ///< Start button
SceCtrlButtons SCE_CTRL_SELECT; ///< Select button
SceCtrlButtons SCE_CTRL_PSBUTTON; ///< PS button (only detected while it's locked with osꓸlock() and SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN)
SceCtrlButtons SCE_CTRL_POWER; ///< Power button
SceCtrlButtons SCE_CTRL_VOLUP; ///< Volume up button
SceCtrlButtons SCE_CTRL_VOLDOWN; ///< Volume down button
SceCtrlButtons SCE_CTRL_HEADPHONE; ///< Headphones are plugged in
SceCtrlButtons SCE_CTRL_INTERCEPTED; ///< The input is being used by the system (e.g. the quick menu is open)
/** @} */
/**
 * Reads the latest state of the buttons, sticks, touch screens and motion sensors.
 * Call this once per frame, before using any other function of this library.
 * @param ext Use the extended reading mode, which keeps reading input even while system dialogs (keyboard, messages, etc.) are open *(optional, default: false)*
 * @param bind Makes L and R also count as L1 and R1 *(optional, default: false)*
*/
nil controlsꓸupdate(boolean ext, boolean bind) {}
/**
 * Checks if a button was just pressed (it's down now, but wasn't on the previous controlsꓸupdate())
 * @param button One of the button constants
 * @return `true` on the frame the button was pressed
*/
boolean controlsꓸpressed(SceCtrlButtons button) {}
/**
 * Checks if a button is being held down (it was already down on the previous controlsꓸupdate())
 * @param button One of the button constants
 * @return `true` while the button is held, except on the frame it was first pressed
*/
boolean controlsꓸheld(SceCtrlButtons button) {}
/**
 * Checks if a button was just released (it's up now, but was down on the previous controlsꓸupdate())
 * @param button One of the button constants
 * @return `true` on the frame the button was released
*/
boolean controlsꓸreleased(SceCtrlButtons button) {}
/**
 * Checks if a button is down right now
 * @param button One of the button constants
 * @return `true` while the button is down
*/
boolean controlsꓸcheck(SceCtrlButtons button) {}
/**
 * Gets the position of the left analog stick
 * @return 2 numbers, X and Y, each from -128 to 127 (0 is the center, negative is left/up)
 * @par Example:
 * @code
 * local lx, ly = controls.leftanalog()
 * if math.abs(lx) > 20 then player_x = player_x + lx / 32 end -- ignore small movements (dead zone)
 * @endcode
*/
number controlsꓸleftanalog() {}
/**
 * Gets the position of the right analog stick
 * @return 2 numbers, X and Y, each from -128 to 127 (0 is the center, negative is left/up)
 * @par Example:
 * @code
 * local rx, ry = controls.rightanalog()
 * @endcode
*/
number controlsꓸrightanalog() {}
/**
 * Gets the accelerometer (tilt) readings
 * @return A table with the fields `x`, `y` and `z`
 * @par Example:
 * @code
 * local acc = controls.accelerometer()
 * draw.text(10, 10, "X: "..acc.x.." Y: "..acc.y.." Z: "..acc.z, white)
 * @endcode
*/
table controlsꓸaccelerometer() {}
/**
 * Gets the gyroscope (rotation speed) readings
 * @return A table with the fields `x`, `y` and `z`
 * @par Example:
 * @code
 * local gyro = controls.gyroscope()
 * draw.text(10, 10, "X: "..gyro.x.." Y: "..gyro.y.." Z: "..gyro.z, white)
 * @endcode
*/
table controlsꓸgyroscope() {}
/**
 * Gets the fingers touching the front touch screen
 * @return A list with one table per finger (empty if nothing is touching). Each table has these fields:
 * * `x`, `y`: Position in screen pixels
 * * `force`: How hard the finger is pressing
 * * `id`: A number that stays the same for a finger while it keeps touching
 * @par Example:
 * @code
 * local touch = controls.fronttouch()
 * for i = 1, #touch do
 *     draw.circle(touch[i].x, touch[i].y, 30, color.new(255, 255, 255))
 * end
 * @endcode
*/
table controlsꓸfronttouch() {}
/**
 * Gets the fingers touching the rear touch pad. Works like controlsꓸfronttouch().
 * @return A list with one table per finger (empty if nothing is touching), each with the fields `x`, `y`, `force` and `id`
*/
table controlsꓸreartouch() {}
/**
 * Makes a connected DualShock controller vibrate (e.g. on a PS TV). Set both motors to 0 to stop.
 * @param small Strength of the small motor, 0-255
 * @param large Strength of the large motor, 0-255
 * @param port Controller port *(optional, default: 1)*
*/
nil controlsꓸvibrate(number small, number large, number port) {}
/**
 * Sets the light bar color of a connected DualShock 4 controller
 * @param r Red, 0-255
 * @param g Green, 0-255
 * @param b Blue, 0-255
 * @param port Controller port *(optional, default: 1)*
*/
nil controlsꓸlightbar(number r, number g, number b, number port) {}
/** @} */
