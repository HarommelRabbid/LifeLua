/**
 * @defgroup timer timer
 * @brief Measuring time (stopwatches)
 *
 * Every function that takes a timer as its first parameter can also be called as a method,
 * e.g. `t:start()` and `t:elapsed()`.
 *
 * @code
 * local t = timer.new()
 * t:start()
 * while true do
 *     controls.update()
 *     if controls.pressed(SCE_CTRL_CROSS) then t:pause() end -- pause/resume
 *     draw.text(10, 10, string.format("%.2f seconds", t:elapsed()), color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * @endcode
 * @{
*/
/**
 * Creates a new timer. Call timerꓸstart() to start it.
 * @return The new timer
*/
timer timerꓸnew() {}
/**
 * Starts (or restarts) a timer from 0
 *
 * Method form: `t:start()`
 * @param timer The timer
*/
nil timerꓸstart(timer timer) {}
/**
 * Pauses a running timer, or resumes a paused one. The time while it's paused isn't counted.
 *
 * Method form: `t:pause()`
 * @param timer The timer
*/
nil timerꓸpause(timer timer) {}
/**
 * Stops a timer. timerꓸelapsed() keeps returning the time it had when stopped.
 *
 * Method form: `t:stop()`
 * @param timer The timer
*/
nil timerꓸstop(timer timer) {}
/**
 * Stops a timer and sets it back to 0
 *
 * Method form: `t:reset()`
 * @param timer The timer
*/
nil timerꓸreset(timer timer) {}
/**
 * Sets the elapsed time of a timer and starts it running from there
 *
 * Method form: `t:set(seconds)`
 * @param timer The timer
 * @param seconds The new elapsed time, in seconds
 * @par Example:
 * @code
 * t:set(60) -- continue counting from 1 minute
 * @endcode
*/
nil timerꓸset(timer timer, number seconds) {}
/**
 * Gets the elapsed time of a timer
 *
 * Method form: `t:elapsed()`
 * @param timer The timer
 * @return The elapsed time in seconds (with decimals)
*/
number timerꓸelapsed(timer timer) {}
/**
 * Checks if a timer is running
 *
 * Method form: `t:running()`
 * @param timer The timer
 * @return `true` if it's running, `false` if it's stopped, paused or reset
*/
boolean timerꓸrunning(timer timer) {}
/** @} */
