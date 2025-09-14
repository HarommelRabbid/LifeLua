/**
 * @defgroup network network
 * @brief Network library
 * @{
*/
/**
 * Initializes FTP
 * @return A table with the enabled IP and port
*/
table network․ftp(boolean enable) {}
/**
 * Adds a partition to the running FTP server
*/
nil network․ftpadddevice(string device) {}
/**
 * Removes a partition from the running FTP server
*/
nil network․ftpremovedevice(string device) {}
/**
 * Checks if FTP is enabled or not
*/
boolean network․ftpinit() {}
/**
 * Checks if the WiFi is enabled on the Vita or not
*/
boolean network․wifi() {}
/**
 * Gets the Vita's IP address
*/
string network․ip() {}
/**
 * Gets the Vita's MAC address
*/
string network․mac() {}
/**
 * Downloads a file to a specified path
 * @remark This function can call `LifeLuaNetworkDownload` if it exists. Arguments' names don't matter. @par Example:
 * ```
function LifeLuaNetworkDownload(read, wrote, size, speed)
draw.text(10, 10, read, white)
draw.text(10, 30, math.floor((wrote*100)/size).."% completed", white)
draw.text(10, 50, size.." B", white)
draw.gradientrect(0, 544-39, (wrote*960)/size, 39, white, blue, white, blue)
draw.swapbuffers()
end
network.download(url, path)
 * ```
*/
nil network․download(string url, string path) {}
/**
 * Gets a URL's header
*/
string network․header(string url) {}
/** @} */