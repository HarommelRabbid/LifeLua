while true do
draw.circle(480, 544/2, 180, blue)
draw.line(480/2, 544/2, 480+180, 544/2+100, white)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
