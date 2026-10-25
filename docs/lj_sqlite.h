/**
 * @defgroup sqlite3 sqlite3
 * @brief SQLite databases
 *
 * Every function that takes a database as its first parameter can also be called as a method,
 * e.g. `db:query(sql)` and `db:close()`. Databases are also closed automatically when they're no longer used.
 *
 * @code
 * local db = sqlite3.open("ux0:data/myapp/scores.db")
 * db:query("CREATE TABLE IF NOT EXISTS scores (name TEXT, points INTEGER)")
 * db:query("INSERT INTO scores VALUES ('Harommel', 1200)")
 * for i, row in ipairs(db:query("SELECT * FROM scores ORDER BY points DESC")) do
 *     print(i, row.name, row.points)
 * end
 * db:close()
 * @endcode
 * @{
*/
/** @name Open modes
 * Used with sqlite3ꓸopen(). Combine several of them by adding them together.
 * @{ */
number SQLITE_OPEN_READONLY; ///< Open the database for reading only
number SQLITE_OPEN_READWRITE; ///< Open the database for reading and writing
number SQLITE_OPEN_CREATE; ///< Create the database if it doesn't exist (use together with SQLITE_OPEN_READWRITE)
number SQLITE_OPEN_URI; ///< Allow the path to be a `file:` URI
number SQLITE_OPEN_NOMUTEX; ///< Multi-thread mode
number SQLITE_OPEN_FULLMUTEX; ///< Serialized mode (safe to use from several threads)
/** @} */
/**
 * Opens a database file
 * @param path Path to the database file
 * @param mode How to open it, one or more of the open mode constants added together *(optional, default: `SQLITE_OPEN_READWRITE + SQLITE_OPEN_CREATE`)*
 * @return The database. Raises an error if it can't be opened.
*/
sqlite3 sqlite3ꓸopen(string path, number mode) {}
/**
 * Runs one or more SQL statements
 *
 * Method form: `db:query(sql)`
 * @param db The database
 * @param sql The SQL to run
 * @return A list of the result rows (empty if there are none). Each row is a table with the column names as keys.
 * All values are returned as strings (use `tonumber()` for numbers), and SQL `NULL` values are `nil`.
 * Raises an error if the SQL fails.
*/
table sqlite3ꓸquery(sqlite3 db, string sql) {}
/**
 * Closes a database
 *
 * Method form: `db:close()`
 * @param db The database
*/
nil sqlite3ꓸclose(sqlite3 db) {}
/** @} */
