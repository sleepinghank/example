import os
import re
import csv
from datetime import datetime, timezone, timedelta

import pandas as pd
import pytz


def extract_scan_time_from_line(line):
    """
    从蓝牙日志行中提取scan_time
    
    Args:
        line (str): 包含蓝牙数据的日志行
        
    Returns:
        int or None: scan_time值，如果无法提取则返回None
    """
    # 匹配包含 Handle:0x003D 和 Value: 的行
    handle_pattern = re.compile(r'.*Handle:0x003D.*Value:\s*([0-9A-Fa-f\s]+)')
    
    match = handle_pattern.search(line)
    if match:
        value_hex = match.group(1).strip()
        # 提取前两个字节（4个十六进制字符）
        if len(value_hex) >= 4:
            # 前两个字节，注意字节序（小端序）
            scan_time_hex = value_hex[:4]
            try:
                # 转换为整数，注意字节序
                scan_time = int(scan_time_hex[2:4] + scan_time_hex[0:2], 16)
                return scan_time
            except ValueError:
                print(f"无法解析scan_time十六进制值: {scan_time_hex}")
                return None
    return None

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

def extractAndAnalyzeTimestampsAndScanTime(input_file):
    """
    提取包含 Handle:0x003D 的行的时间戳和scan_time并进行分析
    
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
    scan_times = []
    
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
                        
                        # 提取scan_time
                        scan_time = extract_scan_time_from_line(line)
                        if scan_time is not None:
                            scan_times.append(scan_time)
                        else:
                            scan_times.append(None)
                            
                    except ValueError:
                        print(f"无法解析时间戳: {timestamp_str}")
                        scan_times.append(None)
    
    # for i in range(len(timestamps)):
    #     print(f"时间戳: {timestamps[i]}, scan_time: {scan_times[i]}")

    # 对时间戳进行排序
    timestamps.sort()
    
    # 分析scan_time
    scan_time_analysis = analyze_scan_times(scan_times)
    
    # 分析时间戳进行分析
    analysis_result = analyze_timestamps(timestamps)
    
    # 计算scan_time与理论值的差异（延迟）
    # 默认以60Hz为基准上报，每帧间隔时间为16.67ms
    theoretical_delay_analysis = calculate_theoretical_delays(timestamps)
    
    # 合并分析结果
    analysis_result.update(scan_time_analysis)
    analysis_result.update(theoretical_delay_analysis)
    
    # 添加文件信息
    analysis_result['file_name'] = os.path.basename(input_file)
    analysis_result['total_packets'] = len(timestamps)
    
    return analysis_result

def calculate_theoretical_delays(scan_times):
    """
    计算scan_time与理论值的差异，即延迟
    
    Args:
        scan_times (list): scan_time值列表
        
    Returns:
        dict: 延迟分析结果
    """
    if not scan_times:
        return {
            'avg_theoretical_delay': 0,
            'min_theoretical_delay': 0,
            'max_theoretical_delay': 0,
            'std_theoretical_delay': 0
        }
    
    # 过滤掉None值
    valid_scan_times = [st for st in scan_times if st is not None]
    
    if len(valid_scan_times) < 2:
        return {
            'avg_theoretical_delay': 0,
            'min_theoretical_delay': 0,
            'max_theoretical_delay': 0,
            'std_theoretical_delay': 0
        }
    
    # 以第一帧为基准，计算理论延迟
    # 默认以60Hz为基准上报，每帧间隔时间为16.67ms
    frame_interval = 1000.0 / 60.0  # 16.67ms
    
    first_scan_time = valid_scan_times[0]
    theoretical_delays = []
    
    for i, scan_time in enumerate(valid_scan_times):
        # 计算理论scan_time值（基于60Hz帧率）
        theoretical_scan_time = i * frame_interval
        # 当前时间减去第一帧的时间并转换为ms
        actual_delay = (scan_time - first_scan_time).total_seconds() * 1000

        print(f"第{i}帧: actual_delay={actual_delay}, 理论scan_time={theoretical_scan_time}")
        # 计算实际scan_time与理论值的差异
        delay = actual_delay - theoretical_scan_time
        theoretical_delays.append(delay)

    theoretical_delays = [x + abs(min(theoretical_delays)) for x in theoretical_delays]

    # 计算延迟统计信息
    avg_delay = sum(theoretical_delays) / len(theoretical_delays) if theoretical_delays else 0
    min_delay = min(theoretical_delays) if theoretical_delays else 0
    max_delay = max(theoretical_delays) if theoretical_delays else 0
    
    # 计算标准差
    if theoretical_delays:
        variance = sum((x - avg_delay) ** 2 for x in theoretical_delays) / len(theoretical_delays)
        std_delay = variance ** 0.5
    else:
        std_delay = 0
    
    return {
        'avg_theoretical_delay': avg_delay,
        'min_theoretical_delay': min_delay,
        'max_theoretical_delay': max_delay,
        'std_theoretical_delay': std_delay
    }

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
    
    # 将intervals另存为ｃｓｖ
    # 将intervals转化为ｍｓ
    intervals_t = [x * 1000 for x in intervals]
    intervals_df = pd.DataFrame(intervals_t, columns=['interval'])
    intervals_df.to_csv('intervals.csv', index=False)
    print(f"已保存时间间隔数据至intervals.csv")

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
    min_interval_count = 0
    max_interval_count = 0
    if intervals:
        avg_interval = sum(intervals) / len(intervals)
        min_interval = min(intervals)
        max_interval = max(intervals)
        # 最小间隔次数
        min_interval_count = sum(1 for x in intervals if x == min_interval)
        # 最大间隔次数
        max_interval_count = sum(1 for x in intervals if x == max_interval)

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
        'packet_loss_rate': packet_loss_rate,
        'min_interval_count': min_interval_count,
        'max_interval_count': max_interval_count
    }

def analyze_scan_times(scan_times):
    """
    分析scan_time数据，计算递增规律、重置次数等指标
    
    Args:
        scan_times (list): scan_time值列表
        
    Returns:
        dict: scan_time分析结果
    """
    if not scan_times or all(st is None for st in scan_times):
        return {
            'valid_scan_times': 0,
            'avg_increment': 0,
            'reset_count': 0,
            'max_scan_time': 0,
            'min_scan_time': 0,
            'scan_time_consistency': 0
        }
    
    # 过滤掉None值
    valid_scan_times = [st for st in scan_times if st is not None]
    
    if len(valid_scan_times) < 2:
        return {
            'valid_scan_times': len(valid_scan_times),
            'avg_increment': 0,
            'reset_count': 0,
            'max_scan_time': max(valid_scan_times) if valid_scan_times else 0,
            'min_scan_time': min(valid_scan_times) if valid_scan_times else 0,
            'scan_time_consistency': 0
        }
    
    # 计算递增值
    increments = []
    reset_count = 0
    
    for i in range(1, len(valid_scan_times)):
        current = valid_scan_times[i]
        previous = valid_scan_times[i-1]
        
        if current < previous:
            # 检测到重置（从20000以上重置到0）
            reset_count += 1
            increment = current + (20000 - previous)
        else:
            increment = current - previous
        
        increments.append(increment)
    
    # 计算平均递增值
    avg_increment = sum(increments) / len(increments) if increments else 0
    
    # 计算一致性（标准差的倒数，值越大越一致）
    if increments:
        variance = sum((x - avg_increment) ** 2 for x in increments) / len(increments)
        stddev = variance ** 0.5
        scan_time_consistency = 1 / (1 + stddev) if stddev > 0 else 1
    else:
        scan_time_consistency = 0
    

    # 计算延迟，即scan_time与理论值的差异。默认以60Hz为基准上报，每帧间隔时间为16.67ms。从scna_time 第一帧为基准，计算每帧的理论scan_time值与实际值的差，即为理论延迟

    return {
        'valid_scan_times': len(valid_scan_times),
        'avg_increment': avg_increment,
        'reset_count': reset_count,
        'max_scan_time': max(valid_scan_times),
        'min_scan_time': min(valid_scan_times),
        'scan_time_consistency': scan_time_consistency
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
            analysis_result = extractAndAnalyzeTimestampsAndScanTime(input_file)
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
        '最小间隔次数',
        '最大间隔(ms)',
        '最大间隔次数',
        '标准差(ms)',
        '期望包数',
        '丢包率(%)',
        '有效scan_time数量',
        '平均递增值',
        '重置次数',
        '最大scan_time',
        '最小scan_time',
        'scan_time一致性',
        '平均理论延迟(ms)',
        '最小理论延迟(ms)',
        '最大理论延迟(ms)',
        '理论延迟标准差(ms)'
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
                '最小间隔(ms)': f"{result['min_interval'] * 1000:.2f}",
                '最小间隔次数': result['min_interval_count'],
                '最大间隔(ms)': f"{result['max_interval']*1000:.2f}",
                '最大间隔次数': result['max_interval_count'],
                '标准差(ms)': f"{result['stddev_interval']*1000:.2f}",
                '期望包数': result['expected_packets'],
                '丢包率(%)': f"{result['packet_loss_rate']:.2f}",
                '有效scan_time数量': result['valid_scan_times'],
                '平均递增值': f"{result['avg_increment']:.2f}",
                '重置次数': result['reset_count'],
                '最大scan_time': result['max_scan_time'],
                '最小scan_time': result['min_scan_time'],
                'scan_time一致性': f"{result['scan_time_consistency']:.2f}",
                '平均理论延迟(ms)': f"{result.get('avg_theoretical_delay', 0):.2f}",
                '最小理论延迟(ms)': f"{result.get('min_theoretical_delay', 0):.2f}",
                '最大理论延迟(ms)': f"{result.get('max_theoretical_delay', 0):.2f}",
                '理论延迟标准差(ms)': f"{result.get('std_theoretical_delay', 0):.2f}"
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

def generate_scan_time_report_simple(input_file, output_directory):
    """
    生成简化的scan_time分析报告（不依赖matplotlib）
    
    Args:
        input_file (str): 输入的日志文件路径
        output_directory (str): 输出目录路径
    """
    # 确保输出目录存在
    os.makedirs(output_directory, exist_ok=True)
    
    # 提取数据
    handle_pattern = re.compile(r'.*Handle:0x003D.*')
    timestamp_pattern = re.compile(r'([A-Za-z]{3}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2}\.\d{3})')
    
    timestamps = []
    scan_times = []
    
    with open(input_file, 'r', encoding='utf-8') as infile:
        for line in infile:
            if handle_pattern.search(line):
                timestamp_match = timestamp_pattern.search(line)
                if timestamp_match:
                    timestamp_str = timestamp_match.group(1)
                    current_year = datetime.now().year
                    try:
                        month_day_time = timestamp_str
                        timestamp_obj = datetime.strptime(f"{current_year} {month_day_time}", 
                                                         "%Y %b %d %H:%M:%S.%f")
                        timestamps.append(timestamp_obj)
                        
                        scan_time = extract_scan_time_from_line(line)
                        scan_times.append(scan_time)
                    except ValueError:
                        print(f"无法解析时间戳: {timestamp_str}")
                        scan_times.append(None)

    # 过滤有效数据
    valid_data = [(ts, st) for ts, st in zip(timestamps, scan_times) if st is not None]
    if not valid_data:
        print("没有找到有效的scan_time数据")
        return
    
    timestamps, scan_times = zip(*valid_data)
    
    # 计算递增值
    increments = []
    ts_increments = []
    reset_points = []
    for i in range(1, len(scan_times)):
        current = scan_times[i]
        previous = scan_times[i-1]
        
        if current < previous:
            # 检测到重置
            reset_points.append(i)
            increment = current + (20000 - previous) + 100
        else:
            increment = current - previous
        
        increments.append(increment)

    ts_increments = [(timestamps[i] - timestamps[0]).total_seconds() * 1000 for i in range(0, len(timestamps))]

    for i in range(len(increments)):
        # 计算时间,保留两位小数
        cal_time = round(i * 12.5, 2)
        # print(f"时间戳: {cal_time}, scan_time: {ts_increments[i]},diff: {round(ts_increments[i] - cal_time,2)}")

    # 计算统计信息
    avg_increment = sum(increments) / len(increments) if increments else 0
    min_increment = min(increments) if increments else 0
    max_increment = max(increments) if increments else 0
    
    # 计算标准差
    if increments:
        variance = sum((x - avg_increment) ** 2 for x in increments) / len(increments)
        stddev = variance ** 0.5
        consistency_score = 1 / (1 + stddev) if stddev > 0 else 1
    else:
        stddev = 0
        consistency_score = 0
    
    # 生成文本报告
    report_filename = os.path.splitext(os.path.basename(input_file))[0] + '_scan_time_report.txt'
    report_path = os.path.join(output_directory, report_filename)
    
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(f"scan_time分析报告 - {os.path.basename(input_file)}\n")
        f.write("=" * 50 + "\n\n")
        
        f.write("基本统计信息:\n")
        f.write(f"  总数据包数: {len(scan_times)}\n")
        f.write(f"  有效scan_time数: {len([st for st in scan_times if st is not None])}\n")
        f.write(f"  最大scan_time值: {max(scan_times)}\n")
        f.write(f"  最小scan_time值: {min(scan_times)}\n")
        f.write(f"  scan_time范围: {max(scan_times) - min(scan_times)}\n\n")
        
        f.write("递增规律分析:\n")
        f.write(f"  理论递增值: 150\n")
        f.write(f"  实际平均递增值: {avg_increment:.2f}\n")
        f.write(f"  递增值标准差: {stddev:.2f}\n")
        f.write(f"  最小递增值: {min_increment}\n")
        f.write(f"  最大递增值: {max_increment}\n\n")
        
        f.write("重置分析:\n")
        f.write(f"  检测到重置次数: {len(reset_points)}\n")
        if reset_points:
            f.write(f"  重置点位置: {reset_points[:10]}{'...' if len(reset_points) > 10 else ''}\n")
        f.write(f"  平均重置间隔: {len(scan_times) / (len(reset_points) + 1) if reset_points else 'N/A':.1f} 包/次\n\n")
        
        f.write("一致性评估:\n")
        f.write(f"  一致性评分: {consistency_score:.3f} (1.0为完美一致)\n")
        
        if consistency_score > 0.8:
            f.write("  评估结果: 优秀 - scan_time递增非常稳定\n")
        elif consistency_score > 0.6:
            f.write("  评估结果: 良好 - scan_time递增基本稳定\n")
        elif consistency_score > 0.4:
            f.write("  评估结果: 一般 - scan_time递增有一定波动\n")
        else:
            f.write("  评估结果: 较差 - scan_time递增不稳定\n")
        
        f.write("\n详细数据:\n")
        f.write("序号\t时间戳\t\tscan_time\t递增值\n")
        f.write("-" * 50 + "\n")
        
        for i, (ts, st) in enumerate(valid_data):
            if i == 0:
                increment_str = "N/A"
            else:
                increment_str = f"{increments[i-1]:.1f}"
            f.write(f"{i+1}\t{ts.strftime('%H:%M:%S.%f')[:-3]}\t{st}\t\t{increment_str}\n")
    
    print(f"scan_time分析报告已生成: {report_path}")
    
    return {
        'report_path': report_path,
        'total_packets': len(scan_times),
        'reset_count': len(reset_points),
        'avg_increment': avg_increment,
        'consistency_score': consistency_score
    }

def batch_generate_scan_time_reports(input_directory, output_directory):
    """
    批量生成目录中所有文件的scan_time分析报告
    
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
            
            print(f"正在生成scan_time报告: {filename}")
            try:
                report_result = generate_scan_time_report_simple(input_file, output_directory)
                if report_result:
                    results.append({
                        'filename': filename,
                        **report_result
                    })
                print(f"报告生成完成: {filename}")
            except Exception as e:
                print(f"生成报告时出错 {filename}: {e}")
            print()
    
    # 生成汇总报告
    if results:
        summary_file = os.path.join(output_directory, 'scan_time_summary_report.txt')
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write("scan_time分析汇总报告\n")
            f.write("=" * 30 + "\n\n")
            
            f.write("文件\t\t总包数\t重置次数\t平均递增值\t一致性评分\n")
            f.write("-" * 70 + "\n")
            
            for result in results:
                f.write(f"{result['filename'][:20]:<20}\t{result['total_packets']}\t{result['reset_count']}\t\t{result['avg_increment']:.2f}\t\t{result['consistency_score']:.3f}\n")
            
            f.write(f"\n总计文件数: {len(results)}\n")
            f.write(f"平均一致性评分: {sum(r['consistency_score'] for r in results) / len(results):.3f}\n")
        
        print(f"汇总报告已生成: {summary_file}")
    
    return results
