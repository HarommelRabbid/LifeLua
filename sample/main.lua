dofile("scroll.lua")
psexchar = font.load("sa0:data/font/pvf/psexchar.pvf")
deff = font.load("app0:font.ttf")
white = color.new(255, 255, 255)
blue = color.new(0, 0, 255)
bg = image.load("app0:llbg.png")
icons = {}
local sel = 1
local samples = {"Hello World", "Controls", ".SFO Reader", "Message", "Keyboard", "3 Button Message", "Progress Message", "Battery", "Image", "File Manager", "App Launcher", "Timer", "FTP", "Network", "SHA1 & CRC32 Hashing"}
local scroll = newScroll(samples, 8)
local bar = false
--os.screenshot(false)
--os.screenshotinfo("Hello from LifeLua!", "Hello", "Comment")
--os.screenshotoverlay("app0:/screenshot_overlay.png")
--os.videoexport("ux0:data/Backup/Top Gear Parody 2.mp4")
local tick = 5
function controls.rapidfireheld(button, time)
if (controls.held(button) and tick <= time) then tick = tick + 1 end
if tick >= time or controls.pressed(button) then tick = 0 return true end
end
while true do
bg:display(0, 0)
draw.gradientrect(0, 0, 960, 60, white, blue, blue, white)
draw.text(480-draw.textwidth("LifeLua Showcase Menu", deff)/2, 20, "LifeLua Showcase Menu", white, deff)
local y = 65
for i=scroll.ini, scroll.lim do
draw.hdoublegradientrect(5, y, 480-5, 50, white, blue, white, white, blue, white)
--draw.hdoublegradientrect(480+5, y, 480-10, 50, white, blue, white, white, blue, white)
if i==scroll.sel then
draw.hdoublegradientrect(5, y, 480-5, 50, color.new(0, 0, 255, 255/2), color.new(0, 0, 0, 0), color.new(0, 0, 255, 255/2), color.new(0, 0, 255, 255/2), color.new(0, 0, 0, 0), color.new(0, 0, 255, 255/2))
if selpressed then
draw.rect(5, y, 480-5, 50, color.new(255, 255, 255, 0), white)
end
end
draw.enableclip(true)
draw.cliprect(5, y, 480, y+50)
draw.text(480/2-draw.textwidth(samples[i], deff)/2, y+15, samples[i], white, deff)
draw.enableclip(false)
y = y + 55
end
draw.rect(0, 544-39, 960, 39, blue)
draw.text(10, 544-39+10, "#", white, psexchar)
draw.text(10+draw.textwidth("#", psexchar), 544-39+10, " Notification", white, deff)
draw.text(10+draw.textwidth("#", psexchar)+draw.textwidth(" Notification", deff)+10, 544-39+10, "4", white, psexchar)
draw.text(10+draw.textwidth("#", psexchar)+draw.textwidth(" Notification", deff)+draw.textwidth("4", psexchar)+10, 544-39+10, " Infobar", white, deff)
controls.update()
if controls.rapidfireheld(SCE_CTRL_UP, 8) then scroll:up() end
if controls.rapidfireheld(SCE_CTRL_DOWN, 8) then scroll:down() end
selpressed = controls.held(SCE_CTRL_CROSS)
if controls.released(SCE_CTRL_CROSS) then controls.update() draw.swapbuffers() dofile("samples/"..samples[scroll.sel]..".lua") end
if controls.released(SCE_CTRL_START) then os.exit() end
if controls.released(SCE_CTRL_SELECT) then if not bar then os.infobar(INFOBAR_VISIBILITY_VISIBLE, INFOBAR_COLOR_WHITE, INFOBAR_TRANSPARENCY_TRANSLUCENT) bar = true else os.infobar() bar = false end end
if controls.released(SCE_CTRL_SQUARE) then os.notification("Hello from LifeLua!") end
draw.swapbuffers()
end
