/**
 * @defgroup string string
 * Method calling will work on all of the functions listed here, just like other Lua string functions (e. g. `(""):startswith` & `(""):endswith`)
 * @brief Lua string library extension
 * @{
*/
/** 
 * Checks if string starts with a substring
*/
boolean string․startswith(string string, string sub) {}
/** 
 * Checks if string ends with a substring
*/
boolean string․endswith(string string, string sub) {}
/** 
 * Splits a string with a delimiter
 * @param delimiter optional, will default to a space if not defined
*/
table string․split(string string, string delimiter) {}
/** @} */