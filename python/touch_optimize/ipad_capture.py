import numpy as np
import pandas as pd
import json
import os
from scipy import signal
import matplotlib.pyplot as plt

def load_touchpad_data(file_path):
    """从JSON文件加载触控板数据"""
    try:
        with open(file_path, 'r') as f:
            data = json.load(f)

        # 验证数据格式
        required_keys = ['timeStamp', 'movementX', 'movementY', 'timeReceived']
        for i, entry in enumerate(data):
            if not all(key in entry for key in required_keys):
                print(f"警告: 数据点 {i} 缺少必要字段: {entry}")

        return data
    except Exception as e:
        print(f"加载数据失败: {e}")
        return []

def analyze_touchpad_data(data):
    """
    分析触控板数据，计算关键性能指标
    :param data: 触控板数据列表，格式为 [{"timeStamp": t1, "movementX": x1, "movementY": y1, "timeReceived": r1}, ...]
    :return: 包含分析结果的字典
    """
    if not data:
        return {"error": "无有效数据"}

    # 转换为DataFrame并排序
    df = pd.DataFrame(data)

    # 数据清洗 - 移除无效数据点
    initial_count = len(df)
    df = df.dropna(subset=['timeStamp', 'movementX', 'movementY', 'timeReceived'])
    # df = df[(df['movementX'] != 0) | (df['movementY'] != 0)]  # 移除零位移点
    cleaned_count = len(df)

    if cleaned_count < 3:
        return {"error": f"有效数据点不足 ({cleaned_count}/{initial_count})，需要至少3个点进行分析"}

    df.sort_values('timeStamp', inplace=True)
    df.reset_index(drop=True, inplace=True)

    # 1. 时间相关计算
    time_deltas = np.diff(df['timeStamp'])
    total_time = df['timeStamp'].iloc[-1] - df['timeStamp'].iloc[0]

    # 2. 刷新率分析
    avg_refresh_rate = len(df) / (total_time / 1000)  # 转换为Hz
    min_refresh_interval = np.min(time_deltas) if len(time_deltas) > 0 else 0
    max_refresh_interval = np.max(time_deltas) if len(time_deltas) > 0 else 0
    refresh_interval_std = np.std(time_deltas) if len(time_deltas) > 0 else 0

    # 3. 丢包检测 (基于时间戳间隔)
    if len(time_deltas) > 0:
        median_interval = np.median(time_deltas)
        threshold = median_interval * 2.5 if median_interval > 0 else 10  # 默认阈值10ms
        packet_loss_count = np.sum(time_deltas > threshold)
        packet_loss_rate = packet_loss_count / len(time_deltas)
    else:
        threshold = 0
        packet_loss_count = 0
        packet_loss_rate = 0

    # 4. 传输延迟分析
    processing_delays = df['timeReceived'] - df['timeStamp']
    avg_processing_delay = np.mean(processing_delays)
    max_processing_delay = np.max(processing_delays)

    # 5. 轨迹平滑度分析 (使用速度和加速度变化)
    # 计算瞬时速度 (像素/毫秒)
    displacements = np.sqrt(df['movementX'] ** 2 + df['movementY'] ** 2)[1:]
    instantaneous_velocities = displacements / time_deltas if len(time_deltas) > 0 else np.array([])

    # 计算加速度
    if len(instantaneous_velocities) > 1:
        acceleration = np.diff(instantaneous_velocities) / time_deltas[:-1]
    else:
        acceleration = np.array([])

    # 平滑度指标 (速度变化率和加速度标准差)
    velocity_variation = np.std(instantaneous_velocities) / np.mean(instantaneous_velocities) if len(
        instantaneous_velocities) > 0 else 0
    acceleration_std = np.std(acceleration) if len(acceleration) > 0 else 0

    # 6. 轨迹线性度分析 (理想圆形应保持恒定曲率)
    angles = []
    for i in range(1, len(df) - 1):
        v1 = np.array([df['movementX'].iloc[i - 1], df['movementY'].iloc[i - 1]])
        v2 = np.array([df['movementX'].iloc[i], df['movementY'].iloc[i]])
        if np.linalg.norm(v1) > 0 and np.linalg.norm(v2) > 0:
            cos_angle = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
            # 处理浮点精度问题
            cos_angle = np.clip(cos_angle, -1.0, 1.0)
            angles.append(np.arccos(cos_angle))

    angle_variation = np.std(angles) if angles else 0

    return {
        # 刷新率指标
        'average_refresh_rate_hz': avg_refresh_rate,
        'min_refresh_interval_ms': min_refresh_interval,
        'max_refresh_interval_ms': max_refresh_interval,
        'refresh_interval_std_ms': refresh_interval_std, #标准差

        # 丢包指标
        'packet_loss_count': packet_loss_count,
        'packet_loss_rate': packet_loss_rate,
        'packet_loss_threshold_ms': threshold,

        # 延迟指标
        'avg_processing_delay_ms': avg_processing_delay,
        'max_processing_delay_ms': max_processing_delay,

        # 轨迹质量指标
        'velocity_variation_coeff': velocity_variation,  # 越小越平滑
        'acceleration_std': acceleration_std,  # 越小越平滑
        'angle_variation_rad': angle_variation,  # 越小越接近圆形

        # 数据统计
        'total_data_points': initial_count,
        'cleaned_data_points': cleaned_count,
        'total_duration_ms': total_time
    }

