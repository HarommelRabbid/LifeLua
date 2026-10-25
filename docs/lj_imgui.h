/**
 * @defgroup imgui imgui
 * @brief Windows, menus, buttons and other UI widgets (uses Dear ImGui)
 *
 * ImGui is an "immediate mode" UI library: there are no button objects to create or keep.
 * Instead, every frame you call the widget functions in the order they should appear, and each one tells you
 * right away if the user interacted with it (e.g. imguiꓸbutton() returns `true` on the frame it's pressed).
 *
 * Call imguiꓸinit() once at the start, then put your widgets between imguiꓸrenderinit() and imguiꓸrenderterm()
 * in your main loop:
 * @code
 * imgui.init()
 * imgui.darktheme()
 * imgui.touch(true)   -- use the touch screen
 * imgui.gamepad(true) -- use the buttons/D-pad to move between widgets
 *
 * local sound = true
 * local level = 1
 * local levels = {"Easy", "Normal", "Hard"}
 *
 * while true do
 *     imgui.renderinit()
 *
 *     imgui.nextwindowposition(20, 20, ImGuiCond_FirstUseEver)
 *     imgui.nextwindowsize(400, 250, ImGuiCond_FirstUseEver)
 *     imgui.windowbegin("Settings")
 *     imgui.text("Hello from ImGui!")
 *     sound = imgui.checkbox("Sound", sound)
 *     level = imgui.combobox("Difficulty", level, levels)
 *     if imgui.button("Start") then
 *         -- start the game with levels[level]
 *     end
 *     imgui.windowend()
 *
 *     imgui.renderterm()
 *     draw.swapbuffers()
 * end
 * @endcode
 *
 * You can draw other things (with @ref draw "draw", @ref image "image", ...) in the same frame; draw them before
 * imguiꓸrenderterm() to have the ImGui windows on top of them.
 *
 * @note The text of imguiꓸtext(), imguiꓸdisabledtext(), imguiꓸwrappedtext() and imguiꓸtooltip() is read as a
 * C format string, so write `%%` to show a `%` sign.
 * @{
*/
/** @name Setup
 * @{ */
/**
 * Starts ImGui. Call it once before using any other ImGui function.
*/
nil imguiꓸinit() {}
/**
 * Stops ImGui and frees its memory
*/
nil imguiꓸshutdown() {}
/**
 * Chooses if the touch screens can be used to interact with ImGui
 * @param front `true` to use the front touch screen
 * @param rear `true` to also use the rear touch pad *(optional, default: false)*
 * @param indirect `true` to use the front touch screen like a laptop touchpad (moving a cursor), instead of touching widgets directly *(optional, default: false)*
*/
nil imguiꓸtouch(boolean front, boolean rear, boolean indirect) {}
/**
 * Chooses if the buttons can be used to move between and use widgets (D-pad to move, the accept button to press)
 * @param enable `true` to use the buttons
*/
nil imguiꓸgamepad(boolean enable) {}
/**
 * Chooses if the left analog stick moves a mouse cursor
 * @param enable `true` to use the left stick as a mouse
*/
nil imguiꓸcursor(boolean enable) {}
/**
 * Chooses if ImGui draws the mouse cursor itself
 * @param enable `true` to draw the cursor
*/
nil imguiꓸdrawcursor(boolean enable) {}
/**
 * Changes the shape of the mouse cursor for this frame
 * @param cursor One of the mouse cursor constants
*/
nil imguiꓸsetcursor(ImGuiMouseCursor cursor) {}
/**
 * Switches to the dark color theme
*/
nil imguiꓸdarktheme() {}
/**
 * Switches to the light color theme
*/
nil imguiꓸlighttheme() {}
/**
 * Switches to the classic color theme
*/
nil imguiꓸclassictheme() {}
/**
 * Gets the version of Dear ImGui
 * @return The version, e.g. `"1.79 WIP"`
*/
string imguiꓸversion() {}
/** @} */

/** @name Frame
 * @{ */
/**
 * Starts a new ImGui frame. Call it at the start of every frame, before any widget.
*/
nil imguiꓸrenderinit() {}
/**
 * Ends the ImGui frame and draws all the widgets. Call it after your last widget, before drawꓸswapbuffers().
*/
nil imguiꓸrenderterm() {}
/**
 * Shows the ImGui metrics/debug window, with information about the windows and how fast they draw
*/
nil imguiꓸmetrics() {}
/** @} */

