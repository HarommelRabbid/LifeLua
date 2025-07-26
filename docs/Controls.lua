white = color.new(255, 255, 255) -- no need to define the same thing multiple times
while true do
    controls.update()
    lx, ly = controls.leftanalog()
    rx, ry = controls.rightanalog()
    acc, gyro = controls.accelerometer(), controls.gyroscope()
    fronttouch, reartouch = controls.fronttouch(), controls.reartouch()
    draw.text(10, 10, "Cross "..tostring(controls.pressed(SCE_CTRL_CROSS)):gsub("true", "pressed"):gsub("false", "isn't pressed"), white)
    draw.text(10, 30, "Square "..tostring(controls.held(SCE_CTRL_SQUARE)):gsub("true", "held"):gsub("false", "isn't held"), white)
    draw.text(10, 50, "Circle "..tostring(controls.pressed(SCE_CTRL_CIRCLE)):gsub("true", "released"):gsub("false", "isn't released"), white)
    draw.text(10, 70, "Left analog stick: X: "..lx..", Y:"..ly, white)
    draw.text(10, 90, "Right analog stick: X: "..rx..", Y:"..ry, white)
    draw.text(10, 110, "Accelerometer: X: "..acc.x.." Y: "..acc.y.." Z: "..acc.z, white)
    draw.text(10, 130, "Gyroscope: X: "..gyro.x.." Y: "..gyro.y.." Z: "..gyro.z, white)
    for i=1, #reartouch do
        draw.circle(reartouch[i].x, reartouch[i].y, 35, blue)
    end
    for i=1, #fronttouch do
        draw.circle(fronttouch[i].x, fronttouch[i].y, 35, white)
    end
    draw.swapbuffers()
end