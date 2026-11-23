# 路线选择

`Lua` `sol2` `replxx`

# built-in Var

`SELECTED_S` 当前选中的场景ID

`SELECTED_N` 当前选中的节点ID

`SELECTED_MTL` 当前选中的材质ID

`SELECTED_MESH` 当前选中的Mesh的ID

`SELECTED_V` 当前选中的视图ID

`SELECTED_D` 当前选中的显示设备ID

# Scene

`engine.displayDevice <type>` 创建显示设备

`engine.displayDevice.list` 打印显示设备

`scene` 创建场景 id赋值给`SELECTED_S` 

`scene.list` 打印场景列表

`view` 创建视图 id赋值给`SELECTED_V`

`view.scene [view_id:SELECTED_V] [scene_id:SELECTED_S]` 在视图上渲染场景

`view.saveImg <view_id> <file_path>` 视图渲染到文件

`engine.displayDevice.view [displayDevice_id:SELECTED_D] [view_id:SELECTED_V]`   在显示设备上绘制视图

# Material

`mtl <mtl_type>` 创建材质 id赋值给`SELECTED_M`

`mtl.prop <mtl_id> <prop> <value...>` 设置材质属性

# SceneNode

`node` 创建节点

`node.mesh [node_id:SELECTED_N] [mesh_id:SELECTED_MESH]` 把mesh挂在node上

`node.rebase [node_id] [node_id_base]` 把node_id_base设为node_id的父节点

`node.mtl [node_id:SELECTED_N] [mtl_id:SELECTED_MTL]`  把mtl_id挂在node_id上

`node.position <node_id> <x> <y> <z>` 设置节点位置

`node.eulerXYZ <node_id> <roll> <pitch> <yaw>` 欧拉旋转 XYZ外旋

`node.scale <node_id> <x> <y> <z>` 缩放

# Model

`mesh` 创建mesh

`mesh.cube <side_len>` 创建边长为side_len的立方体

`mesh.sphere <radius>`  创建半径为radius的球体

`mesh.plane <side_len>` 创建边长为side_len的正方形面片





