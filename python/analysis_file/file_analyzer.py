#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
文件分析器 - 分析指定文件夹下特定格式的文件并提取信息
支持格式：projectname_country_v.v.v.yymmdd_release.bin
"""

import os
import re
import pandas as pd
from datetime import datetime
from pathlib import Path
from typing import List, Dict, Optional


class FileAnalyzer:
    """文件分析器类"""

    def __init__(self, folder_path: str):
        """
        初始化文件分析器

        Args:
            folder_path: 要分析的文件夹路径
        """
        self.folder_path = folder_path
        self.file_pattern = re.compile(
            r'^(?P<projectname>[a-zA-Z0-9_]+)_'
            r'(?P<country>[a-zA-Z]+)_'
            r'(?P<version>\d+\.\d+\.\d+)\.'
            r'(?P<date>\d{6})_'
            r'(?:(?P<crc>[A-F0-9]+)_)?'  # CRC字段可选
            r'(?P<release>beta|release)(?:_\w+)?\.bin$'
        )
        self.results = []

    def analyze_files(self, recursive: bool = True) -> List[Dict]:
        """
        分析文件夹中的文件

        Args:
            recursive: 是否递归搜索子文件夹

        Returns:
            包含分析结果的字典列表
        """
        print(f"开始分析文件夹: {self.folder_path}")

        if not os.path.exists(self.folder_path):
            raise FileNotFoundError(f"文件夹不存在: {self.folder_path}")

        # 遍历文件夹
        if recursive:
            files = Path(self.folder_path).rglob("*.bin")
        else:
            files = Path(self.folder_path).glob("*.bin")

        matched_files = []
        total_files = 0

        for file_path in files:
            total_files += 1
            filename = file_path.name
            match = self.file_pattern.match(filename)

            if match:
                result = match.groupdict()
                result['filename'] = filename
                result['full_path'] = str(file_path)
                result['file_size'] = file_path.stat().st_size if file_path.exists() else 0

                # 转换日期格式
                try:
                    date_str = result['date']
                    formatted_date = f"20{date_str[4:6]}-{date_str[2:4]}-{date_str[0:2]}"
                    result['formatted_date'] = formatted_date
                except:
                    result['formatted_date'] = result['date']

                matched_files.append(result)
                print(f"[OK] 匹配文件: {filename}")
            else:
                print(f"[SKIP] 格式不匹配: {filename}")

        self.results = matched_files
        print(f"\n分析完成！")
        print(f"总文件数: {total_files}")
        print(f"匹配文件数: {len(matched_files)}")

        return matched_files

    def get_summary(self) -> Dict:
        """
        获取分析结果汇总

        Returns:
            汇总统计信息
        """
        if not self.results:
            return {}

        summary = {
            'total_files': len(self.results),
            'projectnames': list(set(r['projectname'] for r in self.results)),
            'countries': list(set(r['country'] for r in self.results)),
            'versions': list(set(r['version'] for r in self.results)),
            'release_types': list(set(r['release'] for r in self.results)),
            'date_range': {
                'min': min(r['date'] for r in self.results),
                'max': max(r['date'] for r in self.results)
            }
        }

        return summary

    def filter_files(self,
                    projectname: Optional[str] = None,
                    country: Optional[str] = None,
                    version: Optional[str] = None,
                    date_start: Optional[str] = None,
                    date_end: Optional[str] = None) -> List[Dict]:
        """
        根据条件筛选文件

        Args:
            projectname: 项目名称
            country: 国家代码
            version: 版本号
            date_start: 开始日期 (yymmdd格式)
            date_end: 结束日期 (yymmdd格式)

        Returns:
            筛选后的文件列表
        """
        filtered = self.results.copy()

        if projectname:
            filtered = [r for r in filtered if r['projectname'] == projectname]

        if country:
            filtered = [r for r in filtered if r['country'] == country]

        if version:
            filtered = [r for r in filtered if r['version'] == version]

        if date_start:
            filtered = [r for r in filtered if r['date'] >= date_start]

        if date_end:
            filtered = [r for r in filtered if r['date'] <= date_end]

        return filtered

    def export_to_csv(self, output_path: str = None, filtered_results: List[Dict] = None):
        """
        导出结果到CSV文件

        Args:
            output_path: 输出文件路径
            filtered_results: 要导出的结果（如果为None则导出所有结果）
        """
        if filtered_results is None:
            filtered_results = self.results

        if not filtered_results:
            print("没有数据可导出")
            return

        # 转换为DataFrame
        df = pd.DataFrame(filtered_results)

        # 重新排列列顺序
        columns = ['filename', 'projectname', 'country', 'version', 'date', 'formatted_date', 'crc', 'release', 'file_size', 'full_path']
        df = df[columns]

        if output_path is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_path = f"file_analysis_results_{timestamp}.csv"

        df.to_csv(output_path, index=False, encoding='utf-8-sig')
        print(f"结果已导出到: {output_path}")

    def print_summary(self):
        """打印汇总信息"""
        summary = self.get_summary()

        print("\n" + "="*50)
        print("文件分析汇总")
        print("="*50)
        print(f"总文件数: {summary['total_files']}")
        print(f"项目名称: {', '.join(sorted(summary['projectnames']))}")
        print(f"国家代码: {', '.join(sorted(summary['countries']))}")
        print(f"版本号: {', '.join(sorted(summary['versions']))}")
        print(f"发布类型: {', '.join(sorted(summary['release_types']))}")
        print(f"日期范围: {summary['date_range']['min']} - {summary['date_range']['max']}")

        # 按项目统计
        project_stats = {}
        for result in self.results:
            project = result['projectname']
            if project not in project_stats:
                project_stats[project] = 0
            project_stats[project] += 1

        print(f"\n各项目文件数量:")
        for project, count in sorted(project_stats.items()):
            print(f"  {project}: {count} 个文件")

    def print_detailed_results(self, filtered_results: List[Dict] = None):
        """
        打印详细结果

        Args:
            filtered_results: 要显示的结果（如果为None则显示所有结果）
        """
        if filtered_results is None:
            filtered_results = self.results

        if not filtered_results:
            print("没有匹配的文件")
            return

        print(f"\n详细结果 (共 {len(filtered_results)} 个文件):")
        print("-" * 95)
        print(f"{'文件名':<40} {'项目':<12} {'国家':<8} {'版本':<10} {'日期':<8} {'CRC':<8} {'发布类型':<10}")
        print("-" * 95)

        for result in filtered_results:
            crc = result.get('crc', 'N/A')
            print(f"{result['filename']:<40} "
                  f"{result['projectname']:<12} "
                  f"{result['country']:<8} "
                  f"{result['version']:<10} "
                  f"{result['formatted_date']:<8} "
                  f"{crc:<8} "
                  f"{result['release']:<10}")


def main():
    """主函数 - 演示用法"""
    # 示例用法
    folder_path = r"D:\Data\Document\ProductsLibrary\C线产品资料库"

    # 创建分析器实例
    analyzer = FileAnalyzer(folder_path)

    try:
        # 分析文件
        results = analyzer.analyze_files(recursive=True)

        # 显示汇总信息
        analyzer.print_summary()

        # 显示详细结果
        analyzer.print_detailed_results()

        # 导出到CSV
        analyzer.export_to_csv()

        # 示例筛选操作
        print("\n" + "="*50)
        print("筛选示例")
        print("="*50)

        # 筛选特定项目的文件
        if results:
            first_project = results[0]['projectname']
            filtered = analyzer.filter_files(projectname=first_project)
            print(f"\n项目 '{first_project}' 的文件 ({len(filtered)} 个):")
            analyzer.print_detailed_results(filtered)

            # 导出筛选结果
            if filtered:
                analyzer.export_to_csv(f"{first_project}_files.csv", filtered)

    except Exception as e:
        print(f"分析过程中出现错误: {e}")
        print("\n请检查:")
        print("1. 文件夹路径是否正确")
        print("2. 是否有权限访问该文件夹")
        print("3. 文件夹中是否存在 .bin 文件")


if __name__ == "__main__":
    main()