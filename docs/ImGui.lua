imgui.init()
imgui.darktheme()
theme = 1 -- for checking only
--imgui.cursor(false)
imgui.touch(true)
imgui.gamepad(true)

while true do
    imgui.renderinit()
    --imgui.setcursor(ImGuiMouseCursor_None)

    if imgui.menubarbegin() then
        if imgui.menubegin("test") then
            if imgui.menubegin("Theme") then
                if imgui.menuitem("Dark theme", theme == 1) then imgui.darktheme() theme = 1 end
                if imgui.menuitem("Light theme", theme == 2) then imgui.lighttheme() theme = 2 end
                if imgui.menuitem("Classic theme", theme == 3) then imgui.classictheme() theme = 3 end
                imgui.menuend()
            end
            if imgui.menuitem("Error") then imgui.shutdown() error() end
            if imgui.menuitem("Exit") then imgui.shutdown() os.exit() end
            imgui.menuend()
        end
        imgui.menubarend()
    end

    draw.text(10, 30, "This is an ImGui test in LifeLua", color.new(255, 255, 255)) -- you can draw anything non-ImGui related

    imgui.text("Test")
    imgui.button("Button", 100)
    imgui.smallbutton("Small button")
    if imgui.menubegin("test1") then
        imgui.menuitem("test")
        imgui.menuend()
    end

    imgui.renderterm()
    draw.swapbuffers()
end