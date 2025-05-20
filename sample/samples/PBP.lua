function message(text)
local msg = true
while msg do
draw.rect(480/2, 544/4, 480, 544/2, blue, white)
draw.text((480/2)+10, (544/4)+10, text, white)
draw.text((480/2)+10, 544/1.4-10, "Cross: OK", white)
controls.update()
if controls.released(SCE_CTRL_CROSS) then controls.update() msg = false return true end
draw.swapbuffers()
end
end
message("test")
