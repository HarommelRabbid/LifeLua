/**
 * @file
 * @defgroup io io
 * @brief Lua IO library extension
 * @{
*/
/**
 * Gets the CRC32 hash of a string
 */
number io․crc32(string string) {}
/**
 * Gets the SHA1 hash of a file
 */
number io․sha1(string path) {}
/**
 * Gets or sets the working path
 * @param path If defined then the work path will be set to the defined path
 */
nil or boolean io․workpath(string path) {}
/**
 * Checks if a file or folder exists or not
 */
boolean io․exists(string path) {}
/**
 * Creates a new folder
 */
nil io․newfolder(string path) {}
/**
 * Reads a .SFO file
 * @return A table with the keys & values of the .SFO
 */
table io․readsfo(string path) {}
/**
 * Edits a parameter in a .SFO file
 */
nil io․editsfo(string path) {}
/**
 * Lists a directory
 * @return A list of tables that represent a file/folder with these keys:
 * * **"name"**: boolean
 * * **"path"**: boolean
 * * **"isafolder"**: boolean
 * * **"created"**: string
 * * **"modified"**: string
 * * **"accessed"**: string
 * * **"size"**: number
 */
table io․list(string path) {}
/** @} */