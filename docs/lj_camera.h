/**
 * @file
 * @defgroup camera camera
 * @brief Camera library
 * @{
*/

SceCameraDevice SCE_CAMERA_DEVICE_FRONT;
SceCameraDevice SCE_CAMERA_DEVICE_BACK;
SceCameraResolution SCE_CAMERA_RESOLUTION_0_0;     //!< Invalid resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_640_480; //!< VGA resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_320_240; //!< QVGA resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_160_120; //!< QQVGA resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_352_288; //!< CIF resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_176_144; //!< QCIF resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_480_272; //!< PSP resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_640_360; //!< NGP resolution
SceCameraFrameRate SCE_CAMERA_FRAMERATE_3_FPS;     //!< 3.75 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_5_FPS;     //!< 5 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_7_FPS;     //!< 7.5 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_10_FPS;    //!< 10 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_15_FPS;    //!< 15 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_20_FPS;    //!< 20 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_30_FPS;    //!< 30 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_60_FPS;    //!< 60 fps
SceCameraFrameRate SCE_CAMERA_FRAMERATE_120_FPS;   //!< 120 fps (resolution must be QVGA or lower)
SceCameraReverse SCE_CAMERA_REVERSE_OFF; //!< Reverse mode off
SceCameraReverse SCE_CAMERA_REVERSE_MIRROR; //!< Mirror mode
SceCameraReverse SCE_CAMERA_REVERSE_FLIP; //!< Flip mode
SceCameraReverse SCE_CAMERA_REVERSE_MIRROR_FLIP; //!< Mirror + Flip mode
SceCameraEffect SCE_CAMERA_EFFECT_NORMAL;
SceCameraEffect SCE_CAMERA_EFFECT_NEGATIVE;
SceCameraEffect SCE_CAMERA_EFFECT_BLACKWHITE;
SceCameraEffect SCE_CAMERA_EFFECT_SEPIA;
SceCameraEffect SCE_CAMERA_EFFECT_BLUE;
SceCameraEffect SCE_CAMERA_EFFECT_RED;
SceCameraEffect SCE_CAMERA_EFFECT_GREEN;
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_20; //!< -2.0
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_17; //!< -1.7
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_15; //!< -1.5
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_13; //!< -1.3
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_10; //!< -1.0
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_7; //!< -0.7
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_5; //!< -0.5
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_3; //!< -0.3
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_0; //!< +0.0
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_3; //!< +0.3
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_5; //!< +0.5
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_7; //!< +0.7
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_10; //!< +1.0
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_13; //!< +1.3
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_15; //!< +1.5
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_17; //!< +1.7
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_20; //!< +2.0
SceCameraSaturation SCE_CAMERA_SATURATION_0;  //!< 0.0
SceCameraSaturation SCE_CAMERA_SATURATION_5;  //!< 0.5
SceCameraSaturation SCE_CAMERA_SATURATION_10; //!< 1.0
SceCameraSaturation SCE_CAMERA_SATURATION_20; //!< 2.0
SceCameraSaturation SCE_CAMERA_SATURATION_30; //!< 3.0
SceCameraSaturation SCE_CAMERA_SATURATION_40; //!< 4.0
SceCameraSharpness SCE_CAMERA_SHARPNESS_100; //!< 100%
SceCameraSharpness SCE_CAMERA_SHARPNESS_200; //!< 200%
SceCameraSharpness SCE_CAMERA_SHARPNESS_300; //!< 300%
SceCameraSharpness SCE_CAMERA_SHARPNESS_400; //!< 400%
SceCameraAntiFlicker SCE_CAMERA_ANTIFLICKER_AUTO; //!< Automatic mode
SceCameraAntiFlicker SCE_CAMERA_ANTIFLICKER_50HZ; //!< 50 Hz mode
SceCameraAntiFlicker SCE_CAMERA_ANTIFLICKER_60HZ; //!< 50 Hz mode
SceCameraISO SCE_CAMERA_ISO_AUTO; //!< Automatic mode
SceCameraISO SCE_CAMERA_ISO_100; //!< ISO100/21
SceCameraISO SCE_CAMERA_ISO_200; //!< ISO200/24
SceCameraISO SCE_CAMERA_ISO_400; //!< ISO400/27
SceCameraGain SCE_CAMERA_GAIN_AUTO;
SceCameraGain SCE_CAMERA_GAIN_1;
SceCameraGain SCE_CAMERA_GAIN_2;
SceCameraGain SCE_CAMERA_GAIN_3;
SceCameraGain SCE_CAMERA_GAIN_4;
SceCameraGain SCE_CAMERA_GAIN_5;
SceCameraGain SCE_CAMERA_GAIN_6;
SceCameraGain SCE_CAMERA_GAIN_7;
SceCameraGain SCE_CAMERA_GAIN_8;
SceCameraGain SCE_CAMERA_GAIN_9;
SceCameraGain SCE_CAMERA_GAIN_10;
SceCameraGain SCE_CAMERA_GAIN_11;
SceCameraGain SCE_CAMERA_GAIN_12;
SceCameraGain SCE_CAMERA_GAIN_13;
SceCameraGain SCE_CAMERA_GAIN_14;
SceCameraGain SCE_CAMERA_GAIN_15;
SceCameraGain SCE_CAMERA_GAIN_16;
SceCameraWhiteBalance SCE_CAMERA_WB_AUTO; //!< Automatic mode
SceCameraWhiteBalance SCE_CAMERA_WB_DAY; //!< Daylight mode
SceCameraWhiteBalance SCE_CAMERA_WB_CWF; //!< Cool White Fluorescent mode
SceCameraWhiteBalance SCE_CAMERA_WB_SLSA; //!< Standard Light Source A mode
SceCameraBacklight SCE_CAMERA_BACKLIGHT_OFF;
SceCameraBacklight SCE_CAMERA_BACKLIGHT_ON;
SceCameraNightmode SCE_CAMERA_NIGHTMODE_OFF;  //!< Disabled
SceCameraNightmode SCE_CAMERA_NIGHTMODE_LESS10;  //!< 10 lux or below
SceCameraNightmode SCE_CAMERA_NIGHTMODE_LESS100;  //!< 100 lux or below
SceCameraNightmode SCE_CAMERA_NIGHTMODE_OVER100;  //!< 100 lux or over

