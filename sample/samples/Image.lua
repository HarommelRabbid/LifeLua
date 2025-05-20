local img = image.load(os.importphoto(0, 0, 255, 255/2) or "app0:pic0.png")
while true do
img:display(0, 0)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
