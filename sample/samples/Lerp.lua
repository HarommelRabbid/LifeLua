local x = 960
local opened = false
while true do
draw.rect(x, 0, 960, 544, blue)
draw.text(x+10, 10, "Test", white)
draw.swapbuffers()
controls.update()
if controls.pressed(SCE_CTRL_CROSS) then if not opened then opened = true else opened = false end end
if opened then x = x + (960-200-x) * 0.2
else x = x + (960+1-x) * 0.2 end
end
