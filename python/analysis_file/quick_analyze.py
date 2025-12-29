#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
快速文件分析脚本
使用方法：直接运行此脚本或修改folder_path变量为目标文件夹
"""

from file_analyzer import FileAnalyzer
import os


def quick_analyze(folder_path: str):
    """
    快速分析指定文件夹

    Args:
        folder_path: 要分析的文件夹路径
    """
    print("="*60)
    print("快速文件分析器")
    print("="*60)

    # 检查文件夹是否存在
    if not os.path.exists(folder_path):
        print(f"[ERROR] 错误：文件夹不存在 - {folder_path}")
        print("\n请检查路径是否正确，或修改此脚本中的folder_path变量")
        return

    print(f"[INFO] 分析文件夹: {folder_path}")

    # 创建分析器
    analyzer = FileAnalyzer(folder_path)

    try:
        # 分析文件
        print("\n[SCAN] 正在分析文件...")
        results = analyzer.analyze_files(recursive=True)

        if results:
            # 显示汇总
            print("\n[SUMMARY] 分析结果汇总:")
            analyzer.print_summary()

            # 显示前10个文件的详细信息
            print("\n[DETAIL] 文件详细信息 (前10个):")
            analyzer.print_detailed_results(results[:10])

            # 导出结果
            print("\n[EXPORT] 导出分析结果...")
            analyzer.export_to_csv()

            print("\n[DONE] 分析完成！")
            print(f"[FILE] 详细结果已保存到 CSV 文件")

            # 交互式筛选
            interactive_filter(analyzer, results)

        else:
            print("\n[WARNING] 未找到匹配的文件")
            print("请检查:")
            print("1. 文件夹中是否有 .bin 文件")
            print("2. 文件名是否符合格式：projectname_country_v.v.v.yymmdd_release.bin")

    except Exception as e:
        print(f"\n[ERROR] 分析过程中出现错误: {e}")


def interactive_filter(analyzer: FileAnalyzer, results):
    """
    交互式筛选功能

    Args:
        analyzer: FileAnalyzer实例
        results: 分析结果
    """
    print("\n" + "="*60)
    print("交互式筛选")
    print("="*60)

    while True:
        print("\n可用的筛选选项:")
        print("1. 按项目名称筛选")
        print("2. 按国家代码筛选")
        print("3. 按版本号筛选")
        print("4. 按日期范围筛选")
        print("5. 显示所有结果")
        print("0. 退出")

        try:
            choice = input("\n请选择操作 (0-5): ").strip()

            if choice == '0':
                print("退出筛选")
                break
            elif choice == '1':
                projectname = input("请输入项目名称: ").strip()
                if projectname:
                    filtered = analyzer.filter_files(projectname=projectname)
                    print(f"\n找到 {len(filtered)} 个匹配的文件:")
                    analyzer.print_detailed_results(filtered)

            elif choice == '2':
                country = input("请输入国家代码: ").strip().upper()
                if country:
                    filtered = analyzer.filter_files(country=country)
                    print(f"\n找到 {len(filtered)} 个匹配的文件:")
                    analyzer.print_detailed_results(filtered)

            elif choice == '3':
                version = input("请输入版本号 (如 1.2.3): ").strip()
                if version:
                    filtered = analyzer.filter_files(version=version)
                    print(f"\n找到 {len(filtered)} 个匹配的文件:")
                    analyzer.print_detailed_results(filtered)

            elif choice == '4':
                date_start = input("请输入开始日期 (yymmdd，如 240101): ").strip()
                date_end = input("请输入结束日期 (yymmdd，如 241231): ").strip()
                if date_start and date_end:
                    filtered = analyzer.filter_files(date_start=date_start, date_end=date_end)
                    print(f"\n找到 {len(filtered)} 个匹配的文件:")
                    analyzer.print_detailed_results(filtered)

            elif choice == '5':
                print(f"\n所有文件 ({len(results)} 个):")
                analyzer.print_detailed_results(results)

            else:
                print("[ERROR] 无效选择，请重新输入")

        except KeyboardInterrupt:
            print("\n\n退出筛选")
            break
        except Exception as e:
            print(f"[ERROR] 操作出错: {e}")


if __name__ == "__main__":
    # 修改此路径为目标文件夹
    folder_path = r"D:\Data\Document\ProductsLibrary\C线产品资料库"

    # 运行快速分析
    quick_analyze(folder_path)