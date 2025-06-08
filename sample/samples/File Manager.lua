dofile("scroll.lua")
local files = io.list("ux0:")
local filescroll = newScroll(files, 24)
while true do
draw.text(10, 10, io.filestrip(files[filescroll.sel].path):gsub("/", "", 1), white)
local y = 50
for i = filescroll.ini, filescroll.lim do
if files[i].isafolder then
if i == filescroll.sel then
if draw.textwidth(files[i].name.."/") > 320-50 then
draw.enableclip(true)
draw.cliprect(10, y, 10+320-5, y+draw.textheight(files[i].name.."/")+10)
draw.text(10, y, files[i].name.."/", color.new(0, 0, 255))
draw.enableclip(false)
else
draw.text(10, y, files[i].name.."/", color.new(0, 0, 255))
end
draw.text(10+300+20, y, files[i].created, color.new(0, 0, 255))
draw.text(10+300+20+draw.textwidth(files[i].created)+10, y, files[i].modified, color.new(0, 0, 255))
else
if draw.textwidth(files[i].name.."/") > 320-50 then
draw.enableclip(true)
draw.cliprect(10, y, 10+320-5, y+draw.textheight(files[i].name.."/")+10)
draw.text(10, y, files[i].name.."/", color.new(255, 255, 255))
draw.enableclip(false)
else
draw.text(10, y, files[i].name.."/", color.new(255, 255, 255))
end
draw.text(10+300+20, y, files[i].created, color.new(255, 255, 255))
draw.text(10+300+20+draw.textwidth(files[i].created)+10, y, files[i].modified, color.new(255, 255, 255))
end
else
if i == filescroll.sel then
if draw.textwidth(files[i].name) > 320-50 then
draw.enableclip(true)
draw.cliprect(10, y, 10+320-5, y+draw.textheight(files[i].name)+10)
draw.text(10, y, files[i].name, color.new(0, 0, 255))
draw.enableclip(false)
else
draw.text(10, y, files[i].name, color.new(0, 0, 255))
end
draw.text(10+300+20, y, files[i].created, color.new(0, 0, 255))
draw.text(10+300+20+draw.textwidth(files[i].created)+10, y, files[i].modified, color.new(0, 0, 255))
else
if draw.textwidth(files[i].name) > 320-50 then
draw.enableclip(true)
draw.cliprect(10, y, 10+320-5, y+draw.textheight(files[i].name)+10)
draw.text(10, y, files[i].name, color.new(255, 255, 255))
draw.enableclip(false)
else
draw.text(10, y, files[i].name, color.new(255, 255, 255))
end
draw.text(10+300+20, y, files[i].created, color.new(255, 255, 255))
draw.text(10+300+20+draw.textwidth(files[i].created)+10, y, files[i].modified, color.new(255, 255, 255))
end
end
y = y + 20
end
controls.update()
local lx, ly = controls.leftanalog()
if controls.rapidfireheld(SCE_CTRL_UP, 6) or ly <= -64 then filescroll:up() end
if controls.rapidfireheld(SCE_CTRL_DOWN, 6) or ly >= 64 then filescroll:down() end
if controls.pressed(SCE_CTRL_LTRIGGER) then for i=1, 23 do filescroll:up() end end
if controls.pressed(SCE_CTRL_RTRIGGER) then for i=1, 23 do filescroll:down() end end
if controls.released(SCE_CTRL_CROSS) then 
if files[filescroll.sel].isafolder then
if #io.list(files[filescroll.sel].path) >= 1 then
files = io.list(files[filescroll.sel].path)
filescroll:set(files, 24) 
end
else
os.keyboard("Enter text", assert(io.open(files[filescroll.sel].path, "r"):read("*all")), SCE_IME_TYPE_DEFAULT, SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT, SCE_IME_OPTION_MULTILINE)
end
end
if controls.released(SCE_CTRL_CIRCLE) then
if io.filestrip(files[1].path):gsub("/", "", 1) == "ux0:" then
controls.update() break
else
files = io.list(io.filestrip(io.filestrip(files[filescroll.sel].path)))
filescroll:set(files, 24) 
end
end
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
