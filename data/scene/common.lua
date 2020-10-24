local common = {}

-- uuid4
function common.uuid()
    local template = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'
    return string.gsub(template, '[xy]', function (c)
        local v = (c == 'x') and math.random(0, 0xf) or math.random(8, 0xb)
        return string.format('%x', v)
    end)
end

-- sequence id with global counter (e.g. one-shot scripts)
local nextId = 0
function common.autoid(t)
    local id = '__' .. (t or 'auto') .. nextId
    nextId = nextId + 1
    return id
end

return common
