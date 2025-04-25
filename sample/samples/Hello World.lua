while true do
draw.text(10, 10, "Hello World!", white)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
