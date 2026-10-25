/**
 * @defgroup json json
 * @brief Reading and writing JSON (uses cJSON)
 *
 * JSON objects become Lua tables with string keys, JSON arrays become Lua lists (starting at index 1),
 * and the other way around.
 *
 * @code
 * local data = json.decode('{"name": "LifeLua", "tags": ["lua", "vita"], "stars": 5}')
 * print(data.name, data.tags[1], data.stars) -- LifeLua   lua   5
 *
 * data.stars = data.stars + 1
 * local text = json.encode(data)
 * @endcode
 *
 * JSON's `null` can't be stored as Lua's `nil` (a table can't hold `nil`), so it's represented by a special value instead.
 * Use jsonꓸisnull() to check for it, and json.null to write it.
 * @{
*/
jsonnull jsonꓸnull; ///< The value used for JSON `null`. Put it in a table to write `null`, e.g. `json.encode({score = json.null})` gives `{"score": null}`
/**
 * Converts a JSON string to a Lua value
 *
 * jsonꓸparse() is the same function under another name.
 * @param json The JSON text
 * @return The resulting table (or string/number/boolean, if the JSON is just one value). Raises an error if the JSON is invalid.
*/
table jsonꓸdecode(string json) {}
/**
 * Same as jsonꓸdecode()
 * @param json The JSON text
 * @return The resulting table
*/
table jsonꓸparse(string json) {}
/**
 * Converts a Lua value to a JSON string.
 * A table whose keys are all numbers becomes a JSON array, any other table becomes a JSON object
 * (keys that aren't strings are skipped). Functions and other unsupported values become `null`.
 *
 * jsonꓸstringify() is the same function under another name.
 * @param value The table (or other value) to convert
 * @param minified `true` to leave out the spaces and line breaks *(optional, default: false, the JSON is nicely formatted)*
 * @return The JSON text
*/
string jsonꓸencode(table value, boolean minified) {}
/**
 * Same as jsonꓸencode()
 * @param value The table (or other value) to convert
 * @param minified `true` to leave out the spaces and line breaks *(optional, default: false)*
 * @return The JSON text
*/
string jsonꓸstringify(table value, boolean minified) {}
/**
 * Removes all spaces, line breaks and comments from a JSON string
 * @param json The JSON text
 * @return The minified JSON text
*/
string jsonꓸminify(string json) {}
/**
 * Checks if a value is JSON's `null`
 * @param value Any value
 * @return `true` if the value is JSON `null`
 * @par Example:
 * @code
 * local data = json.decode('{"score": null}')
 * if json.isnull(data.score) then print("No score yet") end
 * @endcode
*/
boolean jsonꓸisnull(any value) {}
/**
 * Gets the version of the cJSON library
 * @return The version, e.g. `"1.7.19"`
*/
string jsonꓸversion() {}
/** @} */

/**
 * @defgroup toml toml
 * @brief Reading TOML files (useful for settings/config files)
 * @{
*/
/**
 * Reads a TOML file into a Lua table.
 * Dates and times become strings in the ISO 8601 format (e.g. `"2025-08-21"` or `"2025-08-21T10:30:00.000000"`).
 * @param path Path to the .TOML file
 * @return A table with the contents of the file. Raises an error if the file is missing or invalid.
 * @par Example:
 * @code
 * -- settings.toml:
 * -- [player]
 * -- name = "Harommel"
 * -- volume = 80
 * local settings = toml.parse("app0:settings.toml")
 * print(settings.player.name, settings.player.volume)
 * @endcode
*/
table tomlꓸparse(string path) {}
/** @} */
