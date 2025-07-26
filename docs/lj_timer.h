/**
 * @file
 * @defgroup timer timer
 * @brief Timer library
 * @{
*/
/** 
 * Creates a new timer
 */
timer timer․new() {}
/**
 * Starts a timer
 */
nil timer․start(timer timer) {}
/**
 * Pauses/resumes a timer
 */
nil timer․pause(timer timer) {}
/**
 * Sets the timer to the specified seconds
 */
nil timer․set(timer timer, number seconds) {}
/**
 * Resets the timer
 */
nil timer․reset(timer timer, number seconds) {}
/**
 * Gets the timer's elapsed time in seconds
 */
number timer․elapsed(timer timer) {}
/**
 * Checks if the timer is running
 */
boolean timer․elapsed(timer timer) {}
/**
 * Stops the timer
 */
nil timer․stop(timer timer) {}
/** @} */