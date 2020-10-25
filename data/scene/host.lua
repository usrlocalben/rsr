json = require "json"
common = require "common"
-- inspect = require "inspect"

local GUID = common.autoid

local function fold_id(a)
    local out = {}
    for k, v in pairs(a) do
        out[k] = v
    end
    out.id = out.id or GUID()
    return out
end

local function pair(k, v)
    local tmp = {}
    tmp[k] = v
    return tmp
end


HOST = { items={} }
function HOST:compile(tag, data)
    local tmp = pair("$"..tag, data)
    table.insert(self.items, json.encode(tmp))
end

function HOST:dump()
    print('[')
    for n = 1, #self.items-1 do
        print(self.items[n] .. ',')
    end
    print(self.items[#self.items])
    print(']')
end

function Vec3(...)
    local id, x, y, z
    local data
    if type(...) == 'table' then
        local t = ...
        data = fold_id(t)
    else
        x, y, z = ...
        data = {id=GUID(), x=x, y=y, z=z}
    end
    HOST:compile('vec3', data)
    return data.id
end

function Vec2(...)
    local id, x, y
    local data
    if type(...) == 'table' then
        local t = ...
        data = fold_id(t)
    else
        x, y = ...
        data = {id=GUID(), x=x, y=y}
    end
    HOST:compile('vec2', data)
    return data.id
end

function Float(...)
    local id, x
    local data
    if type(...) == 'table' then
        data = fold_id(...)
    else
        x = ...
        data = {id=GUID(), x=x}
    end
    HOST:compile('float', data)
    return data.id
end

function String(...)
    local id, x
    local data
    if type(...) == 'table' then
        data = fold_id(...)
    else
        x = ...
        data = {id=GUID(), data=x}
    end
    HOST:compile('string', data)
    return data.id
end

function Truecolor(d)
    local data = fold_id(d)
    HOST:compile('truecolor', data)
    return data.id
end

function Orthographic(...)
    local data
    if type(...) == 'table' then
        data = fold_id(...)
    else
        local l, r, b, t
        l, r, b, t = ...
        data = {id=GUID(), l=l, r=r, b=b, t=t}
    end
    HOST:compile('orthographic', data)
    return data.id
end

function GPU(d)
    local data = fold_id(d)
    HOST:compile('gpu', data)
    return data.id
end

function Layer(d)
    local data = fold_id(d)
    if type(data.gl) ~= 'table' then
        data.gl = { data.gl, }
    end

    HOST:compile('layer', data)
    return data.id
end

function Perspective(d)
    data = fold_id(d)
    HOST:compile('perspective', data)
    return data.id
end

function Modify(d)
    data = fold_id(d)
    HOST:compile('modify', data)
    return data.id
end

function Repeat(d)
    data = fold_id(d)
    HOST:compile('repeat', data)
    return data.id
end

function Plane(d)
    data = fold_id(d)
    HOST:compile('plane', data)
    return data.id
end

function Material(d)
    data = fold_id(d)
    HOST:compile('material', data)
    return data.id
end

function Image(...)
    local id, fn
    if type(...) == 'table' then
        local args = ...
        id = args.id or GUID()
        fn = args.file
    else
        id = GUID()
        fn = ...
    end
    local tmp = { id=id, file=fn }
    HOST:compile('image', tmp)
    return id
end

function ComputedVec3(d)
    data = fold_id(d)
    HOST:compile('computedVec3', data)
    return data.id
end

function Noise(...)
    local id, coord, data
    if type(...) == 'table' then
        data = fold_id(...)
    else
        data = {id=GUID(), coord=...}
    end
    HOST:compile('noise', data)
    return data.id
end

function AuraForLaura(d)
    local data = fold_id(d)
    HOST:compile('auraForLaura', data)
    return data.id
end

function Kawase(d)
    local data = fold_id(d)
    HOST:compile('kawase', data)
    return data.id
end

function Glow(d)
    local data = fold_id(d)
    HOST:compile('glow', data)
    return data.id
end

function Buffers(d)
    local data = fold_id(d)
    HOST:compile('buffers', data)
    return data.id
end

function Many(d)
    local data = fold_id(d)
    HOST:compile('many', data)
    return data.id
end

function MC(d)
    local data = fold_id(d)
    HOST:compile('mc', data)
    return data.id
end

function Mesh(d)
    local data = fold_id(d)
    HOST:compile('mesh', data)
    return data.id
end

function Writer(d)
    local data = fold_id(d)
    HOST:compile('writer', data)
    return data.id
end

function Spot(d)
    local data = fold_id(d)
    HOST:compile('spot', data)
    return data.id
end

function Controller(d)
    local data = fold_id(d)
    HOST:compile('controller', data)
    return data.id
end

function Group(...)
    local data
    if type(...) == 'table' then
        local t = ...
        if t[1] ~= nil then
            data = {id=GUID(), gl=t}
        else
            data = fold_id(...)
        end
    else
        local tmp = {}, data
        for n = 1, select('#', ...) do
            tmp[#tmp+1] = select(n, ...)
        end
        data = {id=GUID(), gl=tmp}
    end
    HOST:compile('group', data)
    return data.id
end

return HOST
