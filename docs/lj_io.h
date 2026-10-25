/**
 * @defgroup io io
 * @brief Files, folders, archives & hashes (extends Lua's io library)
 *
 * These functions are added to Lua's standard `io` library, so `io.open()`, `io.read()`, etc. still work
 * like in normal Lua (see the [Lua manual](https://www.lua.org/manual/5.1/manual.html#5.7)).
 *
 * Paths on the PS Vita start with a partition, for example:
 * * `app0:` is your app's own folder, where `main.lua` is
 * * `ux0:data/` is the usual place to save your app's files
 * * `ux0:`, `ur0:`, `uma0:`, `imc0:`, ... are the memory card and other storage
 *
 * @code
 * if not io.exists("ux0:data/myapp") then
 *     io.newfolder("ux0:data/myapp")
 * end
 * local file = io.open("ux0:data/myapp/save.txt", "w")
 * file:write("Hello!")
 * file:close()
 * @endcode
 * @{
*/
/**
 * Checks if a file or folder exists
 * @param path Path to check
 * @return `true` if it exists
*/
boolean ioꓸexists(string path) {}
/**
 * Creates a folder, including any missing parent folders. Does nothing if it already exists.
 * @param path Path of the new folder (e.g. `"ux0:data/myapp/saves"`)
*/
nil ioꓸnewfolder(string path) {}
/**
 * Deletes a file or an **empty** folder
 * @param path Path to delete
 * @return `true` on success, `false` if it doesn't exist or couldn't be deleted (e.g. the folder isn't empty)
*/
boolean ioꓸdelete(string path) {}
/**
 * Lists the contents of a folder
 * @param path Path of the folder
 * @return `nil` if the folder can't be opened, otherwise a list with one table per file/folder, with these fields:
 * * **string** `name`: File or folder name
 * * **string** `path`: Full path
 * * **boolean** `isafolder`: `true` for folders
 * * **number** `size`: Size in bytes
 * * **string** `created`, `modified`, `accessed`: Dates in the `"YYYY-MM-DD HH:MM:SS"` format
 * @par Example:
 * @code
 * for i, item in ipairs(io.list("ux0:data")) do
 *     print(item.name, item.isafolder and "folder" or item.size.." bytes")
 * end
 * @endcode
*/
table ioꓸlist(string path) {}
/**
 * Gets information about a file or a folder
 * @param path Path of the file or folder
 * @return `nil` if it doesn't exist, otherwise a table with these fields:
 * * **boolean** `isafolder`: `true` for folders
 * * **number** `size`: Size in bytes
 * * **string** `created`, `modified`, `accessed`: Dates in the `"YYYY-MM-DD HH:MM:SS"` format
*/
table ioꓸinfo(string path) {}
/**
 * Gets the last part of a path (the file or folder name)
 * @param path A path
 * @return The name, e.g. `"save.txt"` for `"ux0:data/myapp/save.txt"`
*/
string ioꓸpathstrip(string path) {}
/**
 * Removes the last part of a path (gets the folder that contains it)
 * @param path A path
 * @return The parent folder, e.g. `"ux0:data/myapp"` for `"ux0:data/myapp/save.txt"`
*/
string ioꓸfilestrip(string path) {}
/**
 * Gets or sets the working directory, which relative paths (like `"main.lua"`) start from
 * @param path New working directory *(optional)*
 * @return The current working directory if `path` is omitted. Raises an error if the directory can't be set.
*/
string ioꓸworkpath(string path) {}
/**
 * Gets the free space of a partition
 * @param partition Partition name (e.g. `"ux0:"`)
 * @return Free space in bytes
 * @par Example:
 * @code
 * print(string.format("%.2f GB free", io.freespace("ux0:") / 1024^3))
 * @endcode
*/
number ioꓸfreespace(string partition) {}
/**
 * Gets the total size of a partition
 * @param partition Partition name (e.g. `"ux0:"`)
 * @return Total size in bytes
*/
number ioꓸfullspace(string partition) {}
/**
 * Reads a .SFO file (like an app's `sce_sys/param.sfo`)
 * @param path Path to the .SFO
 * @return A table with every key of the .SFO, with string or number values. Raises an error if the file is invalid.
 * @par Example:
 * @code
 * local sfo = io.readsfo("app0:sce_sys/param.sfo")
 * print(sfo.TITLE, sfo.TITLE_ID)
 * @endcode
*/
table ioꓸreadsfo(string path) {}
/**
 * Changes one value in a .SFO file. The key must already exist in the file.
 * @param path Path to the .SFO
 * @param key Key to change (e.g. `"TITLE"`)
 * @param value New value: a string for text keys, a number for number keys
*/
nil ioꓸeditsfo(string path, string key, string value) {}
/**
 * Extracts an archive (.ZIP, .7Z, .RAR, .TAR, .VPK, ...). Missing folders are created.
 *
 * If you define a global function called `LifeLuaArchiveExtracting`, it's called for each file before it's extracted.
 * You can use it to draw a progress screen.
 * @param archive Path to the archive
 * @param destination Folder to extract into
 * @par Example:
 * @code
 * function LifeLuaArchiveExtracting(name, index, size)
 *     draw.text(10, 10, "Extracting file #"..index..": "..name, color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * io.extract("ux0:data/game.zip", "ux0:data/game")
 * @endcode
*/
nil ioꓸextract(string archive, string destination) {}
/**
 * Creates an archive from a folder, or from a list of files
 * @param contents Either a folder path (everything inside it is added),
 * or a table where each key is a file path and each value is the name it gets inside the archive
 * @param archive_path Path of the archive to create
 * @param format Archive format *(optional, default: `"zip"`)*. One of `"zip"`, `"7z"`, `"gnutar"`, `"ustar"`, `"v7tar"`,
 * `"xar"`, `"iso9660"`, `"ar_bsd"`, `"ar_svr4"`, `"warc"` or `"raw"`
 * @return `true` when done
 * @par Example:
 * @code
 * io.archive("ux0:data/myapp", "ux0:data/myapp_backup.zip")
 * io.archive({["ux0:data/myapp/save.txt"] = "save.txt"}, "ux0:data/save.7z", "7z")
 * @endcode
*/
boolean ioꓸarchive(string contents, string archive_path, string format) {}
/**
 * Calculates the CRC32 checksum of a text
 * @param text The text
 * @return The checksum as a number
*/
number ioꓸcrc32(string text) {}
/**
 * Calculates the MD5 hash of a string
 * @param text The string (can contain any bytes)
 * @return The hash as uppercase hexadecimal text
*/
string ioꓸmd5(string text) {}
/**
 * Calculates the SHA-1 hash of a text
 * @param text The text
 * @return The hash as uppercase hexadecimal text
 * @par Example:
 * @code
 * print(io.sha1("hello")) -- AAF4C61DDCC5E8A2DABEDE0F3B482CD9AEA9434D
 * @endcode
*/
string ioꓸsha1(string text) {}
/**
 * Calculates the SHA-256 hash of a text
 * @param text The text
 * @return The hash as uppercase hexadecimal text
*/
string ioꓸsha256(string text) {}
/** @} */
