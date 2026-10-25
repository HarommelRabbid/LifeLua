/**
 * @defgroup string string
 * @brief Extra string functions (extends Lua's string library)
 *
 * These are added to Lua's standard `string` library, so like the other string functions,
 * they can be called as methods on any string:
 * @code
 * local file = "Picture.PNG"
 * print(file:endswith(".png"))        -- false
 * print(file:endswith(".png", true))  -- true (ignoring upper/lower case)
 * @endcode
 * @{
*/
/**
 * Checks if a string starts with another string
 *
 * Method form: `str:startswith(prefix, ignorecase)`
 * @param str The string to check
 * @param prefix The start to look for
 * @param ignorecase `true` to ignore the difference between upper and lower case *(optional, default: false)*
 * @return `true` if `str` starts with `prefix`
*/
boolean stringꓸstartswith(string str, string prefix, boolean ignorecase) {}
/**
 * Checks if a string ends with another string
 *
 * Method form: `str:endswith(suffix, ignorecase)`
 * @param str The string to check
 * @param suffix The end to look for
 * @param ignorecase `true` to ignore the difference between upper and lower case *(optional, default: false)*
 * @return `true` if `str` ends with `suffix`
*/
boolean stringꓸendswith(string str, string suffix, boolean ignorecase) {}
/**
 * Splits a string into a list of parts.
 * Every character of `delimiters` counts as a separator, and empty parts are skipped.
 *
 * Method form: `str:split(delimiters)`
 * @param str The string to split
 * @param delimiters The separator character(s) *(optional, default: `" "`)*
 * @return A list of the parts
 * @par Example:
 * @code
 * local parts = ("ux0:data/myapp/save.txt"):split("/")
 * -- parts = {"ux0:data", "myapp", "save.txt"}
 * local words = ("one, two,three"):split(", ")
 * -- words = {"one", "two", "three"}
 * @endcode
*/
table stringꓸsplit(string str, string delimiters) {}
/** @} */