# ----------------------------------------------------
def parse_iso_timestamp(timestamp_str):
    """
    解析ISO格式时间戳，兼容纳秒精度
    
    Args:
        timestamp_str (str): ISO格式时间戳字符串
        
    Returns:
        datetime: 解析后的时间对象
    """
    try:
        # 尝试直接解析
        return datetime.fromisoformat(timestamp_str)
    except ValueError:
        # 如果解析失败，处理纳秒精度问题
        try:
            # 分离时间部分和时区部分
            if '+' in timestamp_str:
                time_part, tz_part = timestamp_str.rsplit('+', 1)
                tz_offset = '+' + tz_part
            elif timestamp_str.endswith('Z'):
                time_part = timestamp_str[:-1]
                tz_offset = '+00:00'
            else:
                # 没有时区信息
                time_part = timestamp_str
                tz_offset = '+00:00'
            
            # 处理纳秒部分（限制为6位微秒）
            if '.' in time_part:
                date_part, time_fraction = time_part.split('.', 1)
                # 只取前6位作为微秒（Python datetime只支持微秒精度）
                if len(time_fraction) > 6:
                    time_fraction = time_fraction[:6]
                # 补齐到6位
                time_fraction = time_fraction.ljust(6, '0')
                time_part = f"{date_part}.{time_fraction}"
            
            # 重新组合时间戳
            formatted_timestamp = time_part + tz_offset
            return datetime.fromisoformat(formatted_timestamp)
        except Exception as e:
            # 如果还是失败，使用另一种方法
            return parse_iso_timestamp_fallback(timestamp_str)

