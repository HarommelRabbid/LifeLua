/**
 * @defgroup camera camera
 * @brief Using the front and back cameras
 *
 * Start a camera with cameraꓸstart(), then get each new frame as an image with cameraꓸoutput().
 * Only one camera can be running at a time. All the setting functions work the same way:
 * call them with a value to change the setting, or without one to get the current value.
 * They raise an error if the camera wasn't started.
 *
 * @code
 * camera.start(SCE_CAMERA_DEVICE_BACK, SCE_CAMERA_RESOLUTION_640_480)
 * camera.effect(SCE_CAMERA_EFFECT_SEPIA)
 * while true do
 *     controls.update()
 *     local frame = camera.output()
 *     frame:display(160, 32)
 *     local qr = frame:qrscan()
 *     if qr then draw.text(10, 10, "QR code: "..qr, color.new(255, 255, 255)) end
 *     if controls.pressed(SCE_CTRL_CIRCLE) then break end
 *     draw.swapbuffers()
 * end
 * camera.stop()
 * @endcode
 * @{
*/
/** @name Devices
 * Used with cameraꓸstart()
 * @{ */
SceCameraDevice SCE_CAMERA_DEVICE_FRONT; ///< Front camera
SceCameraDevice SCE_CAMERA_DEVICE_BACK; ///< Back camera
/** @} */
/** @name Resolutions
 * Used with cameraꓸstart()
 * @{ */
SceCameraResolution SCE_CAMERA_RESOLUTION_0_0; ///< Invalid resolution
SceCameraResolution SCE_CAMERA_RESOLUTION_640_480; ///< 640 × 480 (VGA)
SceCameraResolution SCE_CAMERA_RESOLUTION_320_240; ///< 320 × 240 (QVGA), supports 120 FPS
SceCameraResolution SCE_CAMERA_RESOLUTION_160_120; ///< 160 × 120 (QQVGA), supports 120 FPS
SceCameraResolution SCE_CAMERA_RESOLUTION_352_288; ///< 352 × 288 (CIF), supports 120 FPS
SceCameraResolution SCE_CAMERA_RESOLUTION_176_144; ///< 176 × 144 (QCIF)
SceCameraResolution SCE_CAMERA_RESOLUTION_480_272; ///< 480 × 272 (PSP screen size)
SceCameraResolution SCE_CAMERA_RESOLUTION_640_360; ///< 640 × 360
/** @} */
/** @name Frame rates
 * Used with cameraꓸstart()
 * @{ */
SceCameraFrameRate SCE_CAMERA_FRAMERATE_3_FPS; ///< 3 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_5_FPS; ///< 5 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_7_FPS; ///< 7 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_10_FPS; ///< 10 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_15_FPS; ///< 15 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_20_FPS; ///< 20 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_30_FPS; ///< 30 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_60_FPS; ///< 60 FPS
SceCameraFrameRate SCE_CAMERA_FRAMERATE_120_FPS; ///< 120 FPS (only at 320 × 240, 160 × 120 and 352 × 288, otherwise 60 FPS is used)
/** @} */
/** @name Reverse modes
 * Used with cameraꓸreverse()
 * @{ */
SceCameraReverse SCE_CAMERA_REVERSE_OFF; ///< Normal picture
SceCameraReverse SCE_CAMERA_REVERSE_MIRROR; ///< Mirrored horizontally
SceCameraReverse SCE_CAMERA_REVERSE_FLIP; ///< Flipped vertically
SceCameraReverse SCE_CAMERA_REVERSE_MIRROR_FLIP; ///< Mirrored and flipped
/** @} */
/** @name Effects
 * Used with cameraꓸeffect()
 * @{ */
SceCameraEffect SCE_CAMERA_EFFECT_NORMAL; ///< No effect
SceCameraEffect SCE_CAMERA_EFFECT_NEGATIVE; ///< Inverted colors
SceCameraEffect SCE_CAMERA_EFFECT_BLACKWHITE; ///< Black & white
SceCameraEffect SCE_CAMERA_EFFECT_SEPIA; ///< Sepia
SceCameraEffect SCE_CAMERA_EFFECT_BLUE; ///< Blue tint
SceCameraEffect SCE_CAMERA_EFFECT_RED; ///< Red tint
SceCameraEffect SCE_CAMERA_EFFECT_GREEN; ///< Green tint
/** @} */
/** @name Saturation levels
 * Used with cameraꓸsaturation(), from least to most saturated
 * @{ */
SceCameraSaturation SCE_CAMERA_SATURATION_0; ///< 0.0
SceCameraSaturation SCE_CAMERA_SATURATION_5; ///< 0.5
SceCameraSaturation SCE_CAMERA_SATURATION_10; ///< 1.0 (normal)
SceCameraSaturation SCE_CAMERA_SATURATION_20; ///< 2.0
SceCameraSaturation SCE_CAMERA_SATURATION_30; ///< 3.0
SceCameraSaturation SCE_CAMERA_SATURATION_40; ///< 4.0
/** @} */
/** @name Sharpness levels
 * Used with cameraꓸsharpness()
 * @{ */