/** @name Windows
 * @{ */
/**
 * Sets the position of the next window created with imguiꓸwindowbegin()
 * @param x Left position
 * @param y Top position
 * @param condition When to apply it, one of the condition constants *(optional, default: ImGuiCond_Always)*
*/
nil imguiꓸnextwindowposition(number x, number y, ImGuiCond condition) {}
/**
 * Sets the size of the next window created with imguiꓸwindowbegin()
 * @param width Width in pixels
 * @param height Height in pixels
 * @param condition When to apply it, one of the condition constants *(optional, default: ImGuiCond_Always)*
*/
nil imguiꓸnextwindowsize(number width, number height, ImGuiCond condition) {}
/**
 * Starts a window. The widgets called after it are placed in this window, until imguiꓸwindowend().
 * @param title The window title, it must be unique
 * @param flags Window flag constants, added together *(optional, default: ImGuiWindowFlags_None)*
 * @par Example:
 * @code
 * imgui.windowbegin("Info", ImGuiWindowFlags_NoMove + ImGuiWindowFlags_NoResize)
 * imgui.text("This window can't be moved or resized")
 * imgui.windowend()
 * @endcode
*/
nil imguiꓸwindowbegin(string title, ImGuiWindowFlags flags) {}
/**
 * Ends the window started with imguiꓸwindowbegin()
*/
nil imguiꓸwindowend() {}
/**
 * Gives focus to a window (brings it to the front)
 * @param title Title of the window *(optional, default: the current window)*
*/
nil imguiꓸwindowfocus(string title) {}
/**
 * Sets how rounded the corners of the windows are. Applies to the windows started after calling it.
 * @param rounding Corner radius in pixels *(optional, default: 0)*
*/
nil imguiꓸwindowrounding(number rounding) {}
/** @} */

/** @name Main menu bar & menus
 * @{ */
/**
 * Starts the menu bar at the top of the screen
 * @return `true` if the menu bar is shown. Only then add menus and call imguiꓸmenubarend().
 * @par Example:
 * @code
 * if imgui.menubarbegin() then
 *     if imgui.menubegin("File") then
 *         if imgui.menuitem("Exit") then os.exit() end
 *         imgui.menuend()
 *     end
 *     imgui.menubarend()
 * end
 * @endcode
*/
boolean imguiꓸmenubarbegin() {}
/**
 * Ends the menu bar. Only call it if imguiꓸmenubarbegin() returned `true`.
*/
nil imguiꓸmenubarend() {}
/**
 * Starts a menu (or a sub-menu, inside another menu)
 * @param label The menu's name
 * @param enabled `false` to grey it out *(optional, default: true)*
 * @return `true` if the menu is open. Only then add items and call imguiꓸmenuend().
*/
boolean imguiꓸmenubegin(string label, boolean enabled) {}
/**
 * Ends a menu. Only call it if imguiꓸmenubegin() returned `true`.
*/
nil imguiꓸmenuend() {}
/**
 * Adds an item to the open menu
 * @param label The item's text
 * @param selected `true` to show a check mark next to it *(optional, default: false)*
 * @param enabled `false` to grey it out *(optional, default: true)*
 * @return `true` on the frame it's chosen
*/
boolean imguiꓸmenuitem(string label, boolean selected, boolean enabled) {}
/** @} */

/** @name Text
 * @{ */
/**
 * Shows a line of text
 * @param text The text
*/
nil imguiꓸtext(string text) {}
/**
 * Shows greyed-out text
 * @param text The text
*/
nil imguiꓸdisabledtext(string text) {}
/**
 * Shows text that wraps onto the next line at the edge of the window
 * @param text The text
*/
nil imguiꓸwrappedtext(string text) {}
/**
 * Gets the size a text will have when shown with ImGui
 * @param text The text
 * @return A table with the fields `x` (width) and `y` (height)
*/
table imguiꓸtextsize(string text) {}
/**
 * Shows a tooltip when the last widget is hovered by the cursor
 * @param text The tooltip text
 * @par Example:
 * @code
 * imgui.button("Save")
 * imgui.tooltip("Saves your progress")
 * @endcode
*/
nil imguiꓸtooltip(string text) {}
/** @} */