def generate_report(results, output_file='touchpad_report.txt'):
    """生成详细的分析报告"""
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write("触控板性能分析报告\n")
        f.write("=" * 50 + "\n")

        if 'error' in results:
            f.write(f"错误: {results['error']}\n")
            return

        f.write(f"1. 数据概况:\n")
        f.write(f"  原始数据点: {results['total_data_points']}\n")
        f.write(f"  有效数据点: {results['cleaned_data_points']}\n")
        f.write(f"  测试总时长: {results['total_duration_ms']:.2f} ms\n\n")

        f.write(f"2. 刷新率指标:\n")
        f.write(f"  平均刷新率: {results['average_refresh_rate_hz']:.2f} Hz\n")
        f.write(f"  最小刷新间隔: {results['min_refresh_interval_ms']:.2f} ms\n")
        f.write(f"  最大刷新间隔: {results['max_refresh_interval_ms']:.2f} ms\n")
        f.write(f"  刷新间隔标准差: {results['refresh_interval_std_ms']:.2f} ms\n\n")

        f.write(f"3. 丢包检测:\n")
        f.write(f"  丢包次数: {results['packet_loss_count']}\n")
        f.write(f"  丢包率: {results['packet_loss_rate'] * 100:.2f}%\n")
        f.write(f"  丢包判定阈值: {results['packet_loss_threshold_ms']:.2f} ms\n\n")

        f.write(f"4. 系统延迟:\n")
        f.write(f"  平均处理延迟: {results['avg_processing_delay_ms']:.2f} ms\n")
        f.write(f"  最大处理延迟: {results['max_processing_delay_ms']:.2f} ms\n\n")

        f.write(f"5. 轨迹质量:\n")
        f.write(f"  速度变异系数: {results['velocity_variation_coeff']:.4f} (越小越好)\n")
        f.write(f"  加速度标准差: {results['acceleration_std']:.4f} (越小越好)\n")
        f.write(f"  角度变化标准差: {results['angle_variation_rad']:.4f} rad (越小越好)\n\n")

        f.write("\n详细图表已保存至对应的图片文件中\n")

def plot_touchpad_analysis(data, output_path):
    """
    绘制触控板分析图表
    :param data: 触控板数据
    :param output_path: 图片输出路径
    """
    if not data:
        return

    # 转换为DataFrame并排序
    df = pd.DataFrame(data)
    df = df.dropna(subset=['timeStamp', 'movementX', 'movementY', 'timeReceived'])
    df.sort_values('timeStamp', inplace=True)
    df.reset_index(drop=True, inplace=True)

    # 计算相关数据
    time_deltas = np.diff(df['timeStamp'])
    processing_delays = df['timeReceived'] - df['timeStamp']
    
    # 计算瞬时速度
    displacements = np.sqrt(df['movementX'] ** 2 + df['movementY'] ** 2)[1:]
    instantaneous_velocities = displacements / time_deltas if len(time_deltas) > 0 else np.array([])
    
    # 计算丢包阈值
    if len(time_deltas) > 0:
        median_interval = np.median(time_deltas)
        threshold = median_interval * 2.5 if median_interval > 0 else 10
    else:
        threshold = 0

    # 绘制轨迹图
    plt.figure(figsize=(10, 8))

    # 轨迹图
    plt.subplot(2, 2, 1)
    plt.plot(df['movementX'].cumsum(), df['movementY'].cumsum(), 'b-', linewidth=1)
    plt.title('Touchpad Trajectory')
    plt.xlabel('X displacement (pixels)')
    plt.ylabel('Y displacement (pixels)')
    plt.grid(True)
    plt.axis('equal')

    # 速度分布图
    plt.subplot(2, 2, 2)
    if len(instantaneous_velocities) > 0:
        plt.plot(range(len(instantaneous_velocities)), instantaneous_velocities, 'g-')
        plt.title('Velocity Variation')
        plt.xlabel('Sample index')
        plt.ylabel('Velocity (pixels/ms)')
        plt.grid(True)

    # 刷新间隔分布
    plt.subplot(2, 2, 3)
    if len(time_deltas) > 0:
        plt.plot(range(len(time_deltas)), time_deltas, 'r-')
        plt.axhline(y=threshold, color='k', linestyle='--', label='Packet Loss Threshold')
        plt.title('Refresh Intervals')
        plt.xlabel('Interval index')
        plt.ylabel('Interval (ms)')
        plt.legend()
        plt.grid(True)

    # 延迟分布
    plt.subplot(2, 2, 4)
    plt.plot(range(len(processing_delays)), processing_delays, 'm-')
    plt.title('Processing Delays')
    plt.xlabel('Sample index')
    plt.ylabel('Delay (ms)')
    plt.grid(True)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    plt.close()

