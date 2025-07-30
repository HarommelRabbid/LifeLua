/**
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
 * Rectangle scrissor, useful for clipping text
 * @par Example:
 * @code
 * draw.enableclip(true)
 * draw.clip(0, 0, 25, 10)
 * draw.text(0, 0 "This is some text", ...)
 * draw.enableclip(false)
 * @endcode
*/
nil draw․cliprect(number x, number y, number width, number height) {}
/** 
 * Circle scrissor
*/
nil draw․clipcircle(number x, number y, number radius) {}
/** 
 * Draws a triangle.
 * @param c2 optional
 * @param c3 optional
*/
nil draw․triangle(number x, number y, number x2, number y2, number x3, number y3, color c1, color c2, color c3) {}
/** @} */

/**
<<<<<<< HEAD
 * @file
=======
>>>>>>> 3308273 (Initial commit)
 * @defgroup color color
 * Method calling will work on most of the functions here (e. g. `color:a`, `color:sub`, etc.)
 * @brief Color library
 * @{
*/
/** 
 * Creates a new color
 * @param a: Alpha is optional, will default to 255 (maximum)
*/
color color․new(number r, number g, number b, number a) {}
/** 
 * Gets or sets the red in a color
 * @param r: Optional, new value for the red in the color
*/
number color․r(color color, number r) {}
/** 
 * Gets or sets the green in a color
 * @param g: Optional, new value for the green in the color
*/
number color․g(color color, number g) {}
/** 
 * Gets or sets the blue in a color
 * @param b: Optional, new value for the blue in the color
*/
number color․b(color color, number b) {}
/** 
 * Gets or sets the alpha in a color
 * @param a: Optional, new value for the alpha in the color
*/
number color․a(color color, number a) {}
/** 
 * Sum 2 colors
*/
color color․add(color color1, color color2) {}
/** 
 * Subtract 2 colors
*/
color color․sub(color color1, color color2) {}
/** 
 * Blends 2 colors
*/
color color․blend(color color1, color color2) {}
/** 
 * Blends 2 colors by 2 percentages
*/
color color․mix(color color1, color color2, number p1, number p2) {}
/** 
 * Converts an integer (e. g. 0xFFFF0000) to a color
*/
color tocolor(number color) {}
/** @} */

/**
<<<<<<< HEAD
 * @file
=======
>>>>>>> 3308273 (Initial commit)
 * @defgroup font font
 * @brief Font library
 * @{
*/
/** 
 * Loads a font, can be a .PGF, .PVF, .TTF or a .WOFF
*/
font font․load(string path) {}
/** 
 * Sets a font to be the default one
*/
font font․default(font font) {}
/** @} */

/**
<<<<<<< HEAD
 * @file
=======
>>>>>>> 3308273 (Initial commit)
 * @defgroup image image
 * Method calling will work on most of the functions here (e. g. `image:display`, `image:width`, etc.)
 * @brief Image library
 * @{
*/
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_POINT;
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_LINEAR;
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_MIPMAP_LINEAR;
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_MIPMAP_POINT;
/** 
 * Loads an image, can be a .PNG, .BMP, .JPG/.JPEG, .TGA, .PSD, .PIC, .PPM/.PGM & .HDR
*/
image image․load(string path) {}
/** 
 * Creates a new image
 * @param color Optional, will default to white
*/
image image․new(number w, number h, color color) {}
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
 * Displays a rotated image to the screen
 * @param tint optional
*/
nil image․rotatedisplay(image image, number x, number y, number radius, color tint) {}
/** 
 * Displays a rotated & scaled image to the screen
 * @param tint optional
*/
nil image․scalerotatedisplay(image image, number x, number y, number scale_x, number scale_y, number radius, color tint) {}
/** 
 * Displays a part of an image to the screen
 * @param tint optional
*/
nil image․partdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, color tint) {}
/** 
 * Displays a part of a scaled image to the screen
 * @param tint optional
*/
nil image․scalepartdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, number scale_x, number scale_y, color tint) {}
/** 
 * Displays a part of a scaled & rotated image to the screen
 * @param tint optional
*/
nil image․scalerotatepartdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, number scale_x, number scale_y, number radius, color tint) {}
/** 
 * Displays a part of a rotated image to the screen
 * @param tint optional
*/
nil image․rotatepartdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, number radius, color tint) {}
/** 
 * Gets width of an image
*/
number image․width(image image) {}
/** 
 * Gets height of an image
*/
number image․height(image image) {}
/** 
 * Scans for QR codes in an image
*/
string image․qrscan(image image) {}
/** 
 * Generates a QR code
 * @param parameters:
 * * **string** text: Text content of the QR code
 * * **color** bg_color: optional, color of the QR's background
 * * **color** fg_color: optional, color of the QR's foreground
 * * **number** border: optional, border amount
*/
image image․qr(table parameters) {}
/** 
 * Gets screen buffer as an image
*/
image image․screen(image image) {}
/** 
 * Saves an image
 * @param type: available types:
 * * **"png"**
 * * **"jpeg"/"jpg"**
 * * **"bmp"**
 * * **"tga"**
 * * **"hdr"**
*/
nil image․save(image image, string path, string type) {}
/** 
 * Add filters to an image
*/
nil image․filter(image image, SceGxmTextureFilter min, SceGxmTextureFilter mag) {}
/** @} */