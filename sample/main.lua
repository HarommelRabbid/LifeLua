white = color.new(255, 255, 255)
blue = color.new(0, 0, 255)
local sel = 1
local samples = {"Hello World", "Controls", ".SFO Reader", "Message", "Keyboard", "3 Button Message", "Battery"}
local bar = false
os.screenshotinfo("Hello from LifeLua!", "Hello", "Comment")
while true do
draw.rect(0, 0, 960, 60, blue)
draw.text(480-string.len("LifeLua Showcase Menu")*6, 20, "LifeLua Showcase Menu", white)
local y = 65
for i=1, #samples do
draw.rect(5, y, 480-5, 50, blue)
if i==sel then
draw.rect(5, y, 480-5, 50, color.new(255, 255, 255, 255/2))
if selpressed then 
draw.rect(5, y, 480-5, 50, color.new(255, 255, 255, 0), white)
end
end
draw.text(480/2-50, y+15, samples[i], white)
y = y+55
end
controls.update()
if controls.pressed(SCE_CTRL_UP) and sel > 1 then sel = sel - 1 end
if controls.pressed(SCE_CTRL_DOWN) and sel < #samples then sel = sel + 1 end
selpressed = controls.held(SCE_CTRL_CROSS)
if controls.released(SCE_CTRL_CROSS) then controls.update() dofile("samples/"..samples[sel]..".lua") end
if controls.released(SCE_CTRL_START) then os.exit() end
if controls.released(SCE_CTRL_SELECT) then if not bar then os.infobar(INFOBAR_VISIBILITY_VISIBLE, INFOBAR_COLOR_WHITE, INFOBAR_TRANSPARENCY_TRANSLUCENT) bar = true else os.infobar() bar = false end end
draw.swapbuffers()
end
