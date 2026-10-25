/**
 * @defgroup draw draw
 * @brief Drawing shapes and text on the screen (uses vita2d)
 *
 * Everything drawn with this library (and with images, ImGui, etc.) is drawn to a hidden buffer first.
 * Nothing appears on the screen until you call drawꓸswapbuffers(), which shows the finished frame
 * and clears the buffer for the next one. Because of this, a LifeLua app is usually one big loop:
 *
 * @code
 * local white = color.new(255, 255, 255)
 * while true do
 *     draw.text(10, 10, "Hello World!", white)
 *     draw.rect(10, 40, 100, 50, color.new(255, 0, 0))
 *     draw.swapbuffers()
 * end
 * @endcode
 *
 * The screen is 960 × 544 pixels. (0, 0) is the top-left corner, x grows to the right and y grows downwards.
 *
 * Every parameter written as `color` accepts either a @ref color "color" value made with colorꓸnew()
 * or a plain number in `0xAABBGGRR` order (see tocolor()).
 * @{
*/
/**
 * Shows everything that was drawn since the last call on the screen, then clears the drawing buffer.
 * Call this once at the end of every frame of your main loop.
 * @param color The color the next frame starts with *(optional, default: black)*
*/
nil drawꓸswapbuffers(color color) {}
/**
 * Draws text on the screen
 *
 * After the 4 required parameters, you can choose the font and the size in any of these ways:
 * * `draw.text(x, y, text, color)`: system font (PGF), size 1
 * * `draw.text(x, y, text, color, size)`: system font (PGF) at `size`
 * * `draw.text(x, y, text, color, font)` or `draw.text(x, y, text, color, font, size)`: a font loaded with fontꓸload()
 * * `draw.text(x, y, text, color, size, font)`: same as above, with the size first
 * * `draw.text(x, y, text, color, true)` / `draw.text(x, y, text, color, true, size)`: system PVF font (`false` = system PGF font)
 * * `draw.text(x, y, text, color, size, true)`: same as above, with the size first
 *
 * @param x Left position of the text
 * @param y Top position of the text
 * @param text The text to draw
 * @param color Color of the text
 * @param size Text scale, 1 is the normal size *(optional, default: 1)*
 * @param font A font from fontꓸload(), or a boolean to pick the system PVF (`true`) or PGF (`false`) font *(optional)*
 * @par Example:
 * @code
 * local white = color.new(255, 255, 255)
 * local myfont = font.load("app0:font.ttf")
 * draw.text(10, 10, "System font", white)
 * draw.text(10, 40, "Twice as big", white, 2)
 * draw.text(10, 100, "Custom font", white, myfont)
 * @endcode
*/
nil drawꓸtext(number x, number y, string text, color color, number size, font font) {}
/**
 * Gets how wide a text will be when drawn with drawꓸtext()
 *
 * Accepts the same size/font combinations as drawꓸtext(), e.g. `draw.textwidth(text)`,
 * `draw.textwidth(text, size)`, `draw.textwidth(text, font)`, `draw.textwidth(text, font, size)`, `draw.textwidth(text, true)`, etc.
 * @param text The text to measure
 * @param size Text scale *(optional, default: 1)*
 * @param font A font from fontꓸload(), or `true`/`false` for the system PVF/PGF font *(optional)*
 * @return The width in pixels
 * @par Example:
 * @code
 * -- Center a text horizontally
 * local text = "Centered!"
 * draw.text(480 - draw.textwidth(text) / 2, 10, text, color.new(255, 255, 255))
 * @endcode
*/
number drawꓸtextwidth(string text, number size, font font) {}
/**
 * Gets how tall a text will be when drawn with drawꓸtext()
 *
 * Accepts the same size/font combinations as drawꓸtextwidth().
 * @param text The text to measure
 * @param size Text scale *(optional, default: 1)*
 * @param font A font from fontꓸload(), or `true`/`false` for the system PVF/PGF font *(optional)*
 * @return The height in pixels
*/
number drawꓸtextheight(string text, number size, font font) {}
/**
 * Draws a filled rectangle
 * @param x Left position
 * @param y Top position
 * @param width Width in pixels
 * @param height Height in pixels
 * @param color Fill color
 * @param outline Color of a 1 pixel border around the rectangle *(optional, no border if omitted)*
*/
nil drawꓸrect(number x, number y, number width, number height, color color, color outline) {}
/**
 * Draws a filled circle
 * @param x X position of the center
 * @param y Y position of the center
 * @param radius Radius in pixels
 * @param color Fill color
*/
nil drawꓸcircle(number x, number y, number radius, color color) {}
/**
 * Draws a filled circle with a gradient from its center to its edge
 * @param x X position of the center
 * @param y Y position of the center
 * @param radius Radius in pixels
 * @param center Color in the center
 * @param edge Color at the edge
 * @param smoothness Number of segments used for the edge, higher is smoother *(optional, default: 64, minimum: 3)*
*/
nil drawꓸradialcircle(number x, number y, number radius, color center, color edge, number smoothness) {}
/**
 * Draws a filled triangle.
 * If you give 3 colors, each corner gets its own color and the triangle is shaded between them.
 * @param x1 X of the 1st corner
 * @param y1 Y of the 1st corner
 * @param x2 X of the 2nd corner
 * @param y2 Y of the 2nd corner
 * @param x3 X of the 3rd corner
 * @param y3 Y of the 3rd corner
 * @param c1 Color of the 1st corner (or of the whole triangle)
 * @param c2 Color of the 2nd corner *(optional, default: c1)*
 * @param c3 Color of the 3rd corner *(optional, default: c1)*
*/
nil drawꓸtriangle(number x1, number y1, number x2, number y2, number x3, number y3, color c1, color c2, color c3) {}
/**
 * Draws a 1 pixel wide line
 * @param start_x X of the start point
 * @param start_y Y of the start point
 * @param end_x X of the end point
 * @param end_y Y of the end point
 * @param color Line color
*/
nil drawꓸline(number start_x, number start_y, number end_x, number end_y, color color) {}
/**
 * Draws a 1 pixel wide line that fades from one color to another
 * @param start_x X of the start point
 * @param start_y Y of the start point
 * @param end_x X of the end point
 * @param end_y Y of the end point
 * @param start_color Color at the start point
 * @param end_color Color at the end point
*/
nil drawꓸgradientline(number start_x, number start_y, number end_x, number end_y, color start_color, color end_color) {}
/**
 * Draws lines connecting several points, one after another.
 * Each point is given as 3 values: `x, y, color`. At least 2 points are required.
 * Each line fades between the colors of the 2 points it connects.
 * @param x1 X of the 1st point
 * @param y1 Y of the 1st point
 * @param color1 Color of the 1st point
 * @param x2 X of the 2nd point
 * @param y2 Y of the 2nd point
 * @param color2 Color of the 2nd point
 * @param ... More points, 3 values each
 * @par Example:
 * @code
 * -- A white-blue-green zigzag
 * draw.polyline(10, 10, white, 50, 10, blue, 30, 70, green)
 * @endcode
*/
nil drawꓸpolyline(number x1, number y1, color color1, number x2, number y2, color color2, ...) {}
/**
 * Draws a single pixel
 * @param x X position
 * @param y Y position
 * @param color Pixel color
*/
nil drawꓸpixel(number x, number y, color color) {}
/**
 * Draws a rectangle with a different color in each corner, shaded between them
 * @param x Left position
 * @param y Top position
 * @param width Width in pixels
 * @param height Height in pixels
 * @param top_left Top-left color
 * @param top_right Top-right color
 * @param bottom_left Bottom-left color
 * @param bottom_right Bottom-right color
 * @par Example:
 * @code
 * -- Horizontal gradient from red to blue
 * draw.gradientrect(0, 0, 960, 60, red, blue, red, blue)
 * @endcode
*/
nil drawꓸgradientrect(number x, number y, number width, number height, color top_left, color top_right, color bottom_left, color bottom_right) {}
/**
 * Draws a rectangle shaded in two halves stacked vertically:
 * the top half goes from the top colors to the center colors, the bottom half from the center colors to the bottom colors
 * @param x Left position
 * @param y Top position
 * @param width Width in pixels
 * @param height Height in pixels
 * @param top_left Top-left color
 * @param top_right Top-right color
 * @param center_left Middle-left color
 * @param center_right Middle-right color
 * @param bottom_left Bottom-left color
 * @param bottom_right Bottom-right color
*/
nil drawꓸvdoublegradientrect(number x, number y, number width, number height, color top_left, color top_right, color center_left, color center_right, color bottom_left, color bottom_right) {}
/**
 * Draws a rectangle shaded in two halves side by side:
 * the left half goes from the left colors to the center colors, the right half from the center colors to the right colors
 * @param x Left position
 * @param y Top position
 * @param width Width in pixels
 * @param height Height in pixels
 * @param left_top Top-left color
 * @param center_top Top-middle color
 * @param right_top Top-right color
 * @param left_bottom Bottom-left color
 * @param center_bottom Bottom-middle color
 * @param right_bottom Bottom-right color
*/
nil drawꓸhdoublegradientrect(number x, number y, number width, number height, color left_top, color center_top, color right_top, color left_bottom, color center_bottom, color right_bottom) {}
/**
 * Turns clipping on or off. While clipping is on, only the area set with drawꓸcliprect() or drawꓸclipcircle()
 * can be drawn on; everything outside of it is cut off.
 * @param enable `true` to turn clipping on, `false` to turn it off
*/
nil drawꓸenableclip(boolean enable) {}
/**
 * Sets a rectangular clipping area, useful for cutting off long text or making scrollable areas.
 * Only has an effect while clipping is turned on with drawꓸenableclip().
 * @param x1 Left edge of the area
 * @param y1 Top edge of the area
 * @param x2 Right edge of the area (this is a position, not a width)
 * @param y2 Bottom edge of the area (this is a position, not a height)
 * @par Example:
 * @code
 * draw.enableclip(true)
 * draw.cliprect(10, 10, 110, 30) -- a 100 × 20 area at (10, 10)
 * draw.text(10, 10, "This long text gets cut off at x = 110", white)
 * draw.enableclip(false)
 * @endcode
*/
nil drawꓸcliprect(number x1, number y1, number x2, number y2) {}
/**
 * Sets a circular clipping area. Works like drawꓸcliprect(), but the visible area is a circle.
 * @param x X of the circle's center
 * @param y Y of the circle's center
 * @param radius Radius of the circle
*/
nil drawꓸclipcircle(number x, number y, number radius) {}
/** @} */

