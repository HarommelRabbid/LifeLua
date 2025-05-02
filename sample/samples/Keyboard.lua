if os.message("Do you want the multiline IME keyboard instead of the normal one?", SCE_MSG_DIALOG_BUTTON_TYPE_YESNO) then
text = os.keyboard("Enter text", "Hello\nfrom\nLifeLua!", SCE_IME_TYPE_DEFAULT, SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT, SCE_IME_OPTION_MULTILINE)
else
text = os.keyboard("Enter text", "Hello from LifeLua!")
end
local text = text
while true do
draw.text(10, 10, text or "IME dialog was canceled", white)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
