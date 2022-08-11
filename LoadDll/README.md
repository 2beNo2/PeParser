# Load Dll From File
- 不通过LoadLibrary来装载dll文件，可以隐藏dll
  ```
  - 申请SizeOfImage大小的内存
  - 拷贝PE头部
  - 拉伸PE节区并拷贝
  - 修复导入表
  - 重定位数据修复
  - 调用dllMain
  ```
