while true do
controls.update()
local lx, ly = controls.leftanalog()
local rx, ry = controls.rightanalog()
local acc = controls.accelerometer()
local gyro = controls.gyroscope()
local fronttouch, reartouch = controls.fronttouch(), controls.reartouch()
draw.text(10, 10, "Pressed Cross: "..tostring(controls.pressed(SCE_CTRL_CROSS)), white)
draw.text(10, 30, "Held Circle: "..tostring(controls.held(SCE_CTRL_CIRCLE)), white)
draw.text(10, 50, "Released Square: "..tostring(controls.released(SCE_CTRL_SQUARE)), white)
draw.text(10, 70, "X & Y axis of the left analog stick: "..lx..", "..ly, white)
draw.text(10, 90, "X & Y axis of the right analog stick: "..rx..", "..ry, white)
draw.text(10, 110, "Volume up & down button held: "..tostring(controls.held(SCE_CTRL_VOLUP)).." & "..tostring(controls.held(SCE_CTRL_VOLDOWN)).." (only possible if unsafe mode is activated)", white)
draw.text(10, 130, "Accelerometer: X: "..acc.x.." Y: "..acc.y.." Z: "..acc.z, white)
draw.text(10, 150, "Gyroscope: X: "..gyro.x.." Y: "..gyro.y.." Z: "..gyro.z, white)
draw.text(10, 170, "Touch the front & rear touchscreens to see the magic!", white)
draw.text(10, 190, "Headphones detected: "..tostring(controls.held(SCE_CTRL_HEADPHONE)), white)
for i=1, #reartouch do
draw.circle(reartouch[i].x, reartouch[i].y, 35, blue)
end
for i=1, #fronttouch do
draw.circle(fronttouch[i].x, fronttouch[i].y, 35, white)
end
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
