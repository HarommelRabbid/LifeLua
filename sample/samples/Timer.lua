local timer1 = timer.new()
timer1:start()
while true do
draw.text(10, 10, "Current time:", white)
draw.text(10, 30, timer1:elapsed().." seconds", white)
if timer1:isrunning() then
draw.text(10, 70, "Press Cross to pause", white)
else
draw.text(10, 70, "Press Cross to resume", white)
end
draw.text(10, 90, "Press Circle to reset", white)
controls.update()
if controls.released(SCE_CTRL_CROSS) then if timer1:isrunning() then timer1:pause() else timer1:resume() end end
if controls.released(SCE_CTRL_CIRCLE) then timer1:reset() end
if controls.released(SCE_CTRL_START) then timer1:stop() controls.update() break end
draw.swapbuffers()
end
