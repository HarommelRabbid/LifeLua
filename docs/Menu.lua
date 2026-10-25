-- A simple menu: move with Up/Down, choose with Cross

local white = color.new(255, 255, 255)
local grey = color.new(120, 120, 120)
local highlight = color.new(0, 90, 200)

local options = {"Start game", "Settings", "Exit"}
local selected = 1

while true do
    controls.update()

    -- controls.pressed() is only true on the frame the button goes down,
    -- so holding Down doesn't scroll through the whole menu at once
    if controls.pressed(SCE_CTRL_DOWN) then
        selected = selected + 1
        if selected > #options then selected = 1 end
    elseif controls.pressed(SCE_CTRL_UP) then
        selected = selected - 1
        if selected < 1 then selected = #options end
    end

    if controls.pressed(SCE_CTRL_CROSS) then
        if options[selected] == "Exit" then
            os.exit()
        else
            os.message("You chose: "..options[selected])
        end
    end

    draw.text(40, 40, "My App", white, 2)
    for i, option in ipairs(options) do
        local y = 120 + (i - 1) * 50
        if i == selected then
            draw.rect(30, y - 8, 300, 40, highlight)
            draw.text(40, y, option, white)
        else
            draw.text(40, y, option, grey)
        end
    end

    draw.swapbuffers()
end