/**
 * @defgroup color color
 * @brief Creating and changing colors
 *
 * A color is a value holding red, green, blue and alpha (opacity), each from 0 to 255.
 * Create one with colorꓸnew() and pass it to any function that draws something.
 *
 * Colors are never changed in place: functions like colorꓸr() or colorꓸadd() return a **new** color.
 *
 * You can also call these functions as methods, and use `+`, `-` and `==` on colors:
 * @code
 * local red = color.new(255, 0, 0)
 * local blue = color.new(0, 0, 255)
 * local purple = red + blue           -- same as color.add(red, blue)
 * local halfred = red:a(128)          -- same as color.a(red, 128)
 * print(red:r(), purple == red)       -- 255   false
 * @endcode
 * @{
*/
/**
 * Creates a new color. Values outside of 0-255 are clamped.
 * @param r Red, 0-255
 * @param g Green, 0-255
 * @param b Blue, 0-255
 * @param a Alpha (opacity), 0 is invisible and 255 is fully visible *(optional, default: 255)*
 * @return The new color
 * @par Example:
 * @code
 * local white = color.new(255, 255, 255)
 * local transparent_black = color.new(0, 0, 0, 128)
 * @endcode
*/
color colorꓸnew(number r, number g, number b, number a) {}
/**
 * Gets the red value of a color, or makes a copy of the color with a different red value
 *
 * Also available as a method: `c:r()` / `c:r(value)`
 * @param color The color
 * @param r New red value, 0-255 *(optional)*
 * @return The red value (0-255) if `r` is omitted, otherwise a new color with the changed red value
*/
number colorꓸr(color color, number r) {}
/**
 * Gets the green value of a color, or makes a copy of the color with a different green value
 *
 * Also available as a method: `c:g()` / `c:g(value)`
 * @param color The color
 * @param g New green value, 0-255 *(optional)*
 * @return The green value (0-255) if `g` is omitted, otherwise a new color with the changed green value
*/
number colorꓸg(color color, number g) {}
/**
 * Gets the blue value of a color, or makes a copy of the color with a different blue value
 *
 * Also available as a method: `c:b()` / `c:b(value)`
 * @param color The color
 * @param b New blue value, 0-255 *(optional)*
 * @return The blue value (0-255) if `b` is omitted, otherwise a new color with the changed blue value
*/
number colorꓸb(color color, number b) {}
/**
 * Gets the alpha (opacity) of a color, or makes a copy of the color with a different alpha
 *
 * Also available as a method: `c:a()` / `c:a(value)`
 * @param color The color
 * @param a New alpha value, 0-255 *(optional)*
 * @return The alpha value (0-255) if `a` is omitted, otherwise a new color with the changed alpha
 * @par Example:
 * @code
 * local white = color.new(255, 255, 255)
 * local faded = white:a(100) -- white stays the same, faded is a new color
 * @endcode
*/
number colorꓸa(color color, number a) {}
/**
 * Adds two colors together, channel by channel (results are capped at 255)
 *
 * Also available as `color1 + color2` and `color1:add(color2)`
 * @param color1 First color
 * @param color2 Second color
 * @return The new color
*/
color colorꓸadd(color color1, color color2) {}
/**
 * Subtracts the second color from the first, channel by channel (results are capped at 0)
 *
 * Also available as `color1 - color2` and `color1:sub(color2)`
 * @param color1 Color to subtract from
 * @param color2 Color to subtract
 * @return The new color
*/
color colorꓸsub(color color1, color color2) {}
/**
 * Blends two colors evenly (50% of each)
 *
 * Also available as a method: `color1:blend(color2)`
 * @param color1 First color
 * @param color2 Second color
 * @return The new color
*/
color colorꓸblend(color color1, color color2) {}
/**
 * Mixes two colors using a weight for each. The weights don't need to add up to 100,
 * they are relative to each other (`3, 1` gives 75% of color1 and 25% of color2).
 *
 * Also available as a method: `color1:mix(color2, p1, p2)`
 * @param color1 First color
 * @param color2 Second color
 * @param p1 Weight of the first color
 * @param p2 Weight of the second color
 * @return The new color
 * @par Example:
 * @code
 * local orange = color.mix(color.new(255, 0, 0), color.new(255, 255, 0), 50, 50)
 * @endcode
*/
color colorꓸmix(color color1, color color2, number p1, number p2) {}
/**
 * Converts a number to a color.
 * The number is read in `0xAABBGGRR` order: alpha, blue, green, red (the reverse of the usual HTML order).
 *
 * Any function that takes a color also accepts such a number directly.
 * @param color The color as a number
 * @return The new color
 * @par Example:
 * @code
 * local red  = tocolor(0xFF0000FF) -- alpha FF, blue 00, green 00, red FF
 * local blue = tocolor(0xFFFF0000) -- alpha FF, blue FF, green 00, red 00
 * draw.rect(0, 0, 100, 100, 0xFF00FF00) -- green, without converting first
 * @endcode
*/
color tocolor(number color) {}
/** @} */

