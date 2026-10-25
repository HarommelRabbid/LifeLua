/**
 * @defgroup thread thread
 * @brief Running a function at the same time as the rest of your app
 *
 * Every function that takes a thread as its first parameter can also be called as a method,
 * e.g. `t:start()` and `t:wait()`.
 *
 * @code
 * local done = false
 * local t = thread.new(function()
 *     io.extract("ux0:data/big.zip", "ux0:data/big") -- slow work
 *     done = true
 * end)
 * t:start()
 * while not done do
 *     draw.text(10, 10, "Extracting...", color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * @endcode
 *
 * @warning Threads are experimental: they share the same Lua state as the main code, so running heavy code in a thread,
 * or using the same variables/objects from several threads at once, may crash the app. Keep thread functions simple.
 * @{
*/
/**
 * Creates a new thread. It doesn't run until you call threadꓸstart().
 * @param func The function the thread will run
 * @param name Name of the thread, only useful for debugging *(optional, default: `"LifeLua Thread"`)*
 * @return The new thread
*/
thread threadꓸnew(function func, string name) {}
/**
 * Starts running a thread
 *
 * Method form: `t:start()`
 * @param thread The thread
*/
nil threadꓸstart(thread thread) {}
/**
 * Waits until a thread has finished. The code calling this stops until then.
 *
 * Method form: `t:wait()`
 * @param thread The thread
*/
nil threadꓸwait(thread thread) {}
/**
 * Waits until a thread has finished, like threadꓸwait(), but lets system callbacks run while waiting
 *
 * Method form: `t:waitcb()`
 * @param thread The thread
*/
nil threadꓸwaitcb(thread thread) {}
/**
 * Ends the thread that calls it. Call it from inside a thread function to stop it early.
*/
nil threadꓸexit() {}
/**
 * Ends the thread that calls it and frees its resources. Call it from inside a thread function to stop it early.
*/
nil threadꓸexitdelete() {}
/** @} */
