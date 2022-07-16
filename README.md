# PBR渲染器

3190100926 陈丰

## 简介

本项目基于OpenGL实现了一个PBR渲染器，支持特性如下：

- 读取OBJ模型
- 读取Albedo, Normal, Metallic, Roughness, AO贴图
- 两种渲染方式：Simple和PBR
- 控制平行光和点光源的位置、方向、颜色
- 控制物体的位置、方向、缩放
- 摄像机在场景中的自由移动
- 导出截图

## 操作方式

### 命令行参数

Windows下的可执行文件为`bin/final.exe`，支持命令行参数如下：

- -model: obj格式的模型的路径
- -size: 等比缩放，默认为1.0
- -albedo: Albedo贴图的路径
- -normal: 法线贴图的路径
- -metallic: 金属贴图的路径
- -roughness: 粗糙贴图的路径
- -ao: AO贴图的路径

例：

```
final.exe -model ../data/switch/switch.obj -albedo ../data/switch/Nintendo_Switch_Material_albedo.jpg -metallic ../data/switch/Nintendo_Switch_Material_metallic.jpg -normal ../data/switch/Nintendo_Switch_Material_normal.png -roughness ../data/switch/Nintendo_Switch_Material_roughness.jpg -ao ../data/switch/Nintendo_Switch_Material_AO.jpg
```

若无-model参数，默认载入`data/ext/Extintor.obj`以及相关的贴图。

效果：

![switch2](.\pictures\switch2.png)

### 相机运动

按住Ctrl后可以使用鼠标改变相机朝向，使用WASD在场景中漫游（类似Unreal Engine的操作方式）。