/**
 * @defgroup font font
 * @brief Loading fonts for drawꓸtext()
 *
 * Without loading a font, drawꓸtext() uses the PS Vita's built-in system font.
 * Load a font with fontꓸload() and pass it to drawꓸtext(), drawꓸtextwidth() and drawꓸtextheight().
 * @{
*/
/**
 * Loads a font file. Supported types are the PS Vita's .PGF and .PVF fonts,
 * and anything FreeType can open, like .TTF, .OTF and .WOFF.
 * @param path Path to the font file (e.g. `"app0:font.ttf"`)
 * @return The loaded font. Raises an error if the file is missing or can't be loaded.
 * @par Example:
 * @code
 * local myfont = font.load("app0:font.ttf")
 * draw.text(10, 10, "Hello!", color.new(255, 255, 255), myfont, 1.5)
 * @endcode
*/
font fontꓸload(string path) {}
/** @} */

/**
 * @defgroup image image
 * @brief Loading, creating, drawing and saving images
 *
 * Every function that takes an `image` as its first parameter can also be called as a method:
 * @code
 * local img = image.load("app0:picture.png")
 * while true do
 *     img:display(0, 0)           -- same as image.display(img, 0, 0)
 *     draw.text(10, 10, img:width().." × "..img:height(), color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * @endcode
 *
 * Images are freed automatically when they are no longer used (garbage collected).
 *
 * All the rotating functions take the angle in **radians** (use `math.rad(degrees)` to convert)
 * and use the given position as the **center** of the image.
 * @{
*/
/** @name Texture filters
 * Used with imageꓸfilter(), imageꓸmin() and imageꓸmag()
 * @{ */
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_POINT; ///< Nearest pixel, keeps the image sharp/pixelated when scaled (good for pixel art)
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_LINEAR; ///< Smooths the image when scaled
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_MIPMAP_LINEAR; ///< Linear filtering with mipmaps
SceGxmTextureFilter SCE_GXM_TEXTURE_FILTER_MIPMAP_POINT; ///< Point filtering with mipmaps
/** @} */
/** @name QR code error correction levels
 * Used in the `error_correction` field of imageꓸqr(). Higher levels make the QR code bigger, but readable even if partly damaged.
 * @{ */
