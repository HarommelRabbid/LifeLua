imgui.init()
imgui.darktheme()
imgui.touch(true)   -- touch the widgets directly
imgui.gamepad(true) -- or move between them with the D-pad

local white = color.new(255, 255, 255)
local theme = 1
local sound = true
local level = 2
local levels = {"Easy", "Normal", "Hard"}
local progress = 0

while true do
    imgui.renderinit()

    -- The menu bar at the top of the screen
    if imgui.menubarbegin() then
        if imgui.menubegin("App") then
            if imgui.menubegin("Theme") then
                if imgui.menuitem("Dark theme", theme == 1) then imgui.darktheme() theme = 1 end
                if imgui.menuitem("Light theme", theme == 2) then imgui.lighttheme() theme = 2 end
                if imgui.menuitem("Classic theme", theme == 3) then imgui.classictheme() theme = 3 end
                imgui.menuend()
            end
            if imgui.menuitem("Exit") then imgui.shutdown() os.exit() end
            imgui.menuend()
        end
        imgui.menubarend()
    end

    -- You can draw anything that isn't ImGui too
    draw.text(10, 520, "This is an ImGui test in LifeLua", white)

    -- A window with some widgets
    imgui.nextwindowposition(20, 40, ImGuiCond_FirstUseEver)
    imgui.nextwindowsize(420, 260, ImGuiCond_FirstUseEver)
    imgui.windowbegin("Settings")

    imgui.text("Change the settings below")
    sound = imgui.checkbox("Sound", sound)
    level = imgui.combobox("Difficulty", level, levels)
    imgui.separator()

    if imgui.button("Load", 100) then progress = 0 end
    imgui.tooltip("Starts the progress bar again")
    imgui.sameline()
    if imgui.smallbutton("Done") then progress = 100 end

    if progress < 100 then progress = progress + 0.5 end
    imgui.progressbar(progress)

    imgui.windowend()

    imgui.renderterm()
    draw.swapbuffers()
end
