/**
 * @file
 * @defgroup video video
 * @todo Library doesn't exist yet in LifeLua, this is purely a concept only
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
 * Pauses a video
 */
nil video․pause(video video, bool pause) {}
/**
 * Fast forward by set seconds
 */
image video․fastforward(video video, number seconds) {}
/**
 * Rewind by set seconds
 */
image video․rewind(video video, number seconds) {}
/** @} */