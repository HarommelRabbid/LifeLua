while true do
controls.update()
local lx, ly = controls.leftanalog()
local rx, ry = controls.rightanalog()
draw.text(10, 10, "Pressed Cross: "..tostring(controls.pressed(SCE_CTRL_CROSS)), white)
draw.text(10, 30, "Held Circle: "..tostring(controls.held(SCE_CTRL_CIRCLE)), white)
draw.text(10, 50, "Released Square: "..tostring(controls.released(SCE_CTRL_SQUARE)), white)
draw.text(10, 70, "X & Y axis of the left analog stick: "..lx..", "..ly, white)
draw.text(10, 90, "X & Y axis of the right analog stick: "..rx..", "..ry, white)
draw.text(10, 110, "Volume up & down button held: "..tostring(controls.held(SCE_CTRL_VOLUP)).." & "..tostring(controls.held(SCE_CTRL_VOLDOWN)).." (only possible if in unsafe mode)", white)
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
