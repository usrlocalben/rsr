require "host"

function concat(a, b)
  out = {}
  for i=1,#a do
    out[#out+1] = a[i]
  end
  for i=1,#b do
    out[#out+1] = b[i]
  end
  return out
end

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

function Mul_Float_Float(a, b)
    return ComputedVec3{
        inputs={
            { name="a", type="real", source=a },
            { name="b", type="real", source=b } },
        code="{ a*b, a*b, a*b }"}
end

function Mul_Vec3_Float(a, b)
    return ComputedVec3{
        inputs={
            { name="a", type="vec3", source=a },
            { name="b", type="real", source=b } },
        code="{ a[0]*b, a[1]*b, a[2]*b }"}
end

function Mul_Vec3_Vec3(a, b)
    return ComputedVec3{
        inputs={
            { name="a", type="vec3", source=a },
            { name="b", type="vec3", source=b } },
        code="{ a[0]*b[0], a[1]*b[1], a[2]*b[2] }"}
end

function Merge3(a, b, c)
    return ComputedVec3{
        inputs={
            { name="a", type="real", source=a },
            { name="b", type="real", source=b },
            { name="c", type="real", source=c },},
        code="{ a, b, c }"}
end

function Linear(s)
  return (s/255.0)^2.2333
end

function sRGB(r, g, b)
  return Vec3(Linear(r), Linear(g), Linear(b))
end