/** 
 * Opens the camera
*/
nil camera․open(SceCameraDevice type, SceCameraResolution res, SceCameraFrameRate fps) {}
/** 
 * Gets camera output as an image userdata
*/
image camera․output() {}
/** 
 * Checks if the camera is opened/active
*/
boolean camera․active() {}
/** 
 * Reverses the camera
 * @param mode if not defined it'll return the current reverse mode
*/
nil or SceCameraReverse camera․reverse(SceCameraReverse mode) {}
/** 
 * Applies an effect to the camera
 * @param effect if not defined it'll return the current effect(s) applied
*/
nil or SceCameraEffect camera․effect(SceCameraEffect effect) {}
/** 
 * Zooms the camera
 * @param level if not defined it'll return the current zoom
*/
nil or number camera․zoom(number level) {}
/** 
 * Sets the brightness of the camera
 * @param level if not defined it'll return the current brightness
*/
nil or number camera․brightness(number level) {}
/** 
 * Sets the saturation of the camera
 * @param level if not defined it'll return the current saturation
*/
nil or SceCameraSaturation camera․saturation(SceCameraSaturation level) {}
/** 
 * Sets the contrast of the camera
 * @param level if not defined it'll return the current constrast
*/
nil or number camera․contrast(number level) {}
/** 
 * Sets the sharpness of the camera
 * @param level if not defined it'll return the current sharpness
*/
nil or SceCameraSharpness camera․sharpness(SceCameraSharpness level) {}
/** 
 * Sets the exposure of the camera
 * @param level if not defined it'll return the current exposure
*/
nil or SceCameraExposureCompensation camera․exposure(SceCameraExposureCompensation level) {}
/** 
 * Sets the antiflicker of the camera
 * @param mode if not defined it'll return the current antiflicker mode
*/
nil or SceCameraAntiFlicker camera․antiflicker(SceCameraAntiFlicker mode) {}
/** 
 * Sets the ISO of the camera
 * @param mode if not defined it'll return the current ISO
*/
nil or SceCameraISO camera․iso(SceCameraISO mode) {}
/** 
 * Sets the gain of the camera
 * @param mode if not defined it'll return the current gain
*/
nil or SceCameraGain camera․gain(SceCameraGain mode) {}
/** 
 * Sets the white balance of the camera
 * @param mode if not defined it'll return the current white balance
*/
nil or SceCameraWhiteBalance camera․whitebalance(SceCameraWhiteBalance mode) {}
/** 
 * Sets the backlight of the camera
 * @param mode if not defined it'll return the current backlight
*/
nil or SceCameraBacklight camera․backlight(SceCameraBacklight mode) {}
/** 
 * Sets the night vision of the camera
 * @param mode if not defined it'll return the current night vision
*/
nil or SceCameraNightmode camera․nightvision(SceCameraNightmode mode) {}
/** 
 * Sets the noise reduction of the camera
 * @param level if not defined it'll return the current noise reduction
*/
nil or number camera․noisereduction(number level) {}
/** 
 * Closes the camera
*/
nil camera․close() {}

/** @} */