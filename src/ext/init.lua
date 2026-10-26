function math.range(num, min, max)
    return math.min(math.max(num, min), max)
end

function table.find(value, table) --Checks if an item is in an array
    for i, v in ipairs(table) do
  		if v == value then
           	return true
  		end
    end
    return false
end

function table.removefirst(value, table1) --Removes the first instance of an item in an array
    for i, v in ipairs(table1) do
  		if v == value then
           	return table.remove(table1, i)
  		end
    end
end

function math.round(num, idp)
    local mult = 10^(idp or 0)
    return math.floor(num * mult + 0.5) / mult
end

function math.inrange(num, min, max)
    return num >= min and num <= max
end

function os.installvpk(path, head)
    io.extract(path, "ux0:data/ll_temp")
    if not head then os.installdir("ux0:data/ll_temp")
    else os.installdir("ux0:data/ll_temp", true) end
end