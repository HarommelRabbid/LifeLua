/**
 * @defgroup json json
 * @brief JSON library
 * @{
*/
/// JSON null value
jsonnull json․null;
/**
 * Parses a JSON string to a table
 */
table json․decode(string json) {}
/**
 * Parses a table to a JSON string
 */
string json․encode(table table) {}
/**
 * Minifies a JSON string, removes any pretty formatting.
 */
string json․minify(string json) {}
/**
 * Checks if a value is null in JSON (nil in Lua and null in JSON aren't the same!)
 */
boolean json․isnull(jsonnull value) {}
/**
 * @return cJSON version
 */
string json․version(string json) {}
/** @} */
/**
 * @defgroup toml toml
 * @brief TOML library
 * @{
*/
/**
 * Parses a TOML file to a table
 */
table toml․parse(string path) {}
/** @} */