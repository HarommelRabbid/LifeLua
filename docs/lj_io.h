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
 * Deletes a file, or a folder together with everything inside it
 *
 * If you define a global function called `LifeLuaDeleting`, it's called right before each file or folder is deleted,
 * so you can show the progress. A folder's contents are deleted before the folder itself. It receives:
 * 1. **string**: Path of the file or folder about to be deleted
 * 2. **boolean**: `true` if it's a folder
 * 3. **number**: How many files and folders have been handled so far, counting this one
 *
 * If the callback raises an error, deleting stops (what was already deleted stays deleted) and the error is passed on.
 * @param path Path of the file or folder to delete
 * @return `true` on success, `false` if it doesn't exist or something couldn't be deleted
 * (deleting stops at the first thing that fails)
 * @warning Folders are deleted with everything in them, without asking. Double-check the path!
 * @par Example:
 * @code
 * function LifeLuaDeleting(path, isafolder, number)
 *     draw.text(10, 10, "Deleting #"..number..": "..path, color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * io.delete("ux0:data/myapp/cache")
 * @endcode
*/
boolean ioꓸdelete(string path) {}
/**
 * Copies a file, or a folder together with everything inside it.
 * Missing parent folders of the destination are created, and existing files are overwritten.
 *
 * If you define a global function called `LifeLuaCopying`, it's called while copying, so you can show the progress:
 * once when each folder is created, once when each file starts, and after every piece (64 KB) of a file is copied.
 * It receives:
 * 1. **string**: Path of the file or folder being copied
 * 2. **string**: Path of its copy
 * 3. **number**: Bytes of this file copied so far (0 for folders)
 * 4. **number**: Size of this file in bytes (0 for folders)
 * 5. **number**: How many files and folders have been handled so far, counting this one
 *
 * If the callback raises an error, copying stops (what was already copied stays) and the error is passed on.
 * @param source Path of the file or folder to copy
 * @param destination Full path of the copy, **not** the folder to put it in:
 * `io.copy("ux0:data/a.txt", "ux0:data/backup/a.txt")`, not `io.copy("ux0:data/a.txt", "ux0:data/backup")`
 * @return `true` on success, `false` if the source doesn't exist or something couldn't be copied
 * (copying stops at the first thing that fails). Raises an error if you try to copy a folder into itself.
 * @par Example:
 * @code
 * local white = color.new(255, 255, 255)
 * function LifeLuaCopying(source, destination, copied, size, number)
 *     draw.text(10, 10, "Copying #"..number..": "..source, white)
 *     if size > 0 then draw.text(10, 40, math.floor(copied * 100 / size).."%", white) end
 *     draw.swapbuffers()
 * end
 * io.copy("ux0:data/myapp", "ux0:data/myapp_backup")
 * @endcode
*/
boolean ioꓸcopy(string source, string destination) {}
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
