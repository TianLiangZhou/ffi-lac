## ffi-lac

`ffi-lac`是一个PHP高性能智能中文分词库，基于百度开源的[lac](https://github.com/baidu/lac) 项目，使用`C++`导出`C`函数构建动态链接库来给`php`调用。
项目中大部分源码都可以从 https://github.com/baidu/lac 找到。

### 环境

需要`php >= 7.4` 以上的版本并且开启了`FFI`扩展。

还需要设置`php.ini` 中的`ffi.enable`为`On`。

项目依赖: __paddle预测库__([点击去下载](https://www.paddlepaddle.org.cn/documentation/docs/zh/develop/guides/05_inference_deployment/inference/build_and_install_lib_cn.html) )，__预测模型库__([点击去下载](https://github.com/baidu/lac/releases/tag/v2.1.0) )。

需要为预测库建立软链。

```shell 
[meshell@/] ln -s paddle_inference/paddle/lib/libpaddle_inference.so libpaddle_inference.so
[meshell@/] ln -s paddle_inference/third_party/install/mklml/lib/libmklml_intel.so libmklml_intel.so
[meshell@/] ln -s paddle_inference/third_party/install/mklml/lib/libiomp5.so libiomp5.so
[meshell@/] ln -s paddle_inference/third_party/install/mkldnn/lib/libmkldnn.so.0 libdnnl.so.2
```


> 该目录不支持`window`环境，如果你需要请自行编译。

### Usage 

```php
<?php

include __DIR__ . '/../src/LAC.php';

$dictDir = ""; // 默认库根目录下的model/lac_model

$lac = \FastFFI\LAC\LAC::new($dictDir);

var_dump(
    $lac->parse("LAC智能中文分词库")
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

```

结果分别为词和标签都是以空格分隔。

以下是标签含义：

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


[在线转换](http://loocode.com/tool/lac/chinese-word-segmentation)