qrcodegen_Ecc qrcodegen_Ecc_LOW; ///< About 7% of the QR code can be damaged
qrcodegen_Ecc qrcodegen_Ecc_MEDIUM; ///< About 15% of the QR code can be damaged (default)
qrcodegen_Ecc qrcodegen_Ecc_QUARTILE; ///< About 25% of the QR code can be damaged
qrcodegen_Ecc qrcodegen_Ecc_HIGH; ///< About 30% of the QR code can be damaged
/** @} */
/** @name QR code masks
 * Used in the `mask` field of imageꓸqr(). Leave it at automatic unless you need a specific mask pattern.
 * @{ */
qrcodegen_Mask qrcodegen_Mask_AUTO; ///< Picks the best mask automatically (default)
qrcodegen_Mask qrcodegen_Mask_0; ///< Mask pattern 0
qrcodegen_Mask qrcodegen_Mask_1; ///< Mask pattern 1
qrcodegen_Mask qrcodegen_Mask_2; ///< Mask pattern 2
qrcodegen_Mask qrcodegen_Mask_3; ///< Mask pattern 3
qrcodegen_Mask qrcodegen_Mask_4; ///< Mask pattern 4
qrcodegen_Mask qrcodegen_Mask_5; ///< Mask pattern 5
qrcodegen_Mask qrcodegen_Mask_6; ///< Mask pattern 6
qrcodegen_Mask qrcodegen_Mask_7; ///< Mask pattern 7
/** @} */
/** @name QR code versions
 * Used in the `min_version` and `max_version` fields of imageꓸqr(). The version decides the size of the QR code.
 * @{ */
