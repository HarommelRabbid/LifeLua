/**
 * @defgroup video video
 * @brief Video library
 * @{
*/
/**
 * Loads a video
 */
video video․open(string path) {}
/**
 * Plays a video
 */
nil video․play(video video) {}
/**
 * Gets video output as an image
 */
image video․output(video video) {}
/**
 * Stops a video
 */
nil video․stop(video video) {}
/**
 * Closes the video player
 */
nil video․close(video video) {}
/**
 * Pauses a video
 */
nil video․pause(video video, bool pause) {}
/**
 * Go to a part of the video (in seconds)
 */
nil video․seek(video video, number seconds) {}
/**
 * Gets elapsed time of the video
 */
number video․elapsed(video video) {}
/**
 * Checks if the video player is active or not
 */
boolean video․active(video video) {}
/** @} */