/** @name Widgets
 * @{ */
/**
 * Shows a button
 * @param label The button's text
 * @param width Width in pixels, 0 to fit the text *(optional, default: 0)*
 * @param height Height in pixels, 0 to fit the text *(optional, default: 0)*
 * @return `true` on the frame it's pressed
*/
boolean imguiꓸbutton(string label, number width, number height) {}
/**
 * Shows a small button, with less padding, that fits in a line of text
 * @param label The button's text
 * @return `true` on the frame it's pressed
*/
boolean imguiꓸsmallbutton(string label) {}
/**
 * Shows a checkbox
 * @param label The checkbox's text
 * @param checked Whether it's checked now
 * @return Whether it's checked after this frame. Store it and pass it again next frame.
 * @par Example:
 * @code
 * music_on = imgui.checkbox("Music", music_on)
 * @endcode
*/
boolean imguiꓸcheckbox(string label, boolean checked) {}
/**
 * Shows a radio button (one choice out of several)
 * @param label The radio button's text
 * @param active Whether it's the selected choice
 * @return `true` on the frame it's pressed
 * @par Example:
 * @code
 * if imgui.radiobutton("Small", size == 1) then size = 1 end
 * imgui.sameline()
 * if imgui.radiobutton("Big", size == 2) then size = 2 end
 * @endcode
*/
boolean imguiꓸradiobutton(string label, boolean active) {}
/**
 * Shows a drop-down list
 * @param label The text next to it
 * @param selected Index (starting at 1) of the selected item
 * @param items A list of strings to choose from
 * @return The index of the selected item after this frame. Store it and pass it again next frame.
*/
number imguiꓸcombobox(string label, number selected, table items) {}
/**
 * Shows a list box (a list where all the items are visible)
 * @param label The text next to it
 * @param selected Index (starting at 1) of the selected item
 * @param items A list of strings to choose from
 * @return The index of the selected item after this frame. Store it and pass it again next frame.
*/
number imguiꓸlistbox(string label, number selected, table items) {}
/**
 * Shows a progress bar
 * @param percent Progress from 0 to 100 *(optional, default: 0)*
 * @param width Width in pixels, 0 for automatic *(optional, default: 0)*
 * @param height Height in pixels, 0 for automatic *(optional, default: 0)*
*/
nil imguiꓸprogressbar(number percent, number width, number height) {}
/** @} */

/** @name Layout & style
 * @{ */
/**
 * Puts the next widget on the same line as the previous one, instead of below it
*/
nil imguiꓸsameline() {}
/**
 * Draws a horizontal line to separate widgets
*/
nil imguiꓸseparator() {}
/**
 * Gets or sets where the next widget is placed in the current window
 * @param x New X position inside the window *(optional)*
 * @param y New Y position inside the window *(optional)*
 * @return If `x` and `y` are omitted: a table with the fields `x` and `y`
*/
table imguiꓸcursorposition(number x, number y) {}
/**
 * Sets the width of the next widgets (like combo boxes and progress bars) until imguiꓸresetitemwidth()
 * @param width Width in pixels
*/
nil imguiꓸitemwidth(number width) {}
/**
 * Goes back to the widget width used before the last imguiꓸitemwidth()
*/
nil imguiꓸresetitemwidth() {}
/**
 * Sets where the text is placed inside the buttons shown after calling it
 * @param x Horizontal alignment from 0 (left) to 1 (right), 0.5 is centered *(optional, default: 0)*
 * @param y Vertical alignment from 0 (top) to 1 (bottom), 0.5 is centered *(optional, default: 0)*
*/
nil imguiꓸbuttontextalign(number x, number y) {}
/** @} */

/** @name Window flags
 * Used with imguiꓸwindowbegin(). Add them together to combine them.
 * @{ */
