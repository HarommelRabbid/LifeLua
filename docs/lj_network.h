/**
 * @defgroup network network
 * @brief Wi-Fi, downloads & FTP server
 *
 * @code
 * if network.wifi() then
 *     network.download("https://example.com/file.zip", "ux0:data/file.zip")
 * end
 * @endcode
 * @{
*/
/**
 * Checks if the PS Vita is connected to Wi-Fi
 * @return `true` if it's connected (or connecting)
*/
boolean networkꓸwifi() {}
/**
 * Gets the PS Vita's IP address
 * @return The IP address, e.g. `"192.168.1.20"`, or `"127.0.0.1"` if not connected
*/
string networkꓸip() {}
/**
 * Gets the PS Vita's MAC address
 * @return The MAC address, e.g. `"00:11:22:AA:BB:CC"`
*/
string networkꓸmac() {}
/** @name HTTP methods
 * Used with networkꓸdownload() and networkꓸget() to choose how the request is sent.
 * @{ */
number SCE_HTTP_METHOD_GET; ///< Get a page or file (the default)
number SCE_HTTP_METHOD_POST; ///< Send data to the server, e.g. a form or JSON
number SCE_HTTP_METHOD_HEAD; ///< Like GET, but the server only sends the headers, no content
number SCE_HTTP_METHOD_OPTIONS; ///< Ask the server which methods it supports
number SCE_HTTP_METHOD_PUT; ///< Upload data to replace what's at the URL
number SCE_HTTP_METHOD_DELETE; ///< Delete what's at the URL
number SCE_HTTP_METHOD_TRACE; ///< Ask the server to send the request back (for debugging)
number SCE_HTTP_METHOD_CONNECT; ///< Open a tunnel through a proxy
/** @} */
/**
 * Downloads a file (HTTP or HTTPS). The app waits until the download is finished.
 *
 * If you define a global function called `LifeLuaNetworkDownload`, it's called every time a piece of the file
 * is downloaded, so you can show the progress. It receives these 5 numbers (the names you give them don't matter):
 * 1. Total bytes downloaded so far
 * 2. Total bytes written to the file so far
 * 3. Size of the whole file in bytes (0 if the server doesn't say)
 * 4. Bytes downloaded in this piece
 * 5. Bytes written in this piece
 *
 * If the callback raises an error, the download stops and the error is passed on.
 * @param url The URL of the file
 * @param path Where to save the file
 * @param method The @ref SCE_HTTP_METHOD_GET "HTTP method" *(optional, default: SCE_HTTP_METHOD_GET)*
 * @param data Data to send with the request, e.g. for SCE_HTTP_METHOD_POST *(optional, default: nothing is sent)*
 * @param contenttype The type of `data` *(optional, default: `"application/x-www-form-urlencoded"`)*. Only used if `data` is given.
 * @param useragent The User-Agent sent to the server *(optional, default: a desktop Chrome User-Agent)*
 * @return The HTTP status code (e.g. `200` for OK, `404` for not found), or `nil` if the connection, the request
 * or writing the file failed
 * @par Example:
 * @code
 * local white, blue = color.new(255, 255, 255), color.new(0, 0, 255)
 * function LifeLuaNetworkDownload(downloaded, written, size)
 *     if size > 0 then
 *         draw.text(10, 10, math.floor(written * 100 / size).."% done", white)
 *         draw.rect(0, 505, written * 960 / size, 39, blue)
 *     end
 *     draw.swapbuffers()
 * end
 * local status = network.download("https://example.com/file.zip", "ux0:data/file.zip")
 * if status ~= 200 then print("Download failed:", status) end
 * @endcode
*/
number networkꓸdownload(string url, string path, number method, string data, string contenttype, string useragent) {}
/**
 * Downloads a page (HTTP or HTTPS) and returns its content as a string, instead of saving it to a file.
 * The app waits until it's finished.
 *
 * The `LifeLuaNetworkDownload` callback is called here too (see networkꓸdownload()).
 * Since nothing is written to a file, its "written" numbers are the same as the "downloaded" ones.
 * @param url The URL of the page
 * @param method The @ref SCE_HTTP_METHOD_GET "HTTP method" *(optional, default: SCE_HTTP_METHOD_GET)*
 * @param data Data to send with the request, e.g. for SCE_HTTP_METHOD_POST *(optional, default: nothing is sent)*
 * @param contenttype The type of `data` *(optional, default: `"application/x-www-form-urlencoded"`)*. Only used if `data` is given.
 * @param useragent The User-Agent sent to the server *(optional, default: a desktop Chrome User-Agent)*
 * @return Two values: the content of the page, and the HTTP status code (e.g. `200`). `nil` if the connection or the request failed.
 * @par Example:
 * @code
 * local body, status = network.get("https://api.github.com/repos/HarommelRabbid/LifeLua")
 * if status == 200 then
 *     print(json.decode(body).stargazers_count)
 * end
 *
 * -- Sending JSON with POST
 * local reply = network.get("https://example.com/api/score", SCE_HTTP_METHOD_POST,
 *     json.encode({name = "Harommel", points = 100}), "application/json")
 * @endcode
*/
string networkꓸget(string url, number method, string data, string contenttype, string useragent) {}
/**
 * Gets the HTTP response headers of a URL
 * @param url The URL
 * @param useragent The User-Agent sent to the server *(optional, default: a desktop Chrome User-Agent)*
 * @return All the headers as one string, or `nil` if the request failed
*/
string networkꓸheader(string url, string useragent) {}
/**
 * Starts or stops an FTP server on the PS Vita, so you can access its files from a computer.
 * @param enable `true` to start, `false` to stop
 * @return When starting: a table with the fields `ip` (string) and `port` (number) to connect to.
 * Raises an error if the server is already in the requested state.
 * @par Example:
 * @code
 * local ftp = network.ftp.start(true)
 * network.ftp.adddevice("ux0:")
 * while true do
 *     draw.text(10, 10, "Connect to "..ftp.ip..":"..ftp.port, color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * @endcode
*/
table networkꓸftpꓸstart(boolean enable) {}
/**
 * Checks if the FTP server is running
 * @return `true` if it's running
*/
boolean networkꓸftpꓸisrunning() {}
/**
 * Makes a partition visible on the running FTP server
 * @param partition Partition name (e.g. `"ux0:"`)
*/
nil networkꓸftpꓸadddevice(string partition) {}
/**
 * Hides a partition from the running FTP server
 * @param partition Partition name (e.g. `"ux0:"`)
*/
nil networkꓸftpꓸremovedevice(string partition) {}
/** @} */
