function os.message(message, mode)
local msg = true
controls.update()
if msg then
draw.rect(480-280, 100, 480+280/3, 350, blue, white)
draw.text(480-string.len(message)*6, 350/2, message, white)
if mode == "ok" then
draw.text(480-string.len("X: OK")*6, 450-40, "X: OK", white)
if controls.released(SCE_CTRL_CROSS) then msg = false return true end
elseif mode == "yesno" then
draw.text(480-string.len("X: Yes")*6, 450-60, "X: Yes", white)
draw.text(480-string.len("O: No")*6, 450-40, "O: No", white)
if controls.released(SCE_CTRL_CROSS) then msg = false return true end
if controls.released(SCE_CTRL_CIRCLE) then msg = false return false end
end
end
controls.update()
end
