/**
 * @file
 * @defgroup draw draw
 * Uses vita2d
 * @brief Drawing library
 * @{
*/
/** 
 * Swaps the buffers. Needed for the drawing functions above it to show.
 * @param color Optional color, default will be black
*/
nil draw․swapbuffers(color color) {}
/** 
 * Draws text on the screen
 * @param size: The size argument can be also a font, if so, then the size will be the next argument
*/
nil draw․text(number x, number y, string text, color color, number size, font font) {}
/** 
 * Gets the text width as displayed on the screen
 * @param size: The size argument can be also a font, if so, then the size will be the next argument
*/
number draw․textwidth(string text, number size, font font) {}
/** 
 * Gets the text height as displayed on the screen
 * @param size: The size argument can be also a font, if so, then the size will be the next argument
*/
number draw․textheight(string text, number size, font font) {}
/** 
 * Draws a rectangle on the screen
*/
nil draw․rect(number x, number y, number width, number height, color color, color outline) {}
/** 
 * Draws a circle on the screen
*/
nil draw․circle(number x, number y, number radius, color color) {}
/** 
 * Draws a line on the screen
*/
nil draw․line(number start_x, number start_y, number endx, number endy, color color) {}
/** 
 * Draws a pixel on the screen
*/
nil draw․pixel(number x, number y, color color) {}
/** 
 * Draws a gradient rectangle on the screen
*/
nil draw․gradientrect(number x, number y, number width, number height, color top_left, color top_right, color bottom_left, color bottom_right) {}
/** 
 * Draws a double vertical gradient rectangle on the screen
*/
nil draw․vdoublegradientrect(number x, number y, number width, number height, color top, color center, color bottom) {}
/** 
 * Draws a double horizontal gradient rectangle on the screen
*/
nil draw․hdoublegradientrect(number x, number y, number width, number height, color top, color center, color bottom) {}
/** 
 * Enables or disables clipping
*/
nil draw․enableclip(boolean enable) {}
/** 
 * Clips a rectangle, useful for clipping text
 * @par Example:
 * @code
 * draw.enableclip(true)
 * draw.clip(0, 0, 25, 10)
 * draw.text(0, 0 "This is some text", ...)
 * draw.enableclip(false)
 * @endcode
*/
nil draw․cliprect(number x, number y, number width, number height) {}
/** @} */

/**
 * @file
 * @defgroup color color
 * @brief Color library
 * @{
*/
/** 
 * Creates a new color
 * @param a: Alpha is optional, will default to 255 (maximum)
*/
color color․new(number r, number g, number b, number a) {}
/** @} */

/**
 * @file
 * @defgroup font font
 * @brief Font library
 * @{
*/
/** 
 * Loads a font, can be a .PGF, .PVF, .TTF or a .WOFF
*/
font font․load(string path) {}
/** @} */

/**
 * @file
 * @defgroup image image
 * @brief Image library
 * @{
*/
/** 
 * Loads an image, can be a .PNG, .BMP, or a .JPG/.JPEG
*/
image image․load(string path) {}
/** 
 * Displays an image to the screen
 * @param tint optional
*/
nil image․display(image image, number x, number y, color tint) {}
/** 
 * Displays a scaled image to the screen
 * @param tint optional
*/
nil image․scaledisplay(image image, number x, number y, number scale_x, number scale_y, color tint) {}
/** 
 * Gets width of an image
*/
number image․width(image image) {}
/** 
 * Gets height of an image
*/
number image․height(image image) {}
/** @} */