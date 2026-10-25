/**
 * @defgroup math math
 * @brief Extra math functions (extends Lua's math library)
 *
 * These are added to Lua's standard `math` library, so `math.floor()`, `math.random()`, etc. still work
 * like in normal Lua (see the [Lua manual](https://www.lua.org/manual/5.1/manual.html#5.6)).
 * @{
*/
/**
 * Keeps a number between a minimum and a maximum (also known as "clamp")
 * @param num The number
 * @param min The minimum
 * @param max The maximum
 * @return `min` if `num` is smaller than it, `max` if `num` is bigger than it, otherwise `num`
 * @par Example:
 * @code
 * volume = math.range(volume + 10, 0, 100) -- never goes over 100
 * @endcode
*/
number mathꓸrange(number num, number min, number max) {}
/**
 * Checks if a number is between a minimum and a maximum (both included)
 * @param num The number
 * @param min The minimum
 * @param max The maximum
 * @return `true` if `min <= num <= max`
 * @par Example:
 * @code
 * -- Is the touch inside a button?
 * if math.inrange(touch.x, 100, 300) and math.inrange(touch.y, 50, 100) then ... end
 * @endcode
*/
boolean mathꓸinrange(number num, number min, number max) {}
/**
 * Rounds a number to the nearest whole number, or to a number of decimal places
 * @param num The number
 * @param decimals How many decimal places to keep *(optional, default: 0)*
 * @return The rounded number
 * @par Example:
 * @code
 * print(math.round(2.5))        -- 3
 * print(math.round(3.14159, 2)) -- 3.14
 * @endcode
*/
number mathꓸround(number num, number decimals) {}
/** @} */
