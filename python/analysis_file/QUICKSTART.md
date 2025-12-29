# 快速使用指南

## 快速开始

### 1. 安装依赖
```bash
pip install pandas
```

### 2. 使用快速分析脚本
编辑 `quick_analyze.py` 文件中的路径：
```python
folder_path = r"D:\Data\Document\ProductsLibrary\C线产品资料库"
```

然后运行：
```bash
python quick_analyze.py
```

### 3. 查看结果
- 控制台会显示分析汇总和详细信息
- 自动生成 CSV 文件包含所有结果

## 主要文件说明

| 文件 | 用途 |
|------|------|
| `file_analyzer.py` | 核心分析器类，包含所有分析功能 |
| `quick_analyze.py` | 快速分析脚本，适合直接使用 |
| `demo.py` | 演示脚本，展示如何使用 |
| `README.md` | 详细文档 |
| `requirements.txt` | 依赖包列表 |

## 支持的文件格式

### 无CRC格式
```
projectname_country_v.v.v.yymmdd_release.bin
```
例如：`KB04122_CN_1.0.0.240101_release.bin`

### 有CRC格式
```
projectname_country_v.v.v.yymmdd_CRC_release.bin
```
例如：`KB903_DE_0.4.3.250915_A4A2_release_Open.bin`

**说明**：
- CRC字段为可选（6位十六进制数，如A4A2、ABCD12等）
- release字段仅支持"beta"或"release"，会自动忽略后缀：
  - `release_Encrypt.bin` → release类型
  - `beta_Encrypt.bin` → beta类型

## 主要功能

✅ 自动扫描文件夹下的所有 .bin 文件
✅ 提取项目名称、国家代码、版本号、日期等信息
✅ 支持按多种条件筛选文件
✅ 生成汇总统计报告
✅ 导出为 CSV 格式便于后续分析

## 常见用法示例

### 基本分析
```python
from file_analyzer import FileAnalyzer

analyzer = FileAnalyzer(r"D:\Data\Documents")
results = analyzer.analyze_files()
analyzer.print_summary()
```

### 筛选特定项目
```python
# 筛选 KB04122 项目的所有文件
kb_files = analyzer.filter_files(projectname="KB04122")

# 筛选中国地区的文件
cn_files = analyzer.filter_files(country="CN")

# 筛选特定版本
v100_files = analyzer.filter_files(version="1.0.0")

# 筛选日期范围
q1_files = analyzer.filter_files(date_start="240101", date_end="240331")
```

### 导出结果
```python
# 导出所有结果
analyzer.export_to_csv("all_results.csv")

# 导出筛选结果
filtered = analyzer.filter_files(projectname="KB04122")
analyzer.export_to_csv("kb04122_files.csv", filtered)
```

## 运行演示
```bash
python demo.py
```

## 注意事项

1. 确保有权限访问目标文件夹
2. 文件名必须严格符合格式要求
3. 日期格式为 6 位数字（yymmdd）
4. 需要 Python 3.6+