ImGuiWindowFlags ImGuiWindowFlags_None; ///< Default window
ImGuiWindowFlags ImGuiWindowFlags_NoTitleBar; ///< No title bar
ImGuiWindowFlags ImGuiWindowFlags_NoResize; ///< Can't be resized by the user
ImGuiWindowFlags ImGuiWindowFlags_NoMove; ///< Can't be moved by the user
ImGuiWindowFlags ImGuiWindowFlags_NoScrollbar; ///< No scrollbars (it can still scroll)
ImGuiWindowFlags ImGuiWindowFlags_NoScrollWithMouse; ///< Can't be scrolled with the mouse wheel
ImGuiWindowFlags ImGuiWindowFlags_NoCollapse; ///< Can't be collapsed
ImGuiWindowFlags ImGuiWindowFlags_NoBackground; ///< No background or border
ImGuiWindowFlags ImGuiWindowFlags_NoSavedSettings; ///< Don't remember its position/size
ImGuiWindowFlags ImGuiWindowFlags_NoMouseInputs; ///< Ignores the mouse/touch (they go through it)
ImGuiWindowFlags ImGuiWindowFlags_MenuBar; ///< Has a menu bar
ImGuiWindowFlags ImGuiWindowFlags_HorizontalScrollbar; ///< Allow a horizontal scrollbar
ImGuiWindowFlags ImGuiWindowFlags_NoFocusOnAppearing; ///< Don't take focus when it appears
ImGuiWindowFlags ImGuiWindowFlags_NoBringToFrontOnFocus; ///< Don't come to the front when it gets focus
ImGuiWindowFlags ImGuiWindowFlags_AlwaysVerticalScrollbar; ///< Always show the vertical scrollbar
ImGuiWindowFlags ImGuiWindowFlags_AlwaysUseWindowPadding; ///< Always use the window padding, even for child windows without a border
ImGuiWindowFlags ImGuiWindowFlags_NoNav; ///< Can't be navigated with the buttons
ImGuiWindowFlags ImGuiWindowFlags_NoDecoration; ///< Same as NoTitleBar + NoResize + NoScrollbar + NoCollapse
ImGuiWindowFlags ImGuiWindowFlags_NoInputs; ///< Ignores all input (same as NoMouseInputs + NoNav)
ImGuiWindowFlags ImGuiWindowFlags_UnsavedDocument; ///< Shows a `*` after the title
ImGuiWindowFlags ImGuiWindowFlags_ChildWindow; ///< Used internally by ImGui, don't use
ImGuiWindowFlags ImGuiWindowFlags_Tooltip; ///< Used internally by ImGui, don't use
ImGuiWindowFlags ImGuiWindowFlags_Popup; ///< Used internally by ImGui, don't use
ImGuiWindowFlags ImGuiWindowFlags_Modal; ///< Used internally by ImGui, don't use
ImGuiWindowFlags ImGuiWindowFlags_ChildMenu; ///< Used internally by ImGui, don't use
/** @} */
/** @name Conditions
 * Used with imguiꓸnextwindowposition() and imguiꓸnextwindowsize()
 * @{ */
ImGuiCond ImGuiCond_None; ///< Same as ImGuiCond_Always
ImGuiCond ImGuiCond_Always; ///< Apply it every time (the user can't move/resize the window)
ImGuiCond ImGuiCond_Once; ///< Apply it only the first time it's called
ImGuiCond ImGuiCond_FirstUseEver; ///< Apply it only if the window has no saved position/size yet
ImGuiCond ImGuiCond_Appearing; ///< Apply it every time the window appears after being hidden
/** @} */
/** @name Mouse cursors
 * Used with imguiꓸsetcursor()
 * @{ */
ImGuiMouseCursor ImGuiMouseCursor_None; ///< No cursor
ImGuiMouseCursor ImGuiMouseCursor_Arrow; ///< Normal arrow
ImGuiMouseCursor ImGuiMouseCursor_TextInput; ///< Text cursor
ImGuiMouseCursor ImGuiMouseCursor_ResizeAll; ///< Move in all directions
ImGuiMouseCursor ImGuiMouseCursor_ResizeNS; ///< Resize up/down
ImGuiMouseCursor ImGuiMouseCursor_ResizeEW; ///< Resize left/right
ImGuiMouseCursor ImGuiMouseCursor_ResizeNESW; ///< Resize diagonally (bottom-left/top-right)
ImGuiMouseCursor ImGuiMouseCursor_ResizeNWSE; ///< Resize diagonally (top-left/bottom-right)
ImGuiMouseCursor ImGuiMouseCursor_Hand; ///< Pointing hand
ImGuiMouseCursor ImGuiMouseCursor_NotAllowed; ///< "Not allowed" sign
ImGuiMouseCursor ImGuiMouseCursor_COUNT; ///< Number of cursor types (not a cursor)
/** @} */
/** @} */
