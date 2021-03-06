# 3维场景漫游实验

### 完成功能

- 三维模型的加载和显示
- 用GLSL实现phong光照明模型
- 实现模型中纹理的加载
- 不同光源下阴影的生成
- 交互改变相机位姿
- 碰撞检测
- 简单光线追踪算法的实现

### 开发环境

项目在ubuntu18.04环境下进行开发，使用cmake作为项目的管理工具。

除了使用assimp用作网格文件读入之外，没有使用其他的库，三维绘制，阴影生成，phong模型，以及纹理加载都通过GLSL实现。

项目通过linuxdeployqt进行打包。



### 交互说明

按下N键可以进入navagation状态，通过ASDW控制相机在水平方向前后左右的移动，通过Z和X控制相机的上下运动，移动鼠标即可改变视角

按下T键可以运行光线追踪程序，在命令行窗口中会提示当前计算完成的像素数目与总像素数目之比，程序在这个地方另外开启线程进行计算，不会导致漫游出现卡顿。命令行提示计算完成之后会在当前deploy_cp目录下生成一张名叫ray_tracing的图片。

由于模型存在环绕方向不一致的问题导致法向不一致，目前光线追踪的效果还是不够好。



### 代码编译

在qt creator中导入项目的时候选择在代码中的CMakeList.txt即可。在编译完成之后将deploy_cp目录下的`start.sh`, `simple_scene.obj`, `simple_scene.mtl`以及`windmill`拷贝到生成exe的目录下。如上所述用start.sh控制项目的启动即可。