def parse_iso_timestamp_fallback(timestamp_str):
    """
    备用方法解析ISO时间戳
    
    Args:
        timestamp_str (str): ISO格式时间戳字符串
        
    Returns:
        datetime: 解析后的时间对象
    """
    # 移除时区信息，单独处理
    if '+' in timestamp_str:
        time_part, tz_part = timestamp_str.rsplit('+', 1)
        hours_offset = int(tz_part.split(':')[0])
        minutes_offset = int(tz_part.split(':')[1]) if ':' in tz_part else 0
    elif timestamp_str.endswith('Z'):
        time_part = timestamp_str[:-1]
        hours_offset = 0
        minutes_offset = 0
    else:
        time_part = timestamp_str
        hours_offset = 0
        minutes_offset = 0
    
    # 处理时间部分
    if '.' in time_part:
        # 分离日期时间和小数秒部分
        datetime_part, fraction = time_part.split('.', 1)
        # 只取前6位作为微秒
        microsecond = int(fraction[:6].ljust(6, '0')) if len(fraction) >= 6 else int(fraction.ljust(6, '0'))
    else:
        datetime_part = time_part
        microsecond = 0
    
    # 解析日期时间部分
    dt = datetime.strptime(datetime_part, "%Y-%m-%dT%H:%M:%S")
    dt = dt.replace(microsecond=microsecond)
    
    # 应用时区偏移
    if hours_offset != 0 or minutes_offset != 0:
        tz = timezone(datetime.timedelta(hours=hours_offset, minutes=minutes_offset))
        dt = dt.replace(tzinfo=tz)
    else:
        dt = dt.replace(tzinfo=timezone.utc)
    
    return dt

