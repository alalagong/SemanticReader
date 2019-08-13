## 1. 简介
这是一个用于解码在服务器端编码的语义分割信息的C++代码。通过yaml文件可以得到:
- 每个类别的mask，bounding box
- 整幅图像的像素label值
- 每个像素的置信度图

分割网络见[LEDNet](https://github.com/alalagong/LEDNet)。

## 2. `yaml`文件的格式
`data/`文件夹下有两个示例，具体格式如下：
```yaml
instance:
  - category_id:        # 0~18类别标签
    box:                # bounding box
      - y
      - x
      - height
      - width
    segmentation:       # 分割mask
          size:
            - height
            - width
          counts:       # coco编码值
    . . .
    . . .
    . . .
  - category_id: -1     # 图像像素级label
    box:                # 图像大小
      - 0
      - 0
      - 1241
      - 376
    segmentation:       # base64编码值
confidence:             # 图像置信度
```

## 3. 依赖项
- OpenCV
    
    在OpenCV 3.2.0下测试
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
    
    使用它来读取yaml，没有使用opencv自带的，限制太多了。
- CocCoMaskAPI
    
    在`ThirdParty`中，需要提前进行编译，用来对mask进行解码。

## 4. 使用命令
可以参考`src/read_semantic.cpp`来获得相应的信息。

编译：
```shell
cd your_path_to_read_yaml/ThirdParty/CocCoMaskAPI
mkdir build
cd build
cmake ..
make

cd ../../
mkdir build
cd build
cmake ..
make
```
运行：
```shell
./bin/read_yaml data(or other path of yamls)
```
## 5. 参考
- [yolact_stack](https://github.com/yhfeng1995/yolact_stack)
