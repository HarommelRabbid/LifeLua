/**
 * @defgroup video video
 * @brief Playing videos (uses the system's AvPlayer)
 *
 * Plays .MP4 videos (H.264 video with AAC audio). Only one video can be loaded at a time,
 * so these functions don't take a video object; they all control the currently loaded video.
 * All the functions except videoꓸload() raise an error if no video is loaded.
 *
 * @code
 * video.load("ux0:data/myapp/intro.mp4", true) -- load and start playing
 * while video.active() do
 *     controls.update()
 *     video.output() -- draw the current frame
 *     if controls.pressed(SCE_CTRL_CROSS) then video.stop() end
 *     draw.swapbuffers()
 * end
 * video.close()
 * @endcode
 *
 * @note Loading a video stops any audio playing with the @ref audio "audio" library.
 * @{
*/
/**
 * Loads a video. Does nothing if a video is already loaded (close it first with videoꓸclose()).
 * @param path Path to the video file
 * @param autostart `true` to start playing right away *(optional, default: false)*
 * @param loop `true` to start over every time it ends *(optional, default: false)*
*/
nil videoꓸload(string path, boolean autostart, boolean loop) {}
/**
 * Draws the current video frame, scaled to fill the screen's height and centered.
 * Call it every frame, before drawꓸswapbuffers().
*/
nil videoꓸoutput() {}
/**
 * Checks if the video is playing
 * @return `true` while the video is playing, `false` once it has ended or was stopped
*/
boolean videoꓸactive() {}
/**
 * Pauses the video
*/
nil videoꓸpause() {}
/**
 * Resumes a paused video
*/
nil videoꓸresume() {}
/**
 * Gets how far the video has played
 * @return The current position in **milliseconds**
*/
number videoꓸelapsed() {}
/**
 * Jumps to a position in the video
 * @param seconds Position to jump to, in seconds from the start
*/
nil videoꓸseek(number seconds) {}
/**
 * Stops the video. Call videoꓸclose() afterwards to free it.
*/
nil videoꓸstop() {}
/**
 * Closes the video and frees its memory, so another one can be loaded
*/
nil videoꓸclose() {}
/** @} */