def get_beijing_timestamps_only(csv_file_path):
    """
    解析CSV文件并只返回东八区时间戳字符串数组
    
    Args:
        csv_file_path (str): CSV文件路径
        
    Returns:
        list: 东八区时间戳字符串列表
    """
    beijing_tz = pytz.timezone('Asia/Shanghai')
    utc_tz = pytz.UTC
    
    beijing_timestamps = []
    
    with open(csv_file_path, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # 跳过标题行
        next(reader)  # 跳过第一行

        for row in reader:
            if len(row) >= 4:  # 确保行有足够数据
                name = row[0]
                start_time_str = row[2]  # 时间戳在第3列
                # 当名字为1-Wire [1]的跳过
                if name == "1-Wire [1]":
                    continue
                try:
                    # 解析ISO 8601时间戳
                    dt = parse_iso_timestamp(start_time_str)
                    
                    # 确保时间被视为UTC
                    if dt.tzinfo is None:
                        dt = utc_tz.localize(dt)
                    else:
                        dt = dt.astimezone(utc_tz)
                    
                    # 转换为东八区时间
                    beijing_time = dt.astimezone(beijing_tz)
                    # 直接加八个小时，不转换时区
                    # beijing_time = dt + timedelta(hours=8)
                    beijing_timestamps.append(beijing_time)
                    
                except Exception as e:
                    print(f"解析时间戳时出错: {start_time_str}, 错误: {e}")
    
    return beijing_timestamps


def analyze_ble_timer(input_file):

    # 提取数据
    handle_pattern = re.compile(r'.*Handle:0x003D.*')
    timestamp_pattern = re.compile(r'([A-Za-z]{3}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2}\.\d{3})')
    
    timestamps = []
    scan_times = []
    
    # 定义东八区时区
    beijing_tz = pytz.timezone('Asia/Shanghai')

    with open(input_file, 'r', encoding='utf-8') as infile:
        for line in infile:
            if handle_pattern.search(line):
                timestamp_match = timestamp_pattern.search(line)
                if timestamp_match:
                    timestamp_str = timestamp_match.group(1)
                    current_year = datetime.now().year
                    try:
                        month_day_time = timestamp_str
                        timestamp_obj = datetime.strptime(f"{current_year} {month_day_time}", 
                                                         "%Y %b %d %H:%M:%S.%f")
                        
                        # 为BLE时间添加东八区时区信息
                        timestamp_obj = beijing_tz.localize(timestamp_obj)
                        timestamps.append(timestamp_obj)
                        
                        scan_time = extract_scan_time_from_line(line)
                        scan_times.append(scan_time)
                    except ValueError:
                        print(f"无法解析时间戳: {timestamp_str}")
                        scan_times.append(None)# 使用示例

    # 过滤有效数据
    valid_data = [(ts, st) for ts, st in zip(timestamps, scan_times) if st is not None]
    if not valid_data:
        print("没有找到有效的scan_time数据")
        return
    # 去掉第一行
    valid_data = valid_data[1:]
    return valid_data

def analyze_ble(file_name):
    gpio_file = r"D:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval2\\"+file_name+".csv"
    gpio_result = get_beijing_timestamps_only(gpio_file)
    ble_file = r"D:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval2\\"+file_name+".txt"
    ble_result = analyze_ble_timer(ble_file)
    # 总包数
    total_packets = len(ble_result)
    gpio_packets = len(gpio_result)
    # 持续时间 最后一个包减去第一个包。精确到毫秒
    if total_packets > 0:
        duration = (ble_result[-1][0] - ble_result[0][0]).total_seconds() * 1000
    else:
        duration = 0
    # 平均帧率
    if duration > 0:
        average_fps = total_packets / (duration / 1000)  # 毫秒转为秒
    else:
        average_fps = 0

    print(f"总包数:ble: {total_packets},gpio:{gpio_packets}")
    print(f"持续时间: {duration:.2f} 毫秒")
    print(f"平均帧率: {average_fps:.2f} FPS")

    # 更详细的丢包统计，包括位置信息
    valid_scan_times = [item[1] for item in ble_result if item[1] is not None]

    # 获取scan_time的实际范围
    min_scan_time = min(valid_scan_times)
    max_scan_time = max(valid_scan_times)

    # 创建在这个范围内的期望完整包序列集合
    expected_packets = set(range(min_scan_time, max_scan_time + 1))

    # 创建实际收到的包集合
    actual_packets = set(valid_scan_times)

    # 计算丢失的包数
    missing_packets = len(expected_packets - actual_packets)

    # 获取具体丢失的包序号
    missing_packet_numbers = sorted(list(expected_packets - actual_packets))

    print(f"scan_time范围: {min_scan_time} - {max_scan_time}")
    print(f"期望包数: {len(expected_packets)}")
    print(f"实际收到包数: {len(actual_packets)}")
    print(f"丢包数: {missing_packets}")

    # 显示丢失包的详细信息
    if 0 < missing_packets <= 20:  # 限制显示数量
        print(f"丢失的包序号: {missing_packet_numbers}")

        # 计算这些丢失的包在理想序列中的位置索引
        missing_indices = []
        expected_list = list(range(min_scan_time, max_scan_time + 1))
        for missing_num in missing_packet_numbers:
            index = missing_num - min_scan_time  # 在理想序列中的索引
            missing_indices.append(index)
        print(f"丢失包在理想序列中的位置索引: {missing_indices}")
    elif missing_packets > 20:
        print(f"前10个丢失的包序号: {missing_packet_numbers[:10]}")
        expected_list = list(range(min_scan_time, max_scan_time + 1))
        first_10_missing = missing_packet_numbers[:10]
        missing_indices = [num - min_scan_time for num in first_10_missing]
        print(f"前10个丢失包在理想序列中的位置索引: {missing_indices}")
        print(f"...还有 {missing_packets - 10} 个丢失的包")


    # ble_result 最小间隔，以及最小间隔次数

    ble_intervals = [(ble_result[i + 1][0] - ble_result[i][0]).total_seconds() * 1000
                        for i in range(len(ble_result) - 1)]
    min_interval = min(ble_intervals)
    min_interval_count = ble_intervals.count(min_interval)
    print(f"BLE最小间隔: {min_interval:.2f} 毫秒, 次数: {min_interval_count}")
    # 最大间隔，以及最大间隔次数
    max_interval = max(ble_intervals)
    max_interval_count = ble_intervals.count(max_interval)
    print(f"BLE最大间隔: {max_interval:.2f} 毫秒, 次数: {max_interval_count}")
    # 间隔标准差
    ble_interval_stddev = (sum((x - min_interval) ** 2 for x in ble_intervals) / len(ble_intervals)) ** 0.5
    print(f"BLE间隔标准差: {ble_interval_stddev:.2f} 毫秒")


    # 确保两个列表长度一致
    min_length = min(len(gpio_result), len(ble_result))
    # 计算延迟，gpio_result 为包发送时间，ble_result中第一个参数为包收到时间
    gpio_time_base = gpio_result[0]
    ble_time_base = ble_result[0][0]
    # 重新计算每个时间和基准时间的差值
    gpio_deltas = [(ts - gpio_time_base).total_seconds() * 1000 for ts in gpio_result]
    ble_deltas = [(ts[0] - ble_time_base).total_seconds() * 1000 for ts in ble_result]
    print(f"gpio_deltas len:{len(gpio_deltas)},ble_deltas len:{len(ble_deltas)}")
    # 计算每个包的延迟
    latencies = [ble_deltas[i] - gpio_deltas[i] for i in range(min_length)]
    # 所有的延迟加上延迟最小值，所有值都大于等于0
    latencies = [x + abs(min(latencies)) for x in latencies]
    # for i in range(min_length):
    #     gpio_time = gpio_result[i]
    #     ble_time = ble_result[i][0]  # ble_result中的第一个元素是datetime对象
    #     # print(f"gpio_time:{gpio_time}, ble_time:{ble_time}")
    #     # 计算延迟（毫秒） 统一转换为时间戳计算
    #     latency = (ble_time - gpio_time).total_seconds() * 1000
    #     latencies.append(latency)

    # 平均延迟
    average_latency = sum(latencies) / len(latencies) if latencies else 0
    print(f"平均延迟: {average_latency:.2f} 毫秒")

    if latencies:
        # 最大延迟，以及最大延迟次数
        max_latency = max(latencies)
        max_latency_count = latencies.count(max_latency)
        print(f"最大延迟: {max_latency:.2f} 毫秒, 次数: {max_latency_count}")
        # 获取最大延迟位置
        max_latency_index = latencies.index(max_latency)

        print(f"最大延迟位置: {max_latency_index}，最大延迟时间：{ble_result[max_latency_index]}")

        # 延迟标准差
        latency_stddev = (sum((x - average_latency) ** 2 for x in latencies) / len(latencies)) ** 0.5
        print(f"延迟标准差: {latency_stddev:.2f} 毫秒")

    print(f"总共处理了 {min_length} 对数据包")



    # 汇总所有数据并返回
    return {
        "total_packets": total_packets,
        "gpio_packets": gpio_packets,
        "average_fps": round(average_fps, 2),
        "average_latency": round(average_latency, 2),
        "max_latency": round(max_latency, 2),
        "max_latency_count": max_latency_count,
        "min_interval": round(min_interval, 2),
        "min_interval_count": min_interval_count,
        "max_interval": round(max_interval, 2),
        "max_interval_count": max_interval_count,
        "latency_stddev": round(latency_stddev, 2),
        "missing_packets": missing_packets,
    }

def batch_analyze_ble_data(base_path="./captuer_data/bt_interval2", output_path="./captuer_data/bt_interval2/result"):
    """
    批量分析BLE数据并生成汇总报告
    
    参数:
    base_path: 数据文件夹路径
    output_path: 结果输出路径
    """
    import os
    from pathlib import Path
    import pandas as pd
    
    # 创建输出目录
    Path(output_path).mkdir(parents=True, exist_ok=True)
    
    # 存储所有分析结果
    all_results = []
    
    # 获取所有文件名并去重（去掉后缀）
    file_names = set()
    base_dir = Path(base_path)
    
    # 遍历目录中的所有文件
    for file in base_dir.iterdir():
        if file.is_file() and (file.suffix == '.txt' or file.suffix == '.csv'):
            # 获取文件名（不包含后缀）
            name_without_ext = file.stem
            file_names.add(name_without_ext)
    
    print(f"找到 {len(file_names)} 个不同的文件组: {list(file_names)}")
    
    # 对每个文件名调用analyze_ble方法
    for file_name in file_names:
        print(f"正在分析: {file_name}")
        
        try:
            # 调用analyze_ble方法
            result = analyze_ble(file_name)
            
            # 添加文件名到结果中
            result['file_name'] = file_name
            
            # 添加到汇总结果
            all_results.append(result)
            
            print(f"完成分析: {file_name}")
            
        except Exception as e:
            print(f"分析 {file_name} 时出错: {e}")
    
    # 生成汇总报告
    if all_results:
        # 转换为DataFrame
        df = pd.DataFrame(all_results)
        
        # 保存为CSV格式
        csv_path = Path(output_path) / "ble_analysis_summary.csv"
        df.to_csv(csv_path, index=False, encoding='utf-8-sig')
        
        # 生成文本格式的汇总报告
        txt_path = Path(output_path) / "ble_analysis_summary.txt"
        with open(txt_path, 'w', encoding='utf-8') as f:
            f.write("BLE数据分析汇总报告\n")
            f.write("=" * 50 + "\n\n")
            
            for result in all_results:
                f.write(f"文件名: {result['file_name']}\n")
                f.write(f"  总包数(BLE): {result['total_packets']}\n")
                f.write(f"  GPIO包数: {result['gpio_packets']}\n")
                f.write(f"  平均帧率: {result['average_fps']:.2f} FPS\n")
                f.write(f"  平均延迟: {result['average_latency']:.2f} ms\n")
                f.write(f"  最大延迟: {result['max_latency']:.2f} ms\n")
                f.write(f"  最小间隔: {result['min_interval']:.2f} ms\n")
                f.write(f"  最大间隔: {result['max_interval']:.2f} ms\n")
                f.write(f"  丢包数: {result['missing_packets']}\n")
                f.write(f"  延迟标准差: {result['latency_stddev']:.2f} ms\n")
                f.write("\n")
        
        print(f"分析完成，共处理 {len(all_results)} 个数据集")
        print(f"结果已保存至: {output_path}")
        
        return df
    else:
        print("未找到可分析的数据")
        return None


# 使用示例
if __name__ == "__main__":
    # 创建结果目录
    result_dir = r"d:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval\result"
    os.makedirs(result_dir, exist_ok=True)
    #
    # # 生成单个文件的scan_time详细报告
    # print("\n生成scan_time详细报告...")
    input_file = r"d:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval\KB04122_optimize_03.txt"
    # if os.path.exists(input_file):
    #     try:
    #         report_result = generate_scan_time_report_simple(input_file, result_dir)
    #         print(f"报告生成成功: {report_result}")
    #     except Exception as e:
    #         print(f"生成报告时出错: {e}")
    # else:
    #     print(f"文件不存在: {input_file}")

    # 批量生成scan_time报告
    # print("\n批量生成scan_time报告...")
    input_dir = r"d:\Code\VScode\example\python\touch_optimize\captuer_data\bt_interval"
    # batch_generate_scan_time_reports(input_dir, result_dir)


    result = extractAndAnalyzeTimestampsAndScanTime(input_file)
    print(result)
    # batch_analyze_logs(input_dir, result_dir)

    # analyze_ble("beisi_ble_66hz_3")

    # batch_analyze_ble_data()