def batch_process_json_files(data_dir, result_dir):
    """
    批量处理目录下的所有JSON文件
    :param data_dir: 数据目录路径
    :param result_dir: 结果保存目录路径
    """

    # 获取data_dir 目录的最后一个文件夹名
    data_dir_name = os.path.basename(data_dir)
    # 创建结果目录
    os.makedirs(result_dir, exist_ok=True)
    os.makedirs(os.path.join(result_dir, data_dir_name), exist_ok=True)
    
    # 获取所有JSON文件
    json_files = [f for f in os.listdir(data_dir) if f.endswith('.json')]
    
    if not json_files:
        print(f"在目录 {data_dir} 中未找到JSON文件")
        return
    
    print(f"找到 {len(json_files)} 个JSON文件")
    
    # 存储所有结果用于生成表格
    all_results = []
    
    # 处理每个文件
    for json_file in json_files:
        file_name = os.path.splitext(json_file)[0]
        file_path = os.path.join(data_dir, json_file)
        
        print(f"\n处理文件: {json_file}")
        
        # 加载数据
        touchpad_data = load_touchpad_data(file_path)
        if not touchpad_data:
            print(f"  跳过文件 {json_file} - 无有效数据")
            continue
            
        # 分析数据
        print(f"  分析 {len(touchpad_data)} 个数据点...")
        results = analyze_touchpad_data(touchpad_data)
        
        # 添加文件名到结果中
        results['file_name'] = file_name
        
        # 保存结果
        all_results.append(results)
        
        # 生成报告
        report_path = os.path.join(result_dir, data_dir_name, f"{file_name}_report.txt")
        generate_report(results, report_path)
        
        # 生成图表
        image_path = os.path.join(result_dir, data_dir_name, f"{file_name}_analysis.png")
        plot_touchpad_analysis(touchpad_data, image_path)
        
        print(f"  完成处理: {json_file}")
        print(f"    报告保存至: {report_path}")
        print(f"    图表保存至: {image_path}")
    
    # 生成汇总表格
    generate_summary_table(all_results, os.path.join(result_dir, data_dir_name, data_dir_name+'summary_report.csv'))
    print(f"\n汇总报告已保存至: {os.path.join(result_dir, data_dir_name, data_dir_name+'summary_report.csv')}")

def generate_summary_table(results_list, output_file):
    """
    生成汇总表格报告
    :param results_list: 所有分析结果列表
    :param output_file: 输出文件路径
    """
    # with open(output_file, 'w', encoding='utf-8') as f:
    #     f.write("# 触控板数据分析汇总报告\n\n")
        
    #     f.write("## 性能指标汇总表\n\n")
    #     f.write("| 文件名 | 平均刷新率(Hz) | 丢包率(%) | 平均延迟(ms) | 速度变异系数 | 加速度标准差 |\n")
    #     f.write("|-------|---------------|----------|-------------|-------------|-------------|\n")
        
    #     for results in results_list:
    #         if 'error' in results:
    #             f.write(f"| {results['file_name']} | 错误 | 错误 | 错误 | 错误 | 错误 |\n")
    #             continue
                
    #         f.write(f"| {results['file_name']} | {results['average_refresh_rate_hz']:.2f} | "
    #                 f"{results['packet_loss_rate']*100:.2f} | {results['avg_processing_delay_ms']:.2f} | "
    #                 f"{results['velocity_variation_coeff']:.4f} | {results['acceleration_std']:.4f} |\n")
        
    #     f.write("\n## 详细说明\n\n")
    #     f.write("1. **平均刷新率**: 触控板每秒上报数据的平均次数，越高越好\n")
    #     f.write("2. **丢包率**: 数据传输过程中丢失的数据包比例，越低越好\n")
    #     f.write("3. **平均延迟**: 从触控发生到数据被接收的平均时间，越低越好\n")
    #     f.write("4. **速度变异系数**: 描述轨迹平滑度，越小越平滑\n")
    #     f.write("5. **加速度标准差**: 描述轨迹稳定性的指标，越小越稳定\n")
    # 将数据输出为csv格式
    df = pd.DataFrame(results_list)
    # # 调整顺序将 file_name 字段放到第一位
    # df = df[['file_name', 'average_refresh_rate_hz', 'packet_loss_rate', 'avg_processing_delay_ms', 'velocity_variation_coeff', 'acceleration_std']]
    # # 将file_name 字段设置为索引
    # df.set_index('file_name', inplace=True)
    df.to_csv(output_file, index=False)

# 主程序
if __name__ == "__main__":
    # 设置目录路径
    data_directory = "./captuer_data/data_version"
    result_directory = "./result"
    
    # 批量处理
    batch_process_json_files(data_directory, result_directory)