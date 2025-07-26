/**
 * @file
 * @defgroup sqlite3 sqlite3
 * Method calling will work on most of the functions here (e. g. `sqlite3:query` & `sqlite3:close`)
 * @brief SQLite3 functions
 * @{
*/
number SQLITE_OPEN_READONLY;
number SQLITE_OPEN_READWRITE;
number SQLITE_OPEN_CREATE;
number SQLITE_OPEN_URI;
number SQLITE_OPEN_NOMUTEX;
number SQLITE_OPEN_FULLMUTEX;
/** 
 * Opens an SQLite3 database
*/
sqlite3 sqlite3․open(string path, number mode) {}
/** 
 * Executes a query in an open database
*/
table sqlite3․query(sqlite3 db, string query) {}
/** 
 * Closes an open database (also done automatically via garbage collecting)
*/
nil sqlite3․close(sqlite3 db) {}
/** @} */