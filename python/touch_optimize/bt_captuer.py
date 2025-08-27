import os
import re
import csv
from datetime import datetime

def filter_bluetooth_logs(input_file, output_file):
    """
    筛选蓝牙日志文件，只保留包含 Handle:0x003D 的行
    
    Args:
        input_file (str): 输入的日志文件路径
        output_file (str): 输出的筛选结果文件路径
    """
    # 匹配包含 Handle:0x003D 的行的正则表达式
    pattern = re.compile(r'.*Handle:0x003D.*')
    
    with open(input_file, 'r', encoding='utf-8') as infile, \
         open(output_file, 'w', encoding='utf-8') as outfile:
        
        line_count = 0
        matched_count = 0
        
        for line in infile:
            line_count += 1
            # 检查行是否匹配模式
            if pattern.search(line):
                outfile.write(line)
                matched_count += 1
                
        print(f"处理完成: 总共 {line_count} 行，匹配 {matched_count} 行")

def extract_and_analyze_timestamps(input_file):
    """
    提取包含 Handle:0x003D 的行的时间戳并进行分析
    
    Args:
        input_file (str): 输入的日志文件路径
        
    Returns:
        dict: 包含分析结果的字典
    """
    # 匹配包含 Handle:0x003D 的行的正则表达式
    handle_pattern = re.compile(r'.*Handle:0x003D.*')
    # 匹配时间戳的正则表达式 (例如: Aug 26 19:40:35.407)
    timestamp_pattern = re.compile(r'([A-Za-z]{3}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2}\.\d{3})')
    
    timestamps = []
    
    with open(input_file, 'r', encoding='utf-8') as infile:
        for line in infile:
            # 检查行是否包含 Handle:0x003D
            if handle_pattern.search(line):
                # 提取时间戳
                timestamp_match = timestamp_pattern.search(line)
                if timestamp_match:
                    timestamp_str = timestamp_match.group(1)
                    # 解析时间戳字符串为 datetime 对象
                    # 假设都是同一年，使用当前年份
                    current_year = datetime.now().year
                    try:
                        # 处理类似 "Aug 26 19:40:35.407" 的格式
                        month_day_time = timestamp_str
                        timestamp_obj = datetime.strptime(f"{current_year} {month_day_time}", 
                                                         "%Y %b %d %H:%M:%S.%f")
                        timestamps.append(timestamp_obj)
                    except ValueError:
                        print(f"无法解析时间戳: {timestamp_str}")
    
    # 对时间戳进行排序
    timestamps.sort()
    # 用后一个时间戳减去前一个时间戳，单位为ms
    intervals = [(timestamps[i] - timestamps[i-1]).total_seconds() * 1000 for i in range(1, len(timestamps))]
    # 统计intervals中为0的数量
    zero_intervals = sum(1 for interval in intervals if interval == 0)
    print(f"间隔为0的数量: {zero_intervals}")
    # 进行分析
    analysis_result = analyze_timestamps(timestamps)
    
    # 添加文件信息
    analysis_result['file_name'] = os.path.basename(input_file)
    analysis_result['total_packets'] = len(timestamps)
    
    return analysis_result

def analyze_timestamps(timestamps):
    """
    分析时间戳数据，计算帧率、丢包、上报间隔等指标
    
    Args:
        timestamps (list): datetime 对象列表
        
    Returns:
        dict: 分析结果
    """
    if not timestamps:
        return {
            'frame_rate': 0,
            'avg_interval': 0,
            'min_interval': 0,
            'max_interval': 0,
            'total_duration': 0,
            'expected_packets': 0,
            'actual_packets': 0,
            'packet_loss_rate': 0
        }
    
    # 计算相邻时间间隔
    intervals = []
    for i in range(1, len(timestamps)):
        interval = (timestamps[i] - timestamps[i-1]).total_seconds()
        intervals.append(interval)
    
    # 基本统计信息
    total_duration = (timestamps[-1] - timestamps[0]).total_seconds()
    actual_packets = len(timestamps)
    
    # 帧率计算（每秒帧数）
    frame_rate = 0
    if total_duration > 0:
        frame_rate = actual_packets / total_duration
    
    # 间隔统计
    avg_interval = 0
    min_interval = 0
    max_interval = 0
    if intervals:
        avg_interval = sum(intervals) / len(intervals)
        min_interval = min(intervals)
        max_interval = max(intervals)

    # 计算标准差
    stddev_interval = 0
    if intervals:
        # 计算每个间隔与平均值的平方差
        squared_diffs = [(x - avg_interval) ** 2 for x in intervals]
        # 计算标准差
        stddev_interval = (sum(squared_diffs) / len(intervals)) ** 0.5

    # 丢包分析（假设理想情况下应该是固定间隔）
    expected_packets = 1475  # 默认情况下期望包数等于实际包数
    packet_loss_rate = 0
    
    # 如果有目标帧率，可以进一步分析丢包
    # 这里假设目标是80Hz（根据文件名中的"80Hz"）
    target_fps = 80
    if total_duration > 0:
        # expected_packets = int(total_duration * target_fps)
        if expected_packets > 0:
            packet_loss_rate = (expected_packets - actual_packets) / expected_packets * 100
    
    return {
        'frame_rate': frame_rate,
        'avg_interval': avg_interval,
        'min_interval': min_interval,
        'max_interval': max_interval,
        'total_duration': total_duration,
        'stddev_interval': stddev_interval,
        'expected_packets': expected_packets,
        'actual_packets': actual_packets,
        'packet_loss_rate': packet_loss_rate
    }

