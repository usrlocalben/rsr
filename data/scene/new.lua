HOST = require "host"

Vec3(3, 4, 5)
Vec3{id='foo', x=11, y=22, z=33}
Truecolor{id="mytc", sRGB=true, program="Default", gpu=Vec3(6, 7, 8)}
Layer{ id="lAmy",
    camera=Orthographic(0, 16, 0, 9),
    color=Vec3(math.pow(238/255.0, 2.2333), math.pow(248/255.0, 2.2333), 1.0),
    gl="nAmy"}
ComputedVec3{ id="auraRotation",
    inputs={{ name="t", type="real", source="#globals:wallclock" },},
    code="{0, sin(t*0.5)*0.2+0.2, sin(t*0.222)*0.1}"}
Noise(Vec3(4, 5, 6))
Noise{ coord="foo" }

Group('a', 'b')
HOST:dump()
