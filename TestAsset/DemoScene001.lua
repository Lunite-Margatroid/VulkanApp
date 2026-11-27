-- 测试场景
-- 包含400个立方体 1个平行光 1个Phong材质 加载UVTest.png贴图作为

local img_id = engine.img.file("./TestAsset/UVTest.png")
local mtl_id = engine.mtl.create("Phong")

engine.mtl.prop(mtl_id, "TexDiffuse", img_id)
engine.mtl.prop(mtl_id, "ColorDiffuse", vec3(1.0,1.0,1.0))
engine.mtl.prop(mtl_id, "Roughness", 0.5)

engine.mtl.prop(mtl_id, "IOR", 1.5)
engine.mtl.prop(mtl_id, "Metalness", 0)
engine.mtl.prop(mtl_id, "ColorSpecular", vec3(1.0,1.0,1.0))

local light_id = engine.light.create("LightDirectional")
engine.light.color(light_id, vec3(1,1,1))


local function rand_sphere(r)
    local z = math.random() * 2.0 - 1.0
    local phi = math.random() * math.pi * 2.0
    local t = math.sqrt(1.0 - z * z) * r
	return vec3(t * math.cos(phi), t * math.sin(phi), z * r)
end

local cube_count = 0

while(cube_count < 400)
do
	local cube_id = engine.node.mesh.cube(1.0)
	engine.node.mtl(cube_id, mtl_id)
	engine.node.position(cube_id, rand_sphere(20.0))
	engine.node.eulerXYZ(cube_id, vec3(math.random() * math.pi, math.random() * math.pi, math.random() * math.pi))
	cube_count = cube_count + 1
end
