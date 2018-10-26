# 简介
这是一个从 [OpenNT](http://www.opennt.net/) 中提取的CMD源码及其编译环境(x86), 精简掉了不需要的部分

源码路径: `./nt4/private/windows/cmd`

编译方法: 执行 `./start_build.bat `

生成二进制文件路径: `./nt4/private/windows/cmd/cmd/objxxx/x86/`, `./binn/x86xxx/`

已在 WindowsXP(x86) 及 Windows7(x64) 下测试通过

BUG: 在Win7上进行build时会弹出大量iexplore窗口, 目前解决方案是在build完后全部taskkill掉 

## 相关项目
OpenNT: https://github.com/Paolo-Maffei/OpenNT

NTOSBE: https://github.com/stephanosio/NTOSBE
