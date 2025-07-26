/**
 * @file
 * @defgroup controls controls
 * Controls, touch & motion
 * @brief Controls, touch & motion
 * @{
*/
SceCtrlButtons SCE_CTRL_SELECT;
SceCtrlButtons SCE_CTRL_L3;
SceCtrlButtons SCE_CTRL_R3;
SceCtrlButtons SCE_CTRL_START;
SceCtrlButtons SCE_CTRL_UP;
SceCtrlButtons SCE_CTRL_RIGHT;
SceCtrlButtons SCE_CTRL_DOWN;
SceCtrlButtons SCE_CTRL_LEFT;
SceCtrlButtons SCE_CTRL_LTRIGGER;
SceCtrlButtons SCE_CTRL_L2;
SceCtrlButtons SCE_CTRL_RTRIGGER;
SceCtrlButtons SCE_CTRL_R2;
SceCtrlButtons SCE_CTRL_L1;
SceCtrlButtons SCE_CTRL_R1;
SceCtrlButtons SCE_CTRL_TRIANGLE;
SceCtrlButtons SCE_CTRL_CIRCLE;
SceCtrlButtons SCE_CTRL_CROSS;
SceCtrlButtons SCE_CTRL_SQUARE;
SceCtrlButtons SCE_CTRL_INTERCEPTED;
SceCtrlButtons SCE_CTRL_PSBUTTON;
SceCtrlButtons SCE_CTRL_HEADPHONE;
SceCtrlButtons SCE_CTRL_VOLUP;
SceCtrlButtons SCE_CTRL_VOLDOWN;
SceCtrlButtons SCE_CTRL_POWER;
SceCtrlButtons SCE_CTRL_ACCEPT;
SceCtrlButtons SCE_CTRL_CANCEL;
/** 
 * Updates the controls
 * @param ext Extended controls, will update even if there are common dialogs running (IME keyboard, message etc.), optional
 * @param bind Binds the LTRIGGER & RTRIGGER to L1 & R1 respectively, optional
*/
nil controls․update(boolean ext, boolean bind) {}
/** 
 * Checks if a button has been pressed
*/
boolean controls․pressed(SceCtrlButtons button) {}
/** 
 * Checks if a button has been held
*/
boolean controls․held(SceCtrlButtons button) {}
/** 
 * Checks if a button has been released
*/
boolean controls․released(SceCtrlButtons button) {}
/** 
 * Checks if a button was triggered
*/
boolean controls․check(SceCtrlButtons button) {}
/** 
 * Sets the vibration intensity on a DualShock controller
 * @param port Optional
*/
nil controls․check(number min, number max, number port) {}
/** 
 * Sets the light bar color on a DualShock 4 controller
 * @param port Optional
*/
nil controls․check(number r, number g, number b, number port) {}
/** 
 * Left analog stick input
 * @par Example:
 * @code
 * lx, ly = controls.leftanalog()
 * @endcode
 * @return X and Y of the left analog stick, as 2 values
*/
number controls․leftanalog() {}
/** 
 * Right analog stick input
 * @par Example:
 * @code
 * rx, ry = controls.rightanalog()
 * @endcode
 * @return X and Y of the Right analog stick, as 2 values
*/
number controls․rightanalog() {}
/** 
 * Accelerometer input
 * @par Example:
 * @code
 * acc = controls.accelerometer()
 * -- acc.x, acc.y, acc.z
 * @endcode
 * @return A table with the X, Y and Z of the accelerometer
*/
number controls․accelerometer() {}
/** 
 * Gyroscope input
 * @par Example:
 * @code
 * gyro = controls.gyroscope()
 * -- gyro.x, gyro.y, gyro.z
 * @endcode
 * @return A table with the X, Y and Z of the gyroscope
*/
number controls․gyroscope() {}
/** 
 * Front touch pad input
 * @par Example:
 * @code
 * touch = controls.fronttouch()
 * -- touch[1].x, touch[1].y, touch[1].force, touch[1].id
 * -- touch[2].x, touch[2].y, touch[2].force, touch[2].id
 * -- index will increase if more fingers are touching the screen...
 * @endcode
 * @return Tables with the X, Y, force and ID of the touch
*/
number controls․fronttouch() {}
/** 
 * Rear touch pad input
 * @par Example:
 * @code
 * touch = controls.reartouch()
 * -- touch[1].x, touch[1].y, touch[1].force, touch[1].id
 * -- touch[2].x, touch[2].y, touch[2].force, touch[2].id
 * -- index will increase if more fingers are touching the rear touch pad...
 * @endcode
 * @return Tables with the X, Y, force and ID of the touch
*/
number controls․reartouch() {}
/** 
 * Sets the lightbar on DualShock 4s
*/
nil controls․lightbar(number r, number g, number b) {}
/** 
 * Sets the vibration intensity on DualShock controllers
 * @param port optional
*/
nil controls․vibrate(number small, number large, number port) {}
/** @} */