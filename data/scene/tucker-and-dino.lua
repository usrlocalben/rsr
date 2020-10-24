require "extras"

Truecolor{ id="root",
  sRGB=false, program="IQ",
  gpu=GPU{
    targetSize="globals:windowSize",
    layers={"effect"}}}

Layer{ id="effect",
  camera=Orthographic{l=0, r=8, b=0, t=4.5},
  color=Vec3(0.2, 0.2, 0.2),
  gl="cuddleFieldMoving" }

DINO_ROTATION = T3("{ 0, 0, sin(t/4.0)*0.5+0.5 }")
NO_ROTATION = Vec3(0, 0, 0.1)

-- move field around
ORIGIN_OFFSET = T3("{ 4+sin(t), 2.25+cos(t), 0 }")

Modify{ id="cuddleFieldMoving",
  translate=ORIGIN_OFFSET,
  gl=Modify{
    rotate=NO_ROTATION,  -- DINO_ROTATION to feel sick
    gl="cuddleField" }}

-- tuckerPair is 1x2
tinyOffset = T3("{ frac(t/2), 0, 0 }")
cuddlePair = Group{
  gl={
    -- dino at y=0
    Plane{ material="dinoPicture" },
    -- and tucker at y=1, with x animated by tinyOffset
    Modify{ translate=Vec3{ x=tinyOffset..":x", y=1.0 },
            gl=Plane{ material="tuckerPicture" }}}}

-- create a dim-by-dim field of cuddlePairs
local dim = 24
Modify{ id="cuddleField",
  translate=Vec2(-dim/2, -dim/2),
  gl=Repeat{
    many=dim,
    translate=Vec2(1.0, 0),
    gl=Repeat{
      many=dim/2, -- cuddlePair is 1x2, so half as many in y
      translate=Vec2(0, 2.0),
      gl=Modify{
        scale=Vec2(1, 1),
        translate=Vec2(0.5, 0.5),
        gl=cuddlePair}}}}

Material{ id="dinoPicture",
  program="Amy", filter=true,
  texture0=Image('data/texture/dino512.png') }

Material{ id="tuckerPicture",
  program="Amy", filter=true,
  texture0=Image('data/texture/tiny512.png') }