number qrcodegen_VERSION_MIN; ///< Smallest QR code version (1)
number qrcodegen_VERSION_MAX; ///< Biggest QR code version (40)
/** @} */
/**
 * Loads an image file.
 * Supported formats: .PNG, .JPG/.JPEG, .BMP, .TGA, .PSD, .HDR, .PIC, .PPM/.PGM and .SVG.
 * @param path Path to the image (e.g. `"app0:bg.png"`)
 * @return The image, or `nil` if the file doesn't exist or its format isn't supported
 * @par Example:
 * @code
 * local bg = image.load("app0:bg.png")
 * if not bg then error("Couldn't load bg.png") end
 * @endcode
*/
image imageꓸload(string path) {}
/**
 * Creates a new, empty image
 * @param width Width in pixels
 * @param height Height in pixels
 * @param color Color to fill the image with *(optional, the image is fully transparent if omitted)*
 * @return The new image
*/
image imageꓸnew(number width, number height, color color) {}
/**
 * Takes a screenshot of the screen as an image (960 × 544)
 * @return The new image
*/
image imageꓸscreen() {}
/**
 * Draws an image
 *
 * Method form: `img:display(x, y, tint)`
 * @param image The image
 * @param x Left position
 * @param y Top position
 * @param tint Color multiplied with the image, e.g. a half transparent white makes the image half transparent *(optional)*
*/
nil imageꓸdisplay(image image, number x, number y, color tint) {}
/**
 * Draws a scaled image
 *
 * Method form: `img:scaledisplay(x, y, scale_x, scale_y, tint)`
 * @param image The image
 * @param x Left position
 * @param y Top position
 * @param scale_x Horizontal scale, 1 is the original size, 2 is twice as wide, -1 flips the image
 * @param scale_y Vertical scale, 1 is the original size, 2 is twice as tall, -1 flips the image
 * @param tint Color multiplied with the image *(optional)*
*/
nil imageꓸscaledisplay(image image, number x, number y, number scale_x, number scale_y, color tint) {}
/**
 * Draws a rotated image
 *
 * Method form: `img:rotatedisplay(x, y, angle, tint)`
 * @param image The image
 * @param x X position of the image's center
 * @param y Y position of the image's center
 * @param angle Rotation in radians
 * @param tint Color multiplied with the image *(optional)*
*/
nil imageꓸrotatedisplay(image image, number x, number y, number angle, color tint) {}
/**
 * Draws a scaled and rotated image
 *
 * Method form: `img:scalerotatedisplay(x, y, scale_x, scale_y, angle, tint)`
 * @param image The image
 * @param x X position of the image's center
 * @param y Y position of the image's center
 * @param scale_x Horizontal scale, 1 is the original size
 * @param scale_y Vertical scale, 1 is the original size
 * @param angle Rotation in radians
 * @param tint Color multiplied with the image *(optional)*
*/
nil imageꓸscalerotatedisplay(image image, number x, number y, number scale_x, number scale_y, number angle, color tint) {}
/**
 * Draws only a part of an image, useful for sprite sheets
 *
 * Method form: `img:partdisplay(x, y, tex_x, tex_y, tex_w, tex_h, tint)`
 * @param image The image
 * @param x Left position on the screen
 * @param y Top position on the screen
 * @param tex_x Left of the part inside the image
 * @param tex_y Top of the part inside the image
 * @param tex_w Width of the part
 * @param tex_h Height of the part
 * @param tint Color multiplied with the image *(optional)*
 * @par Example:
 * @code
 * -- Draw the 3rd 32 × 32 frame of a sprite sheet
 * sheet:partdisplay(100, 100, 2 * 32, 0, 32, 32)
 * @endcode
*/
nil imageꓸpartdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, color tint) {}
/**
 * Draws a scaled part of an image
 *
 * Method form: `img:scalepartdisplay(x, y, tex_x, tex_y, tex_w, tex_h, scale_x, scale_y, tint)`
 * @param image The image
 * @param x Left position on the screen
 * @param y Top position on the screen
 * @param tex_x Left of the part inside the image
 * @param tex_y Top of the part inside the image
 * @param tex_w Width of the part
 * @param tex_h Height of the part
 * @param scale_x Horizontal scale, 1 is the original size
 * @param scale_y Vertical scale, 1 is the original size
 * @param tint Color multiplied with the image *(optional)*
*/
nil imageꓸscalepartdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, number scale_x, number scale_y, color tint) {}
/**
 * Draws a scaled and rotated part of an image
 *
 * Method form: `img:scalerotatepartdisplay(x, y, tex_x, tex_y, tex_w, tex_h, scale_x, scale_y, angle, tint)`
 * @param image The image
 * @param x X position of the part's center on the screen
 * @param y Y position of the part's center on the screen
 * @param tex_x Left of the part inside the image
 * @param tex_y Top of the part inside the image
 * @param tex_w Width of the part
 * @param tex_h Height of the part
 * @param scale_x Horizontal scale, 1 is the original size
 * @param scale_y Vertical scale, 1 is the original size
 * @param angle Rotation in radians
 * @param tint Color multiplied with the image *(optional)*
*/
nil imageꓸscalerotatepartdisplay(image image, number x, number y, number tex_x, number tex_y, number tex_w, number tex_h, number scale_x, number scale_y, number angle, color tint) {}
/**
 * Gets the width of an image
 *
 * Method form: `img:width()`
 * @param image The image
 * @return Width in pixels
*/
number imageꓸwidth(image image) {}
/**
 * Gets the height of an image
 *
 * Method form: `img:height()`
 * @param image The image
 * @return Height in pixels
*/
number imageꓸheight(image image) {}
/**
 * Gets or sets the color of one pixel of an image
 *
 * Method form: `img:pixel(x, y)` / `img:pixel(x, y, color)`
 * @param image The image
 * @param x X of the pixel, from 0 to width - 1
 * @param y Y of the pixel, from 0 to height - 1
 * @param color New color of the pixel *(optional)*
 * @return The pixel's color if `color` is omitted, otherwise nothing
 * @warning The position isn't checked, make sure it's inside the image.
 * @par Example:
 * @code
 * local img = image.new(64, 64, color.new(0, 0, 0))
 * img:pixel(10, 10, color.new(255, 0, 0)) -- set
 * print(img:pixel(10, 10):r())            -- get, prints 255
 * @endcode
*/
color imageꓸpixel(image image, number x, number y, color color) {}
/**
 * Turns an image into black & white. This changes the image itself.
 *
 * Method form: `img:grayscale()`
 * @param image The image
*/
nil imageꓸgrayscale(image image) {}
/**
 * Inverts the colors of an image (like a photo negative). This changes the image itself, the alpha stays the same.
 *
 * Method form: `img:invert()`
 * @param image The image
*/
nil imageꓸinvert(image image) {}
/**
 * Sets how an image is smoothed when it's drawn scaled
 *
 * Method form: `img:filter(min, mag)`
 * @param image The image
 * @param min Filter used when the image is drawn smaller, one of the texture filter constants
 * @param mag Filter used when the image is drawn bigger *(optional, default: same as min)*
 * @par Example:
 * @code
 * -- Keep pixel art sharp when scaling it up
 * sprite:filter(SCE_GXM_TEXTURE_FILTER_POINT)
 * sprite:scaledisplay(0, 0, 4, 4)
 * @endcode
*/
nil imageꓸfilter(image image, SceGxmTextureFilter min, SceGxmTextureFilter mag) {}
/**
 * Gets the filter used when the image is drawn smaller
 *
 * Method form: `img:min()`
 * @param image The image
 * @return One of the texture filter constants
*/
SceGxmTextureFilter imageꓸmin(image image) {}
/**
 * Gets the filter used when the image is drawn bigger
 *
 * Method form: `img:mag()`
 * @param image The image
 * @return One of the texture filter constants
*/
SceGxmTextureFilter imageꓸmag(image image) {}
/**
 * Saves an image to a file
 *
 * Method form: `img:save(path, type, quality)`
 * @param image The image
 * @param path Where to save it (e.g. `"ux0:data/picture.png"`)
 * @param type File type *(optional, default: `"png"`)*, one of:
 * * `"png"`
 * * `"jpg"` / `"jpeg"`
 * * `"bmp"`
 * * `"tga"`
 * * `"hdr"`
 * @param quality JPEG quality from 1 to 100, only used for `"jpg"`/`"jpeg"` *(optional, default: 100)*
 * @par Example:
 * @code
 * local shot = image.screen()
 * shot:save("ux0:data/screenshot.jpg", "jpg", 90)
 * @endcode
*/
nil imageꓸsave(image image, string path, string type, number quality) {}
/**
 * Looks for a QR code in an image and reads it
 *
 * Method form: `img:qrscan(index)`
 * @param image The image
 * @param index Which QR code to read if there are several, starting at 0 *(optional, default: 0)*
 * @return The text inside the QR code, or `nil` if no readable QR code was found
 * @see cameraꓸoutput() to get camera frames as images, for scanning QR codes live
*/
string imageꓸqrscan(image image, number index) {}
/**
 * Generates a QR code image
 * @param parameters A table with these fields (only `text` is required):
 * * **string** `text`: Text stored in the QR code
 * * **color** `fg_color`: Color of the dark squares *(default: black)*
 * * **color** `bg_color`: Background color *(default: white)*
 * * **number** `scale`: Size of each square in pixels *(default: 4)*
 * * **number** `border`: Empty space around the code, in squares *(default: 2)*
 * * **number** `error_correction`: One of the QR error correction constants *(default: qrcodegen_Ecc_MEDIUM)*
 * * **number** `min_version`, `max_version`: Allowed QR versions *(default: qrcodegen_VERSION_MIN to qrcodegen_VERSION_MAX)*
 * * **number** `mask`: One of the QR mask constants *(default: qrcodegen_Mask_AUTO)*
 * * **boolean** `boostEcl`: Use a higher error correction level if it fits in the same size *(default: true)*
 * @return The QR code image. Raises an error if the text is too long to fit.
 * @par Example:
 * @code
 * local qr = image.qr({text = "https://github.com/HarommelRabbid/LifeLua", scale = 6})
 * while true do
 *     qr:display(480 - qr:width() / 2, 272 - qr:height() / 2)
 *     draw.swapbuffers()
 * end
 * @endcode
*/
image imageꓸqr(table parameters) {}
/** @} */
