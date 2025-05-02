os.message("This is a message!")
local msg = os.message("This is a 2 button message\nClick to see the magic", SCE_MSG_DIALOG_BUTTON_TYPE_YESNO)
while true do
if msg then
draw.text(10, 10, "Accepted", white)
else
draw.text(10, 10, "Declined", white)
end
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