SceCameraSharpness SCE_CAMERA_SHARPNESS_100; ///< 100% (normal)
SceCameraSharpness SCE_CAMERA_SHARPNESS_200; ///< 200%
SceCameraSharpness SCE_CAMERA_SHARPNESS_300; ///< 300%
SceCameraSharpness SCE_CAMERA_SHARPNESS_400; ///< 400%
/** @} */
/** @name Exposure compensation levels
 * Used with cameraꓸexposure(). The number is the exposure value × 10, e.g. `NEGATIVE_15` is -1.5 EV.
 * @{ */
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_20; ///< -2.0 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_17; ///< -1.7 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_15; ///< -1.5 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_13; ///< -1.3 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_10; ///< -1.0 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_7; ///< -0.7 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_5; ///< -0.5 EV
SceCameraExposureCompensation SCE_CAMERA_EV_NEGATIVE_3; ///< -0.3 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_0; ///< 0 EV (normal)
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_3; ///< +0.3 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_5; ///< +0.5 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_7; ///< +0.7 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_10; ///< +1.0 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_13; ///< +1.3 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_15; ///< +1.5 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_17; ///< +1.7 EV
SceCameraExposureCompensation SCE_CAMERA_EV_POSITIVE_20; ///< +2.0 EV
/** @} */
/** @name Anti-flicker modes
 * Used with cameraꓸantiflicker(). Reduces flickering caused by lights running at 50 or 60 Hz.
 * @{ */
SceCameraAntiFlicker SCE_CAMERA_ANTIFLICKER_AUTO; ///< Detect automatically
SceCameraAntiFlicker SCE_CAMERA_ANTIFLICKER_50HZ; ///< 50 Hz
SceCameraAntiFlicker SCE_CAMERA_ANTIFLICKER_60HZ; ///< 60 Hz
/** @} */
/** @name ISO levels
 * Used with cameraꓸiso()
 * @{ */
SceCameraISO SCE_CAMERA_ISO_AUTO; ///< Automatic
SceCameraISO SCE_CAMERA_ISO_100; ///< ISO 100
SceCameraISO SCE_CAMERA_ISO_200; ///< ISO 200
SceCameraISO SCE_CAMERA_ISO_400; ///< ISO 400
/** @} */
/** @name Gain levels
 * Used with cameraꓸgain()
 * @{ */
SceCameraGain SCE_CAMERA_GAIN_AUTO; ///< Automatic
SceCameraGain SCE_CAMERA_GAIN_1; ///< Gain 1
SceCameraGain SCE_CAMERA_GAIN_2; ///< Gain 2
SceCameraGain SCE_CAMERA_GAIN_3; ///< Gain 3
SceCameraGain SCE_CAMERA_GAIN_4; ///< Gain 4
SceCameraGain SCE_CAMERA_GAIN_5; ///< Gain 5
SceCameraGain SCE_CAMERA_GAIN_6; ///< Gain 6
SceCameraGain SCE_CAMERA_GAIN_7; ///< Gain 7
SceCameraGain SCE_CAMERA_GAIN_8; ///< Gain 8
SceCameraGain SCE_CAMERA_GAIN_9; ///< Gain 9
SceCameraGain SCE_CAMERA_GAIN_10; ///< Gain 10
SceCameraGain SCE_CAMERA_GAIN_11; ///< Gain 11
SceCameraGain SCE_CAMERA_GAIN_12; ///< Gain 12
SceCameraGain SCE_CAMERA_GAIN_13; ///< Gain 13
SceCameraGain SCE_CAMERA_GAIN_14; ///< Gain 14
SceCameraGain SCE_CAMERA_GAIN_15; ///< Gain 15
SceCameraGain SCE_CAMERA_GAIN_16; ///< Gain 16
/** @} */
/** @name White balance modes
 * Used with cameraꓸwhitebalance()
 * @{ */
SceCameraWhiteBalance SCE_CAMERA_WB_AUTO; ///< Automatic
SceCameraWhiteBalance SCE_CAMERA_WB_DAY; ///< Daylight
SceCameraWhiteBalance SCE_CAMERA_WB_CWF; ///< Cool white fluorescent light
SceCameraWhiteBalance SCE_CAMERA_WB_SLSA; ///< Standard light source A (incandescent light)
/** @} */
/** @name Backlight modes
 * Used with cameraꓸbacklight()
 * @{ */
SceCameraBacklight SCE_CAMERA_BACKLIGHT_OFF; ///< Backlight compensation off
SceCameraBacklight SCE_CAMERA_BACKLIGHT_ON; ///< Backlight compensation on
/** @} */
/** @name Night modes
 * Used with cameraꓸnightmode()
 * @{ */
