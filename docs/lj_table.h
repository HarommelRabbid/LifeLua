/**
 * @defgroup table table
 * @brief Extra table functions (extends Lua's table library)
 *
 * These are added to Lua's standard `table` library and work on lists (tables with keys 1, 2, 3, ...).
 * Note that the value comes **first** and the list second.
 * @{
*/
/**
 * Checks if a value is in a list
 * @param value The value to look for
 * @param list The list to search
 * @return `true` if the value is in the list
 * @par Example:
 * @code
 * local fruits = {"apple", "banana"}
 * print(table.find("banana", fruits)) -- true
 * @endcode
*/
boolean tableꓸfind(any value, table list) {}
/**
 * Removes the first time a value appears in a list (the items after it move down by one)
 * @param value The value to remove
 * @param list The list to remove it from
 * @return The removed value, or nothing if it wasn't found
 * @par Example:
 * @code
 * local items = {"a", "b", "a"}
 * table.removefirst("a", items) -- items = {"b", "a"}
 * @endcode
*/
any tableꓸremovefirst(any value, table list) {}
/** @} */
