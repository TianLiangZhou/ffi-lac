## ffi-lac

`ffi-lac`是一个PHP高性能智能中文分词库，基于百度开源的[lac](https://github.com/baidu/lac) 项目，使用`C++`导出`C`函数构建动态链接库来给`php`调用。
项目中大部分源码都可以从 https://github.com/baidu/lac 找到。

### 环境

需要`php >= 7.4` 以上的版本并且开启了`FFI`扩展。

还需要设置`php.ini` 中的`ffi.enable`为`On`。

### paddlepaddle

推理预测库版本: [2.5](https://www.paddlepaddle.org.cn/inference/v2.5/guides/introduction/index_intro.html)

项目依赖: __paddle推理预测库__([点击去下载](https://www.paddlepaddle.org.cn/inference/v2.5/guides/install/download_lib.html) )，__预测模型库__([点击去下载](https://github.com/baidu/lac/releases/tag/v2.1.0) )。

### Linux

将下载的推理预测库存放到: `/opt/paddle_inference`目录。

可以通过`ldd lib/liblacffi.so`显示库的依赖, 再根据情况建立软链。

```shell 
[meshell@/] ln -s paddle_inference/paddle/lib/libpaddle_inference.so libpaddle_inference.so
[meshell@/] ln -s paddle_inference/third_party/install/mklml/lib/libmklml_intel.so libmklml_intel.so
[meshell@/] ln -s paddle_inference/third_party/install/mklml/lib/libiomp5.so libiomp5.so
[meshell@/] ln -s paddle_inference/third_party/install/mkldnn/lib/libmkldnn.so.0 libdnnl.so.2
```

```shell
ffi-lac git:(main) ✗ ldd lib/liblacffi.so
	linux-vdso.so.1 (0x00007ffc0939f000)
	libpaddle_inference.so => /opt/paddle_inference/paddle/lib/libpaddle_inference.so (0x00007f397be00000)
	libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f397bb93000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f397bb6f000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f397b946000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f3983256000)
	libpaddle2onnx.so.1.0.0rc2 => /lib/libpaddle2onnx.so.1.0.0rc2 (0x00007f397b000000)
	libonnxruntime.so.1.11.1 => /lib/libonnxruntime.so.1.11.1 (0x00007f397a198000)
	librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007f3983251000)
	libdnnl.so.2 => /lib/libdnnl.so.2 (0x00007f3977e00000)
	libiomp5.so => /lib/libiomp5.so (0x00007f3977a00000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f397b85f000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f3983290000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f398324a000)
	libgomp.so.1 => /lib/x86_64-linux-gnu/libgomp.so.1 (0x00007f39779ac000)
	
```

### THULAC

`thulac`由清华大学自然语言处理与社会人文计算实验室研制推出的一套中文词法分析工具包，具有中文分词和词性标注功能。
目前是基于 [thulac.so](https://github.com/thunlp/THULAC.so) 导出`.so`供FFI调用。该库需要字典词库下载地址: [thulac.thunlp.org](thulac.thunlp.org)

### Usage 

```php
<?php

include __DIR__ . '/../src/LAC.php';

$dictDir = ""; // 默认库根目录下的model/lac_model

$lac = \FastFFI\LAC\LAC::new($dictDir);

var_dump(
    $lac->parse("LAC智能中文分词库")
);


$thuLac = \FastFFI\LAC\ThuLAC::new();

var_dump(
    $thuLac->parse("thulac智能中文分词库")
);



```

以上程序执行后的结果: 

```php 
array(2) {
  ["words"]=>
  string(29) "LAC 智能 中文 分 词库 "
  ["tags"]=>
  string(11) "nz n n v n "
}

array(3) {
  ["words"]=>
  string(31) "thulac 智能 中文 分词库 "
  ["tags"]=>
  string(9) "x n nz n "
  ["weight"]=>
  string(0) ""
}
```

结果分别为词和标签都是以空格分隔。

百度LAC以下是标签含义：

| 标签 | 含义     | 标签 | 含义     | 标签 | 含义     | 标签 | 含义     |
| ---- | -------- | ---- | -------- | ---- | -------- | ---- | -------- |
| n    | 普通名词 | f    | 方位名词 | s    | 处所名词  | nw   | 作品名   |
| nz   | 其他专名 | v    | 普通动词 | vd   | 动副词   | vn   | 名动词   |
| a    | 形容词   | ad   | 副形词   | an   | 名形词   | d    | 副词     |
| m    | 数量词   | q    | 量词     | r    | 代词     | p    | 介词     |
| c    | 连词     | u    | 助词     | xc   | 其他虚词 | w    | 标点符号 |
| PER  | 人名     | LOC  | 地名     | ORG  | 机构名   | TIME | 时间     |

权重定义:

| 标签 | 含义       | 常见于词性|
| ---- | --------  | ----   | 
| 0    | query中表述的冗余词   |  p, w, xc ...    | 
| 1    | query中限定较弱的词   |  r, c, u ...     | 
| 2    | query中强限定的词     |  n, s, v ...     | 
| 3    | query中的核心词       |  nz, nw, LOC ... | 


THULAC标签含义:



| 标签 | 含义      | 标签     | 含义     | 标签    | 含义     | 标签    | 含义  | 标签    | 含义  |
|----|---------|--------|--------|-------|--------|-------|-----| ---- | ---- |
| n  | 名词      | np     | 人名     | ns    | 地名     | ni    | 机构名 | nz |其它专名|
|m|数词| q|量词 |mq|数量词| t|时间词| f|方位词 |s|处所词|
|v|动词 |a|形容词| d|副词| h|前接成分| k|后接成分 |
|i|习语 |j|简称| r|代词| c|连词| p|介词| u|助词 |y|语气助词|
|e|叹词 |o|拟声词 |g|语素| w|标点| x|其它 |


> 该项目不支持`window`环境，如果你需要请自行编译。

[在线转换](http://loocode.com/tool/lac/chinese-word-segmentation)
