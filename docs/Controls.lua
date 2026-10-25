-- Create the colors once, before the loop
local white = color.new(255, 255, 255)
local blue = color.new(0, 0, 255)

while true do
    controls.update() -- read the latest input, once per frame

    local lx, ly = controls.leftanalog()
    local rx, ry = controls.rightanalog()
    local acc, gyro = controls.accelerometer(), controls.gyroscope()
    local fronttouch, reartouch = controls.fronttouch(), controls.reartouch()

    draw.text(10, 10, "Cross: "..(controls.check(SCE_CTRL_CROSS) and "down" or "up"), white)
    draw.text(10, 30, "Square: "..(controls.held(SCE_CTRL_SQUARE) and "held" or "not held"), white)
    draw.text(10, 50, "Circle: "..(controls.released(SCE_CTRL_CIRCLE) and "released!" or "-"), white)
    draw.text(10, 70, "Left analog stick: X: "..lx..", Y: "..ly, white)
    draw.text(10, 90, "Right analog stick: X: "..rx..", Y: "..ry, white)
    draw.text(10, 110, string.format("Accelerometer: X: %.2f Y: %.2f Z: %.2f", acc.x, acc.y, acc.z), white)
    draw.text(10, 130, string.format("Gyroscope: X: %.2f Y: %.2f Z: %.2f", gyro.x, gyro.y, gyro.z), white)

    -- A blue circle for each finger on the rear touch pad, a white one for the front touch screen
    for i = 1, #reartouch do
        draw.circle(reartouch[i].x, reartouch[i].y, 35, blue)
    end
    for i = 1, #fronttouch do
        draw.circle(fronttouch[i].x, fronttouch[i].y, 35, white)
    end

    draw.swapbuffers()
end
