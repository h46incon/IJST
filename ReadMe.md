## ijst

branch | TravisCI (gcc,clang) | AppVeyor (msvc)
:---: | :---: | :---:
master | [![Status][m_i_travis_ci]][m_l_travis_ci] | [![Status][m_i_appveyor]][m_l_appveyor]
dev    | [![Status][d_i_travis_ci]][d_l_travis_ci] | [![Status][d_i_appveyor]][d_l_appveyor]

[m_i_travis_ci]: https://travis-ci.org/h46incon/ijst.svg?branch=master
[m_l_travis_ci]: https://travis-ci.org/h46incon/ijst/branches
[d_i_travis_ci]: https://travis-ci.org/h46incon/ijst.svg?branch=dev
[d_l_travis_ci]: https://travis-ci.org/h46incon/ijst/branches

[m_i_appveyor]: https://ci.appveyor.com/api/projects/status/pkwfr31bdoicw1hd/branch/master?svg=true
[m_l_appveyor]: https://ci.appveyor.com/project/h46incon/ijst/branch/master
[d_i_appveyor]: https://ci.appveyor.com/api/projects/status/pkwfr31bdoicw1hd/branch/dev?svg=true
[d_l_appveyor]: https://ci.appveyor.com/project/h46incon/ijst/branch/dev


ijst (iJsonStruct) 一个是 C++ JSON 序列化/反序列化库：

- 只需定义**一次**结构体，无须重复添加元信息。
- 支持 Getter Chaining，可以很简单地访问路径较深的字段。
- 丰富的语义：支持 unknown 字段和可选字段，支持成员名与 JSON 键名不同，支持添加自定义的类型及序列化行为。
- 支持 UTF-8, UTF-16, UTF-32 编码。
- 轻量：header-only，仅依赖 stl 和 [RapidJSON](https://github.com/Tencent/rapidjson)。
- 反序列失败时，会有详细的错误信息。
- 兼容 C++ 98/03。支持 C++ 11 特性，如右值构造、extern template 等。

## 使用
### 安装
1. 安装 RapidJSON v1.1.0 以上版本（将其加入 header 搜索路径即可）。
2. 将 `include/ijst` 文件夹复制进工程。

### 单元测试（可选）
可通过以下命令安装依赖并执行单元测试：

```shell
cd IJST
git submodule update --init
mkdir CMakeBuild && cd CMakeBuild
cmake ..
make
./unit_test/unit_test
```

### 基本使用

#### 定义结构体
```cpp
#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <ijst/types_container.h>
using namespace ijst;

//*** 需要反序列化的 JSON 字符串
const std::string jsonStr = R"(
{
    "iVal": 42,
    "vecVal": ["str1", "str2"],
    "map_val": {"k1": 1, "k2": 2}
})";

//*** 定义一个 ijst 结构体：
IJST_DEFINE_STRUCT(
    //-- 结构体名字
    JsonStruct

    //-- 定义字段
    // 32 位整型
    , (T_int, iVal)
    // 数组，元素为 string
    , (IJST_TVEC(T_string), vecVal)
    // 字典，元素为 uint64_t；且 JSON 键值与定义的字段名不同
    , (IJST_TMAP(T_uint64), mapVal, "map_val")
);

//*** 默认情况下会生成这样的类：
/*
class JsonStruct {
public:
    ijst::Accessor _;   // 通过这个对象进行序列化等操作
    int iVal;
    std::vector<std::string> vecVal;
    std::map<std::string, uint64_t> mapVal; 

private:
    //... Some private methods
};
*/
```

#### 字段访问及(反)序列化
```cpp
//*** 定义一个 JsonStruct 对象
JsonStruct jStruct;

//*** 反序列化
int ret = jStruct._.Deserialize(jsonStr);
assert(ret == 0);

//*** 访问字段
assert(jStruct.iVal == 42);
assert(jStruct.vecVal[0] == "str1");
assert(jStruct.mapVal["k2"] == 2);

//*** 序列化
std::string strOut;
ret = jStruct._.Serialize(strOut);
assert (ret == 0);
```

### Getter Chaining
如果所需访问的字段的路径比较深的时候，为避免累赘的判断，可使用 `get_*` 方法，比如：

```cpp
//*** 和 IJST_DEFINE_STRUCT 类似
IJST_DEFINE_STRUCT_WITH_GETTER(
    StIn
    // 通过 Optional 描述该字段在 JSON 中是可选的
    , (T_int, iData, ijst::FDesc::Optional)
    , (IJST_TVEC(T_int), vecData, ijst::FDesc::Optional)
    , (IJST_TMAP(T_int), mapData, ijst::FDesc::Optional)
)

//*** 默认情况下会生成这样的结构体：
/*
class JsonStruct {
public:
    //... 普通的字段，同 IJST_DEFINE_STRUCT
    
    // Getters
    ijst::Optional<int> get_iData();
    ijst::Optional<std::vector<int> > get_vecData();
    ijst::Optional<std::map<std::string, int> > get_mapData();
private:
};
*/

IJST_DEFINE_STRUCT_WITH_GETTER(
    StOut
    , (IJST_TST(StOut), stIn, "inner", ijst::FDesc::Optional)
)
StOut st;

//*** 可以通过连串的 get_* 尝试直接访问字段，而不用关注路径的中间节点是否存在
// 下行语句可访问 "/stIn/vecData/2"
// int* pData = st.get_stIn()->get_vecData()[2].Ptr();
// 即：
int* pData = st         // StOut 对象
    .get_stIn()         // 访问 stIn 字段
    ->get_vecData()     // 访问 vecData 字段，注意需使用 -> 操作符
    [2]                 // 访问数组中第2个元素
    .Ptr();             // 获取最后结果的地址
assert (pData == NULL);

// 如果路径中的每个字段都是 kValid 的，且 vector 的下标存在，则最终得到的指针会指向该字段：
// int* pData = st.get_stIn()->get_vecData()[2].Ptr() == &st.stIn.vecData[2];
```

## 性能

ijst 底层使用的是 RapidJSON，其本身具有优秀的性能。
ijst 因有额外的工作，会带来一些性能上的开销，但也比常用的 JsonCpp 快上不少：

| Library   | 序列化 | 反序列化 | 
|-----------|-------|---------|
| RapidJSON | 14    | 10      |
| ijst      | 16    | 23      |
| JsonCpp   | 128   | 109     |

测试环境：Corei7-4790_3.60GHz_vc2017_win7x64，测试代码： [nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark)。
 
注：不同环境测得的性能会有差异，一般而言，ijst 的序列化性能和 RapidJSON 相似，反序列化性能为其 1/4 ~ 1/2。

## 详细说明

ijst 的其他功能，如字段状态，Unknown 字段，JSON 注释等，请移步 [usage](docs/usage.md)，或 `docs/Doxygen/html`。