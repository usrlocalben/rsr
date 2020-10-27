require "extras"

Truecolor{ id="root",
  sRGB=false,
  program="IQ",
  gpu=GPU{ targetSize="globals:windowSize",
           layers={ "lEffects", } }}

BACKGROUND_COLOR=sRGB(128, 128, 128)

Layer{ id="lEffects",
  --camera="uiCamera",
  camera=Perspective{ position=Vec3(0, 0, 130),
                      h=3.14159, v=0, fov=45.0 },
  color=BACKGROUND_COLOR,
  gl={"sceneGeo",}}

--[[debugGl = Group{
  translate=Vec3(1, 1, 0),
  scale=Vec3(1, 1, 1),
  gl=DepthDebug()},]]--

wireframe = Material{ program="Wireframe" }
objPixelLight = Material{ program="OBJ2" }

glLight = Group{
  translate=T3("{ 0, sin(t*1.872398)*6, 140+sin(t)*12}"),
  rotate=Vec3(0,0,0),
  gl={ Spot{ id="spot", size=1024, angle=70.0 },
       Mesh{ id="spotCone", name="cone.obj", material=wireframe } }}

glRqdq = Group{
  translate=Vec3(0,0,0),
  rotate=T3("{(sin(t)+1)/8-.125, (sin(t)+1)/8-.125, 0}"),
  gl=Group{
    translate=Vec3(0, -24, 0),
    rotate=Vec3(0.25, 0, 0),
    scale=T3("{ 2.2, 2.2, 2.2 }"),
    gl=Mesh{ name="rqdqoutline.obj", material=objPixelLight }}}

glRoom = Group{
  translate=Vec3(0,0,0),
  rotate=T3("{0, max(frac(t*1.36), 0.5)*1, (sin(t/1)+1)/8-.125}"),
  gl=Mesh{ name="room1.obj", material=objPixelLight }}

Group{ id="sceneGeo", gl={ glRoom, glRqdq, glLight } }
