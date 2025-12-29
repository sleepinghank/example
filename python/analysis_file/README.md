# 文件分析器

## 功能介绍

本工具用于分析指定文件夹下特定格式的文件，自动提取文件名中的信息并生成汇总报告。

**支持的文件格式**：
- `projectname_country_v.v.v.yymmdd_release.bin`（无CRC）
- `projectname_country_v.v.v.yymmdd_CRC_release.bin`（有CRC）

**提取的信息**：
- `projectname` - 项目名称
- `country` - 国家代码
- `v.v.v` - 版本号
- `yymmdd` - 日期（格式化为 YYYY-MM-DD）
- `crc` - CRC校验值（可选，无CRC时为None）
- `release` - 发布类型（仅提取 "beta" 或 "release"，忽略后缀）
- `filename` - 完整文件名
- `file_size` - 文件大小（字节）

**文件格式示例**：
- 无CRC：`KB04122_CN_1.0.0.240101_release.bin`
- 有CRC：`KB903_DE_0.4.3.250915_A4A2_release_Open.bin`
- release字段处理：`release_Encrypt.bin` → 提取为 `release`

## 文件结构

```
analysis_file/
├── file_analyzer.py    # 核心分析器类
├── quick_analyze.py    # 快速分析脚本
├── requirements.txt    # 依赖包列表
└── README.md          # 本文档
```

## 安装依赖

```bash
pip install -r requirements.txt
```

## 使用方法

### 方法一：使用快速分析脚本（推荐）

1. 编辑 `quick_analyze.py` 文件，修改 `folder_path` 变量：

```python
folder_path = r"D:\Data\Document\ProductsLibrary\C线产品资料库"
```

2. 运行脚本：

```bash
python quick_analyze.py
```

3. 按照交互提示进行操作

### 方法二：在代码中使用 FileAnalyzer 类

```python
from file_analyzer import FileAnalyzer

# 创建分析器
analyzer = FileAnalyzer(r"D:\Data\Document\ProductsLibrary\C线产品资料库")

# 分析文件
results = analyzer.analyze_files(recursive=True)

# 查看汇总信息
analyzer.print_summary()

# 查看详细信息
analyzer.print_detailed_results()

# 导出结果
analyzer.export_to_csv()

# 筛选文件
filtered = analyzer.filter_files(
    projectname="KB04122",
    country="CN",
    version="1.0.0",
    date_start="240101",
    date_end="241231"
)
```

## 主要功能

### 1. 文件扫描与匹配
- 自动扫描指定文件夹下的所有 `.bin` 文件
- 使用正则表达式验证文件名格式
- 支持递归搜索子文件夹

### 2. 信息提取
从匹配的文件名中提取以下信息：
- 项目名称
- 国家代码
- 版本号
- 日期（转换为 YYYY-MM-DD 格式）
- 发布类型
- 文件大小

### 3. 数据筛选
支持按以下条件筛选文件：
- 项目名称
- 国家代码
- 版本号
- 日期范围

### 4. 结果导出
- 将结果导出为 CSV 格式
- 包含完整的文件信息
- 支持自定义输出文件名

### 5. 交互式操作
- 提供交互式筛选菜单
- 实时查看筛选结果
- 支持多种筛选条件组合

## 输出示例

### 控制台输出
```
============================================================
快速文件分析器
============================================================
📁 分析文件夹: D:\Data\Document\ProductsLibrary\C线产品资料库

🔍 正在分析文件...
✓ 匹配文件: KB04122_CN_1.0.0.240101_release.bin
✓ 匹配文件: KB04122_CN_1.1.0.240315_release.bin
✓ 匹配文件: BASEUS_US_2.0.0.240201_release.bin
...

分析完成！
总文件数: 150
匹配文件数: 120

📊 分析结果汇总:
==================================================
总文件数: 120
项目名称: KB04122, BASEUS, PRODUCT123
国家代码: CN, US, EU
版本号: 1.0.0, 1.1.0, 2.0.0
发布类型: release
日期范围: 240101 - 241231

各项目文件数量:
  BASEUS: 45 个文件
  KB04122: 50 个文件
  PRODUCT123: 25 个文件
```

### CSV 输出
生成的 CSV 文件包含以下列：
- filename - 文件名
- projectname - 项目名称
- country - 国家代码
- version - 版本号
- date - 原始日期（yymmdd）
- formatted_date - 格式化日期（YYYY-MM-DD）
- crc - CRC校验值（无CRC时为None）
- release - 发布类型
- file_size - 文件大小（字节）
- full_path - 完整文件路径

## 高级用法

### 自定义文件格式
如果需要分析其他格式的文件，可以修改 `file_analyzer.py` 中的正则表达式：

```python
self.file_pattern = re.compile(
    r'^(?P<projectname>[a-zA-Z0-9_]+)_'
    r'(?P<country>[a-zA-Z]+)_'
    r'(?P<version>\d+\.\d+\.\d+)\.'
    r'(?P<date>\d{6})_'
    r'(?P<release>\w+)\.bin$'
)
```

### 批量分析多个文件夹
```python
folders = [
    r"D:\Data\Folder1",
    r"D:\Data\Folder2",
    r"D:\Data\Folder3"
]

for folder in folders:
    print(f"\n分析文件夹: {folder}")
    analyzer = FileAnalyzer(folder)
    results = analyzer.analyze_files()
    analyzer.export_to_csv(f"results_{os.path.basename(folder)}.csv")
```

## 注意事项

1. **权限要求**：确保有权限访问目标文件夹
2. **文件格式**：文件名必须严格符合格式要求
3. **日期格式**：日期必须为 6 位数字（yymmdd 格式）
4. **Python 版本**：需要 Python 3.6 或更高版本

## 常见问题

**Q: 为什么有些文件没有被匹配？**
A: 请检查文件名是否符合格式要求：projectname_country_v.v.v.yymmdd_release.bin

**Q: 如何修改输出文件路径？**
A: 在 `export_to_csv()` 方法中指定 `output_path` 参数

**Q: 如何分析非 .bin 文件？**
A: 修改 `file_analyzer.py` 中的文件扩展名或使用通配符

**Q: 如何处理大量文件？**
A: 程序会自动处理，建议使用 CSV 导出功能进行后续分析

## 更新日志

- v1.0.0 (2024-12-29)
  - 初始版本
  - 支持基本文件分析和信息提取
  - 添加交互式筛选功能
  - 支持 CSV 导出