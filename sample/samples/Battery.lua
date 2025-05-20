while true do
draw.text(10, 10, "Battery percentage: "..os.batterypercent().."%", white)
draw.text(10, 30, "Is battery charging: "..tostring(os.isbatterycharging()), white)
draw.text(10, 50, "Battery health: "..os.batterySOH().."%", white)
draw.text(10, 70, "Battery lifetime: "..os.batterylifetime().." minutes", white)
draw.text(10, 90, "Battery voltage: "..os.batteryvoltage().." mV", white)
draw.text(10, 110, "Battery cycle count: "..os.batterycyclecount(), white)
draw.text(10, 130, "Battery temperature: "..os.batterytemperature().."°C", white)
draw.text(10, 150, "Battery capacity: "..os.batterycapacity().." mAh", white)
draw.text(10, 170, "Remaining battery capacity: "..os.remainingbatterycapacity().." mAh", white)
draw.text(10, 190, "Is battery low: "..tostring(os.isbatterylow()), white)
draw.text(10, 210, "External battery (true on PSTV): "..tostring(os.externalbattery()), white)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
