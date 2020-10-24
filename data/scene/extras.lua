require "host"

function AuraForLauraMT(args)
    assert(args.material)
    assert(args.freq)
    assert(args.phase)
    assert(args.amp)
    local auraDivs = args.divs or 6

    local hunkIds = {}
    for n = 0, 3 do
        local iid = AuraForLaura{
            hunk=n,
            material=args.material,
            freq=args.freq, phase=args.phase, amp=args.amp,
            divs=auraDivs }
        table.insert(hunkIds, iid)
    end
    return Group(hunkIds)
end

function T3(id, expr)
    if expr == nil then
        -- id is expr
        return ComputedVec3{
            inputs={{ name="t", type="real", source="globals:wallclock" },},
            code=id}
    else
        return ComputedVec3{ id=id,
            inputs={{ name="t", type="real", source="globals:wallclock" },},
            code=expr}
    end
end

function Linear(s)
  return math.pow(s/255.0, 2.2333)
end

function sRGB(r, g, b)
  return Vec3(Linear(r), Linear(g), Linear(b))
end


