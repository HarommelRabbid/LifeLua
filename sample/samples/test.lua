local msg = os.message("This is a 3 button message!", SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS, "Test", SCE_MSG_DIALOG_FONT_SIZE_DEFAULT, "Test 2", SCE_MSG_DIALOG_FONT_SIZE_DEFAULT)
while true do
--draw.text(10, 10, msg, white)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
