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
 * Gets the SHA1 hash of a string
 */
string io․sha1(string string) {}
/**
 * Gets the MD5 hash of a string
 */
string io․md5(string string) {}
/**
 * Gets or sets the working path
 * @param path If defined, the work path will be set to the defined path
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
 * Deletes a file or a folder
 */
nil io․delete(string path) {}
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
 * * **"name"**: string
 * * **"path"**: string
 * * **"isafolder"**: boolean
 * * **"created"**: string
 * * **"modified"**: string
 * * **"accessed"**: string
 * * **"size"**: number
 */
table io․list(string path) {}
/**
 * Strips the last file/directory from a path
 */
string io․filestrip(string path) {}
/**
 * Strips the files/directories from a path besides the last one
 */
string io․pathstrip(string path) {}
/**
 * Gets the free space of a partition
 */
number io․freespace(string partition) {}
/**
 * Gets the size of a partition
 */
number io․totalspace(string partition) {}
/** @} */