SceCameraNightmode SCE_CAMERA_NIGHTMODE_OFF; ///< Night mode off
SceCameraNightmode SCE_CAMERA_NIGHTMODE_LESS10; ///< For 10 lux or below
SceCameraNightmode SCE_CAMERA_NIGHTMODE_LESS100; ///< For 100 lux or below
SceCameraNightmode SCE_CAMERA_NIGHTMODE_OVER100; ///< For 100 lux or over
/** @} */
/**
 * Starts a camera. Raises an error if a camera is already running.
 * @param device Which camera, one of the device constants
 * @param resolution One of the resolution constants
 * @param fps One of the frame rate constants *(optional, default: SCE_CAMERA_FRAMERATE_60_FPS)*
*/
nil cameraꓸstart(SceCameraDevice device, SceCameraResolution resolution, SceCameraFrameRate fps) {}
/**
 * Gets the latest camera frame
 * @return The frame as an image. It's always the same image object, updated with the new frame each time you call this,
 * so save it with imageꓸsave() if you want to keep a frame (e.g. to take a photo).
*/
image cameraꓸoutput() {}
/**
 * Stops the camera
*/
nil cameraꓸstop() {}
/**
 * Checks if the camera is running
 * @return `true` if it's running
*/
boolean cameraꓸactive() {}
/**
 * Gets or sets the mirroring/flipping of the picture
 * @param mode One of the reverse mode constants *(optional)*
 * @return The current reverse mode if `mode` is omitted
*/
SceCameraReverse cameraꓸreverse(SceCameraReverse mode) {}
/**
 * Gets or sets the color effect
 * @param effect One of the effect constants *(optional)*
 * @return The current effect if `effect` is omitted
*/
SceCameraEffect cameraꓸeffect(SceCameraEffect effect) {}
/**
 * Gets or sets the zoom
 * @param level Zoom level, a higher value zooms in more *(optional)*
 * @return The current zoom level if `level` is omitted
*/
number cameraꓸzoom(number level) {}
/**
 * Gets or sets the brightness
 * @param level Brightness from 0 to 255 *(optional)*
 * @return The current brightness if `level` is omitted
*/
number cameraꓸbrightness(number level) {}
/**
 * Gets or sets the saturation
 * @param level One of the saturation constants *(optional)*
 * @return The current saturation if `level` is omitted
*/
SceCameraSaturation cameraꓸsaturation(SceCameraSaturation level) {}
/**
 * Gets or sets the contrast
 * @param level Contrast from 0 to 255 *(optional)*
 * @return The current contrast if `level` is omitted
*/
number cameraꓸcontrast(number level) {}
/**
 * Gets or sets the sharpness
 * @param level One of the sharpness constants *(optional)*
 * @return The current sharpness if `level` is omitted
*/
SceCameraSharpness cameraꓸsharpness(SceCameraSharpness level) {}
/**
 * Gets or sets the exposure compensation
 * @param level One of the exposure compensation constants *(optional)*
 * @return The current exposure compensation if `level` is omitted
*/
SceCameraExposureCompensation cameraꓸexposure(SceCameraExposureCompensation level) {}
/**
 * Gets or sets the anti-flicker mode
 * @param mode One of the anti-flicker constants *(optional)*
 * @return The current anti-flicker mode if `mode` is omitted
*/
SceCameraAntiFlicker cameraꓸantiflicker(SceCameraAntiFlicker mode) {}
/**
 * Gets or sets the ISO
 * @param mode One of the ISO constants *(optional)*
 * @return The current ISO if `mode` is omitted
*/
SceCameraISO cameraꓸiso(SceCameraISO mode) {}
/**
 * Gets or sets the gain
 * @param mode One of the gain constants *(optional)*
 * @return The current gain if `mode` is omitted
*/
SceCameraGain cameraꓸgain(SceCameraGain mode) {}
/**
 * Gets or sets the white balance
 * @param mode One of the white balance constants *(optional)*
 * @return The current white balance if `mode` is omitted
*/
SceCameraWhiteBalance cameraꓸwhitebalance(SceCameraWhiteBalance mode) {}
/**
 * Gets or sets backlight compensation
 * @param mode One of the backlight constants *(optional)*
 * @return The current backlight mode if `mode` is omitted
*/
SceCameraBacklight cameraꓸbacklight(SceCameraBacklight mode) {}
/**
 * Gets or sets the night mode, which helps in dark places
 * @param mode One of the night mode constants *(optional)*
 * @return The current night mode if `mode` is omitted
*/
SceCameraNightmode cameraꓸnightmode(SceCameraNightmode mode) {}
/**
 * Gets or sets the noise reduction
 * @param level Noise reduction level, a higher value removes more noise *(optional)*
 * @return The current noise reduction level if `level` is omitted
*/
number cameraꓸnoisereduction(number level) {}
/** @} */
