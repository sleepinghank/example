#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
实际使用示例
演示如何在实际项目中集成和使用文件分析器
"""

from file_analyzer import FileAnalyzer
import os


def analyze_project_files(project_path: str):
    """
    分析项目文件并生成报告

    Args:
        project_path: 项目文件夹路径
    """
    print(f"开始分析项目: {project_path}")

    # 创建分析器
    analyzer = FileAnalyzer(project_path)

    # 分析文件
    results = analyzer.analyze_files(recursive=True)

    if not results:
        print("未找到匹配的文件")
        return None

    # 获取统计信息
    summary = analyzer.get_summary()

    # 生成报告
    report = {
        'project_name': project_path,
        'total_files': summary['total_files'],
        'project_list': summary['projectnames'],
        'country_list': summary['countries'],
        'version_list': summary['versions'],
        'date_range': summary['date_range'],
    }

    return report, analyzer


def find_latest_versions(analyzer: FileAnalyzer):
    """
    查找每个项目的最新版本

    Args:
        analyzer: FileAnalyzer实例
    """
    print("\n查找每个项目的最新版本...")

    projects = {}
    for result in analyzer.results:
        project = result['projectname']
        version = result['version']
        date = result['date']

        if project not in projects:
            projects[project] = []

        projects[project].append({
            'version': version,
            'date': date,
            'filename': result['filename']
        })

    # 查找最新版本
    latest_versions = {}
    for project, versions in projects.items():
        # 按版本号排序
        versions.sort(key=lambda x: [int(n) for n in x['version'].split('.')])
        latest = versions[-1]
        latest_versions[project] = latest
        print(f"{project}: {latest['version']} ({latest['filename']})")

    return latest_versions


def find_old_files(analyzer: FileAnalyzer, before_date: str):
    """
    查找指定日期之前的文件

    Args:
        analyzer: FileAnalyzer实例
        before_date: 日期 (yymmdd格式)
    """
    print(f"\n查找 {before_date} 之前的文件...")

    old_files = analyzer.filter_files(date_end=before_date)

    if old_files:
        print(f"找到 {len(old_files)} 个旧文件:")
        for file in old_files:
            print(f"  - {file['filename']} ({file['formatted_date']})")
    else:
        print("未找到旧文件")

    return old_files


def compare_projects(analyzer: FileAnalyzer, project1: str, project2: str):
    """
    比较两个项目的文件数量和版本

    Args:
        analyzer: FileAnalyzer实例
        project1: 第一个项目名
        project2: 第二个项目名
    """
    print(f"\n比较项目 {project1} 和 {project2}:")

    files1 = analyzer.filter_files(projectname=project1)
    files2 = analyzer.filter_files(projectname=project2)

    print(f"{project1}: {len(files1)} 个文件")
    print(f"{project2}: {len(files2)} 个文件")

    # 比较版本
    versions1 = set(f['version'] for f in files1)
    versions2 = set(f['version'] for f in files2)

    print(f"\n{project1} 版本: {sorted(versions1)}")
    print(f"{project2} 版本: {sorted(versions2)}")

    common_versions = versions1 & versions2
    if common_versions:
        print(f"共同版本: {sorted(common_versions)}")


def main():
    """
    主函数 - 演示完整的工作流程
    """
    print("="*70)
    print("文件分析器实际使用示例")
    print("="*70)

    # 示例文件夹（请根据实际情况修改）
    folder_path = r"D:\Data\Document\ProductsLibrary\C线产品资料库"

    # 检查文件夹是否存在
    if not os.path.exists(folder_path):
        print(f"示例文件夹不存在: {folder_path}")
        print("\n使用演示模式...")

        # 运行演示
        import demo
        demo.run_demo()
        return

    try:
        # 1. 分析文件
        print("\n步骤 1: 分析文件")
        report, analyzer = analyze_project_files(folder_path)

        if report:
            # 2. 显示汇总
            print("\n步骤 2: 显示汇总")
            analyzer.print_summary()

            # 3. 查找最新版本
            print("\n步骤 3: 查找最新版本")
            latest = find_latest_versions(analyzer)

            # 4. 查找旧文件
            print("\n步骤 4: 查找旧文件")
            old_files = find_old_files(analyzer, "240601")

            # 5. 比较项目
            if len(report['project_list']) >= 2:
                print("\n步骤 5: 比较项目")
                compare_projects(analyzer, report['project_list'][0], report['project_list'][1])

            # 6. 导出结果
            print("\n步骤 6: 导出结果")
            analyzer.export_to_csv("project_analysis.csv")

            # 7. 生成自定义报告
            print("\n步骤 7: 生成自定义报告")
            with open("custom_report.txt", "w", encoding="utf-8") as f:
                f.write("项目文件分析报告\n")
                f.write("="*50 + "\n\n")
                f.write(f"分析文件夹: {folder_path}\n")
                f.write(f"总文件数: {report['total_files']}\n")
                f.write(f"项目列表: {', '.join(report['project_list'])}\n")
                f.write(f"国家列表: {', '.join(report['country_list'])}\n")
                f.write(f"版本列表: {', '.join(report['version_list'])}\n")
                f.write(f"日期范围: {report['date_range']['min']} - {report['date_range']['max']}\n\n")

                f.write("最新版本:\n")
                for project, info in latest.items():
                    f.write(f"  {project}: {info['version']} ({info['filename']})\n")

                if old_files:
                    f.write(f"\n{before_date} 之前的文件 ({len(old_files)} 个):\n")
                    for file in old_files:
                        f.write(f"  - {file['filename']} ({file['formatted_date']})\n")

            print("自定义报告已保存到: custom_report.txt")

    except Exception as e:
        print(f"\n[ERROR] 分析过程中出现错误: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()