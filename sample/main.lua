local white = color.new(255, 255, 255)
local blue = color.new(0, 0, 255)
local mode = 1
while true do
  if mode == 1 then
    draw.text(10, 10, "Hello from LifeLua!", white)
    draw.swapbuffers()
    os.delay(2)
    mode = 2
  elseif mode == 2 then
    draw.text(10, 10, "Hello again!", white)
    draw.swapbuffers(blue)
    os.delay(2)
    mode = 1
  end
end
