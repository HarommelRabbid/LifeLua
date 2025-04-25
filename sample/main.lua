white = color.new(255, 255, 255)
blue = color.new(0, 0, 255)
local sel = 1
local samples = {"Hello World", "Controls"}
while true do
draw.rect(0, 0, 960, 65, blue)
draw.text(480-string.len("LifeLua Showcase Menu")*6, 20, "LifeLua Showcase Menu", white)
local y = 70
for i=1, #samples do
draw.rect(5, y, 480-5, 50, blue)
if i==sel then
draw.rect(5, y, 480-5, 50, color.new(255, 255, 255, 255/2))
end
draw.text(480/2-50, y+10, samples[i], white)
y = y+55
end
controls.update()
if controls.pressed(SCE_CTRL_UP) and sel > 1 then sel = sel - 1 end
if controls.pressed(SCE_CTRL_DOWN) and sel < #samples then sel = sel + 1 end
if controls.released(SCE_CTRL_CROSS) then controls.update() dofile("samples/"..samples[sel]..".lua") end
if controls.released(SCE_CTRL_START) then os.exit() end
draw.swapbuffers()
end