def batch_analyze_logs(input_directory, output_directory):
    """
    批量分析目录中的所有蓝牙日志文件并将结果保存到CSV文件
    
    Args:
        input_directory (str): 包含原始日志文件的目录
        output_directory (str): 存放结果文件的目录
    """
    # 确保输出目录存在
    os.makedirs(output_directory, exist_ok=True)
    
    results = []
    
    # 遍历输入目录中的所有.txt文件
    for filename in os.listdir(input_directory):
        if filename.endswith('.txt'):
            input_file = os.path.join(input_directory, filename)
            
            print(f"正在分析文件: {filename}")
            analysis_result = extract_and_analyze_timestamps(input_file)
            results.append(analysis_result)
            print(f"分析完成: {filename}\n")
    
    # 保存结果到CSV文件
    csv_file = os.path.join(output_directory, 'analysis_results.csv')
    save_results_to_csv(results, csv_file)
    print(f"分析结果已保存到: {csv_file}")
    
    return results

def save_results_to_csv(results, csv_file):
    """
    将分析结果保存到CSV文件
    
    Args:
        results (list): 分析结果列表
        csv_file (str): CSV文件路径
    """
    # 定义CSV列名
    fieldnames = [
        '文件名',
        '总包数',
        '持续时间(秒)',
        '平均帧率(FPS)',
        '平均间隔(ms)',
        '最小间隔(ms)',
        '最大间隔(ms)',
        '标准差(ms)',
        '期望包数',
        '丢包率(%)'
    ]
    
    with open(csv_file, 'w', newline='', encoding='utf-8-sig') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        
        for result in results:
            writer.writerow({
                '文件名': result['file_name'],
                '总包数': result['actual_packets'],
                '持续时间(秒)': f"{result['total_duration']:.2f}",
                '平均帧率(FPS)': f"{result['frame_rate']:.2f}",
                '平均间隔(ms)': f"{result['avg_interval']*1000:.2f}",
                '最小间隔(ms)': f"{result['min_interval']*1000:.2f}",
                '最大间隔(ms)': f"{result['max_interval']*1000:.2f}",
                '标准差(ms)': f"{result['stddev_interval']*1000:.2f}",
                '期望包数': result['expected_packets'],
                '丢包率(%)': f"{result['packet_loss_rate']:.2f}"
            })

def batch_filter_logs(input_directory, output_directory):
    """
    批量处理目录中的所有蓝牙日志文件
    
    Args:
        input_directory (str): 包含原始日志文件的目录
        output_directory (str): 存放筛选结果的目录
    """
    # 确保输出目录存在
    os.makedirs(output_directory, exist_ok=True)
    
    # 遍历输入目录中的所有.txt文件
    for filename in os.listdir(input_directory):
        if filename.endswith('.txt'):
            input_file = os.path.join(input_directory, filename)
            # 修改输出文件名，添加 _filter 后缀
            name, ext = os.path.splitext(filename)
            output_filename = f"{name}_filter{ext}"
            output_file = os.path.join(output_directory, output_filename)
            
            print(f"正在处理文件: {filename}")
            filter_bluetooth_logs(input_file, output_file)
            print(f"结果已保存到: {output_file}\n")

# 使用示例
if __name__ == "__main__":
    # 创建结果目录
    result_dir = r"d:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval\result"
    os.makedirs(result_dir, exist_ok=True)

    # 单个文件处理示例
    # input_file = r"d:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval\KB04122-主板固定60Hz发送1.txt"
    # # 修改输出文件名，基于原始文件名添加 _filter 后缀
    # input_filename = os.path.basename(input_file)
    # name, ext = os.path.splitext(input_filename)
    # output_file = os.path.join(
    #     os.path.dirname(input_file),
    #     f"{name}_filter{ext}"
    # )
    # filter_bluetooth_logs(input_file, output_file)


    #
    # # 分析单个文件并将结果保存到CSV
    # print("正在分析单个文件...")
    # analysis_result = extract_and_analyze_timestamps(input_file)
    #
    # # 保存单个文件的分析结果到CSV
    # single_result_file = os.path.join(result_dir, 'single_file_analysis.csv')
    # save_results_to_csv([analysis_result], single_result_file)
    # print(f"单文件分析结果已保存到: {single_result_file}")
    
    # 批量分析整个目录
    input_dir = r"d:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval"
    print("正在批量分析目录...")
    batch_results = batch_analyze_logs(input_dir, result_dir)