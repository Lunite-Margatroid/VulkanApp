# 路线选择

`Lua` `sol2` 

# built-in Var

`SELECTED_S` 当前选中的场景ID 暂时为常数 禁止修改

`SELECTED_N` 当前选中的节点ID

`SELECTED_MTL` 当前选中的材质ID

`SELECTED_MESH` 当前选中的Mesh的ID (未实现)

`SELECTED_V` 当前选中的视图ID (未实现)

`SELECTED_D` 当前选中的显示设备ID (未实现)

`SELECTED_IMG` 当前选中的图像ID

# Scene

**创建显示设备 (未实现)**

`engine.displayDevice <type>` 

---

**打印显示设备  (未实现)**

`engine.displayDevice.list` 

---

**创建场景 (未实现)**

`scene` 创建场景 id赋值给`SELECTED_S`  (未实现)

---

**打印场景列表 (未实现)**

`scene.list` 打印场景列表 (未实现)

---

**创建视图**(未实现)

`view` 创建视图 id赋值给`SELECTED_V` (未实现)

---

**在视图上渲染场景 (未实现)**

`view.scene [view_id:SELECTED_V] [scene_id:SELECTED_S]` 在视图上渲染场景 (未实现)

---

**视图渲染到文件 (未实现)**

`view.saveImg <view_id> <file_path>` 视图渲染到文件 (未实现)

---

**在显示设备上绘制视图 (未实现)**

`engine.displayDevice.view [displayDevice_id:SELECTED_D] [view_id:SELECTED_V]`   在显示设备上绘制视图 (未实现)

# Material

**创建材质**

**`cmd`**: `mtl.create <str_mtl_type>` 

创建材质 id赋值给`SELECTED_M`

```cmd
mtl.create "Phong"
```

**`lua`**: `engine.mtl.create(mtl_type : string)->int`


```lua
mtl_id = engine.mtl.create("Phong")
```

---

**设置材质属性**

**`cmd`**: `mtl.prop <mtl_id> <prop> <value...>`

设置材质属性

```cmd
mtl.prop $SELECTED_MTL "ColorDiffuse" vec3(1,1,1)
mtl.prop $SELECTED_MTL "TexDiffuse" 1
mtl.prop 9 "Roughness" 1
```

**`lua`**: `engine.mtl.prop(mtl_id : int, prop_name : string, value : float | vec3 | int )->void`

```lua
engine.mtl.prop(0, "ColorDiffuse", value)
```



# SceneNode

**创建节点**

**`cmd`**: `node.create` 

创建节点赋值给`SELECTED_N`

```cmd
node.create
node.position $SELECTED_N vec3(1,1,1)
```

**`lua`**: `engine.node.create()->int`

```lua
node_id = engine.node.create()
```

---

**节点rebase**

**`cmd`**: `node.rebase <parent_node> <child_node>`

把`parent_node`设为`child_node`的父节点

```cmd
node.rebase $SELECTED_N 9
node.rebase $SELECTED_S $SELECTED_N
```

**`lua`**: `engine.node.rebase(parent_node : int, child_node : int)->void`

```lua
engine.node.rebase(parent_node, child_node)
```

---

**设置材质**

**`cmd`**: `node.mtl <node_id> <mtl_id>`

把`mtl_id`挂在`node_id`上

```cmd
node.mtl $SELECTED_N $SELECTED_MTL
node.mtl 6 7
```

**`lua`**: `engine.node.mtl(node_id : int, mtl_id : int)->void`

```lua
engine.node.mtl(node_id, mtl_id)
```

---

**节点transform**

**`cmd`**: `node.position <node_id> vec(<x>,<y>,<z>)`

设置节点位置

**`cmd`**:`node.eulerXYZ <node_id> vec(<roll>,<pitch>,<yaw>)` 

欧拉旋转 XYZ外旋

**`cmd`**:`node.scale <node_id> vec(<x>,<y>,<z>)` 

缩放节点

```cmd
node.position $SELECTED_N vec(2,2,2)
node.eulerXYZ $SELECTED_N vec(1.57,0,0)
node.scale $SELECTED_N vec(0.5,1,1)
```

**`lua`**: `engine.node.position(node_id : int, pos : vec3)->void`

**`lua`**: `engine.node.eulerXYZ(node_id : int, rotate : vec3)->void`

**`lua`**: `engine.node.scale(node_id : int, scale : vec3)->void`

```lua
engine.node.position(node_id, vec3(2,2,2))
engine.node.eulerXYZ(node_id, vec3(1.57,0,0))
engine.node.scale(node_id, vec3(0.5,1,1))
```

---

**创建内置几何体节点**

**`cmd`**: `node.mesh.cube <side_len>`

创建边长为side_len的立方体节点, 置为`SELECTED_S`的子节点, 节点ID赋值给`SELECTED_N`

**`cmd`**:`node.mesh.sphere <radius> <long_sub> <lat_sub>` 

创建半径为radius的球体节点 经向细分`long_sub` 纬向细分`lat_sub` 置为`SELECTED_S`的子节点 节点ID赋值给`SELECTED_N`

**`cmd`**:`node.mesh.plane <side_len>` 

创建边长为side_len的正方形面片节点 置为`SELECTED_S`的子节点 节点ID赋值给`SELECTED_N`

```cmd
node.mesh.cube 1.0
node.mesh.sphere 1.0 32 16
node.mesh.plane 2.0
```

**`lua`**:`engine.node.mesh.cube(side_len : float | int)->int`

**`lua`**:`engine.node.mesh.sphere(radius: float | int , long_sub : int, lat_sub : int)->int`

**`lua`**:`engine.node.mesh.plane(side_len : float | int)->int`

```lua
cube_id = engine.node.mesh.cube(1.0)
sphere_id = engine.node.mesh.sphere(1.0, 32, 16)
plane_id = engine.node.mesh.plane(2.0)
```

---

**创建灯光**

**`cmd`**: `node.light.create <light_type>`

创建灯光节点 id赋给`SELECTED_N` 并把它置为`SELECTED_S`的子节点

```cmd
node.light.create "LightDirectional"
```

**`lua`**: `engine.node.light.create(light_type : string)->int`

```lua
light_id = engine.node.light.create("LightDirectional")
```

# Image

**`cmd`**:`img.file <str_file>`  

从文件创建Image 把ID赋值给`SELECTED_IMG`

```cmd
img.file "./TestAsset/UVTest.png"
```

**`lua`**: `engine.img.file(file_paht : string)->int`

```lua
img_id = engine.img.file("./TestAsset/UVTest.png")
```







