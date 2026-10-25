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
 * @param url The URL of the file
 * @param path Where to save the file
 * @param useragent The User-Agent sent to the server *(optional, default: a desktop Chrome User-Agent)*
 * @return `nil` if the connection or request failed
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
 * network.download("https://example.com/file.zip", "ux0:data/file.zip")
 * @endcode
*/
nil networkꓸdownload(string url, string path, string useragent) {}
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
