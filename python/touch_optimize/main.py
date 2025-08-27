from parse import compile, findall

import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

import os
from pathlib import Path
import pandas as pd

# 设置中文字体支持
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'DejaVu Sans']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号

def correct_timestamps(timestamps, max_reset_value=100000):
    """
    校正时间戳溢出问题
    :param timestamps: 原始时间戳数组(微秒)
    :param max_reset_value: 时间戳最大值(超过此值会重置)
    :return: 校正后的时间戳数组
    """
    corrected = []
    offset = 0
    last_ts = timestamps[0]
    diff_ts = timestamps[1] - last_ts
    error_ts = 0
    error_before_ts = 0
    for i, ts in enumerate(timestamps):
        # 检测时间戳重置 (当前值远小于前一个值)
        if i > 0 and ts < last_ts and (last_ts - ts) > max_reset_value // 2:
            error_ts = ts
            # error_before_ts = last_ts
            print(f"检测到时间戳重置: {last_ts} -> {ts} (添加偏移 {offset})")
            corrected_ts = last_ts + diff_ts
            error_before_ts = corrected_ts
        else:
            corrected_ts = (ts - error_ts)+ error_before_ts
        corrected.append(corrected_ts)
        diff_ts = ts - last_ts
        last_ts = ts
    
    return np.array(corrected)

def detect_anomalies(timestamps, coords, threshold=3.0):
    """
    检测异常数据点(时间跳跃或位置突变)
    :param timestamps: 时间戳数组(微秒)
    :param coords: 坐标数组(N,2)
    :param threshold: 异常检测阈值(标准差的倍数)
    :return: 有效点掩码数组
    """
    # 计算时间间隔
    time_diffs = np.diff(timestamps)
    # 算数平均值
    avg_diff = np.mean(time_diffs)
    # 标准差
    std_diff = np.std(time_diffs)
    print(f"avg_diff: {avg_diff}, std_diff: {std_diff}")
    # 计算点间距离
    coord_diffs = np.linalg.norm(np.diff(coords, axis=0), axis=1)
    avg_dist = np.mean(coord_diffs)
    std_dist = np.std(coord_diffs)
    
    # 创建有效点掩码(所有点初始有效)
    valid_mask = np.ones(len(timestamps), dtype=bool)
    
    # 检测时间异常
    time_anomalies = np.where(
        (time_diffs > avg_diff + threshold * std_diff) | 
        (time_diffs < avg_diff - threshold * std_diff)
    )[0] + 1  # 偏移到下一个点
    # print(f"time_diffs:{time_diffs[time_anomalies[0]-1]}")
    # 检测位置异常
    pos_anomalies = np.where(
        coord_diffs > avg_dist + threshold * std_dist
    )[0] + 1
    
    # 标记异常点
    anomalies = np.unique(np.concatenate([time_anomalies, pos_anomalies]))
    for anomaly in anomalies:
        print(anomaly)
    # valid_mask[anomalies] = False
    
    print(f"检测到 {len(anomalies)} 个异常点，其中时间异常点{len(time_anomalies)},位置异常点{len(pos_anomalies)}")
    return valid_mask

def analyze_touchpad_data_dev(data):
    """
    分析开发板采集的触控板测试数据(增强版)
    
    参数:
    data - 包含触控数据的字典列表，每个字典包含:
        'x': X坐标
        'y': Y坐标
        'timestamp': 时间戳(微秒)
        'valid': 数据有效性标志 (布尔值)
        'confidence': 置信度标志 (布尔值)
    
    返回:
    包含分析结果的字典
    """
    # 1. 初步过滤无效数据
    valid_data = [d for d in data if d.get('valid', True) and d.get('confidence', True)]
    
    if len(valid_data) < 10:
        raise ValueError("有效数据点不足，无法进行分析")
    
    # 提取原始坐标和时间戳
    x_coords = np.array([d['x'] for d in valid_data])
    y_coords = np.array([d['y'] for d in valid_data])
    timestamps = np.array([d['timestamp'] for d in valid_data])
    
    # 2. 时间戳校正和异常检测
    corrected_timestamps = correct_timestamps(timestamps)
    coords = np.column_stack((x_coords, y_coords))
    valid_mask = detect_anomalies(corrected_timestamps, coords)
    
    # 应用异常检测结果
    final_data = [d for i, d in enumerate(valid_data) if valid_mask[i]]
    x_coords = np.array([d['x'] for d in final_data])
    y_coords = np.array([d['y'] for d in final_data])
    corrected_timestamps = corrected_timestamps[valid_mask]
    
    print(f"最终有效数据点: {len(final_data)}")
    
    # 转换为相对时间(毫秒)
    relative_timestamps = (corrected_timestamps - corrected_timestamps[0]) / 1000.0
    
    # 3. 计算时间间隔和刷新率
    time_intervals = np.diff(relative_timestamps)
    avg_interval = np.mean(time_intervals)
    print(avg_interval)
    refresh_rate = 1000 / avg_interval if avg_interval > 0 else 0
    
    # 4. 丢包检测
    std_interval = np.std(time_intervals)
    packet_loss_threshold = avg_interval + 2 * std_interval
    packet_loss_points = np.where(time_intervals > packet_loss_threshold)[0]
    packet_loss_rate = len(packet_loss_points) / len(time_intervals) * 100
    
    # 5. 轨迹平滑度分析
    distances = np.sqrt(np.diff(x_coords)**2 + np.diff(y_coords)**2)
    velocities = distances / time_intervals
    
    accels = np.diff(velocities) / time_intervals[1:] if len(velocities) > 1 else []
    
    velocity_std = np.std(velocities) if len(velocities) > 0 else 0
    acceleration_std = np.std(accels) if len(accels) > 0 else 0
    
    # 6. 轨迹线性度分析
    slopes = []
    for i in range(1, len(x_coords)):
        dx = x_coords[i] - x_coords[i-1]
        dy = y_coords[i] - y_coords[i-1]
        if dx != 0:
            slopes.append(dy/dx)
    
    linearity_error = np.std(np.abs(slopes)) if slopes else 0
    
    # 7. 圆形轨迹分析
    circle_error = 0
    circle_params = None
    try:
        def circle_func(xy, xc, yc, r):
            x, y = xy
            return (x - xc)**2 + (y - yc)**2 - r**2
        
        initial_guess = (np.mean(x_coords), np.mean(y_coords), 
                         np.max([np.ptp(x_coords), np.ptp(y_coords)])/2)
        params, _ = curve_fit(circle_func, (x_coords, y_coords), 
                             np.zeros(len(x_coords)), 
                             p0=initial_guess)
        xc, yc, r = params
        circle_params = (xc, yc, r)
        
        distances_to_center = np.sqrt((x_coords - xc)**2 + (y_coords - yc)**2)
        radial_errors = np.abs(distances_to_center - r)
        circle_error = np.mean(radial_errors)
    except RuntimeError:
        circle_error = -1
    
    # 8. 压力数据分析
    force_analysis = {}
    if 'force' in final_data[0]:
        forces = np.array([d['force'] for d in final_data])
        max_force = np.max(forces) if len(forces) > 0 else 0
        
        force_analysis = {
            'avg_force': np.mean(forces),
            'force_std': np.std(forces),
            'max_force': max_force,
            'force_variation': np.ptp(forces)
        }
    
    # 9. 结果汇总
    results = {
        # 刷新率
        'refresh_rate': refresh_rate,
        # 丢包率
        'packet_loss_rate': packet_loss_rate,
        # 速度波动
        'velocity_std': velocity_std,
        # 加速度波动
        'acceleration_std': acceleration_std,
        # 线性度误差
        'linearity_error': linearity_error,
        # 圆形轨迹误差
        'circle_fit_error': circle_error,
        # 圆形轨迹参数
        'circle_params': circle_params,
        # 平均时间间隔
        'avg_interval': avg_interval,
        # 时间间隔标准差
        'time_interval_std': std_interval,
        # 总数据点数
        'total_points': len(final_data),
        # 移动距离
        'movement_distance': np.sum(distances),
        # 压力分析
        'force_analysis': force_analysis if force_analysis else None,
        'raw_data': {
            'x': x_coords,
            'y': y_coords,
            'timestamps': relative_timestamps
        },
        # 异常点信息
        'anomaly_info': {
            'initial_points': len(valid_data),
            'final_points': len(final_data),
            'removed_points': len(valid_data) - len(final_data)
        }
    }
    
    return results

def visualize_results(results, save_path=None):
    """可视化分析结果"""
    # 创建图形和子图
    fig = plt.figure(figsize=(15, 10))
    
    # 1. 轨迹图
    ax1 = plt.subplot(2, 2, 1)
    ax1.plot(results['raw_data']['x'], results['raw_data']['y'], 'b-', label='实际轨迹')
    ax1.plot(results['raw_data']['x'], results['raw_data']['y'], 'ro', markersize=2)
    
    if results['circle_params'] and results['circle_fit_error'] >= 0:
        xc, yc, r = results['circle_params']
        theta = np.linspace(0, 2*np.pi, 100)
        x_fit = xc + r * np.cos(theta)
        y_fit = yc + r * np.sin(theta)
        ax1.plot(x_fit, y_fit, 'g--', label='拟合圆')
        ax1.plot(xc, yc, 'kx', markersize=8, label='圆心')
    
    ax1.set_title(f'触控轨迹 (拟合误差: {results["circle_fit_error"]:.2f} units)')
    ax1.axis('equal')
    ax1.legend()
    ax1.grid(True)
    
    # 2. 速度分析
    ax2 = plt.subplot(2, 2, 2)
    if len(results['raw_data']['timestamps']) > 1:
        distances = np.sqrt(np.diff(results['raw_data']['x'])**2 + 
                          np.diff(results['raw_data']['y'])**2)
        time_intervals = np.diff(results['raw_data']['timestamps'])
        velocities = distances / time_intervals
        
        ax2.plot(results['raw_data']['timestamps'][1:], velocities, 'b-')
        ax2.set_title(f'移动速度 (标准差: {results["velocity_std"]:.2f} units/ms)')
        ax2.set_xlabel('时间 (ms)')
        ax2.set_ylabel('速度 (units/ms)')
        ax2.grid(True)
    
    # 3. 时间间隔分布
    ax3 = plt.subplot(2, 2, 3)
    if len(results['raw_data']['timestamps']) > 1:
        time_intervals = np.diff(results['raw_data']['timestamps'])
        ax3.hist(time_intervals, bins=30, alpha=0.7)
        ax3.axvline(results['avg_interval'], color='r', linestyle='dashed', 
                   label=f'平均间隔: {results["avg_interval"]:.2f}ms')
        ax3.set_title(f'时间间隔分布 (刷新率: {results["refresh_rate"]:.1f}Hz)')
        ax3.set_xlabel('时间间隔 (ms)')
        ax3.set_ylabel('频数')
        ax3.legend()
        ax3.grid(True)
    
    # 4. 压力分析 (如果存在)
    if results['force_analysis']:
        ax4 = plt.subplot(2, 2, 4)
        # 注意：这里需要获取实际的压力数据而不是平均值
        forces = [d.get('force', results['force_analysis']['avg_force']) 
                 for d in results.get('final_data', [])] if 'final_data' in results else [results['force_analysis']['avg_force']]
        if not forces:
            forces = [results['force_analysis']['avg_force']]
        ax4.plot(results['raw_data']['timestamps'], 
                [results['force_analysis']['avg_force']] * len(results['raw_data']['timestamps']), 'g-')
        ax4.set_title('压力变化')
        ax4.set_xlabel('时间 (ms)')
        ax4.set_ylabel('压力值')
        ax4.grid(True)
    
    plt.tight_layout()
    
    # 如果提供了保存路径，则保存图像
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
    
    return fig

def generate_report(results, material_name, thickness, frequency):
    """生成详细的测试报告"""
    report = f"""
    ==================================================
    触控板性能测试报告 - {material_name}
    ==================================================
    测试条件:
      材质: {material_name}
      厚度: {thickness} mm
      刷新率: {frequency} Hz
    
    数据质量:
      初始数据点: {results['anomaly_info']['initial_points']}
      有效数据点: {results['total_points']}
      移除异常点: {results['anomaly_info']['removed_points']}
    
    性能指标:
      平均刷新率: {results['refresh_rate']:.1f} Hz
      丢包率: {results['packet_loss_rate']:.2f}%
      速度波动: {results['velocity_std']:.4f} units/ms (标准差)
      加速度波动: {results['acceleration_std']:.6f} units/ms^2 (标准差)
      线性度误差: {results['linearity_error']:.4f}
    """
    
    if results['circle_fit_error'] >= 0:
        report += f"圆形轨迹误差: {results['circle_fit_error']:.2f} units (平均径向偏差)\n"
        xc, yc, r = results['circle_params']
        report += f"拟合圆参数: 中心({xc:.1f}, {yc:.1f}), 半径: {r:.1f} units\n"
    else:
        report += "圆形轨迹拟合失败\n"
    
    report += f"""
    时间特性:
      平均时间间隔: {results['avg_interval']:.3f} ± {results['time_interval_std']:.3f} ms
      总移动距离: {results['movement_distance']:.1f} units
    """
    
    if results['force_analysis']:
        report += f"""
    压力分析:
      平均压力: {results['force_analysis']['avg_force']:.1f}
      压力波动: {results['force_analysis']['force_std']:.1f} (标准差)
      最大压力: {results['force_analysis']['max_force']}
      压力变化范围: {results['force_analysis']['force_variation']}
    """
    
    report += "\n=================================================="
    return report

def parse_raw_touch_data(path: str) -> object:
    """
        解析触摸板原始数据
    """
    with open(path, 'r', encoding='utf8') as f:
        index = 0
        p_idx = compile("[{index:^d}] [{timestamp:^d}] {idx:^d} |")   # [ 19] [    10381877] 1 | True  True 00 ( 1345, 1121)   4  1290  2000
        p_item = compile("{valid:^} {confidence:^} 0{id:d} ({x:^d},{y:^d}) {size:^d} {force:^d} {max_force:^d}")
        response = []
        while True:
            line = f.readline()
            if not line:
                break
            line = line.replace('\n', '')
            result = p_idx.search(line)
            if result is not None:
                cnt = result['idx']
                # if cnt != 2:
                #     continue
                start_idx = result.spans['idx'][1]
                timestamp = result['timestamp']
                items_str = line[start_idx:]

                start_list = []
                for r in findall("|{start}", items_str):
                    start_list.append(r.spans['start'][1] - 1)
                if len(start_list) != cnt:
                    print("error:cnt not current")
                    continue
                item = []
                for idx in range(len(start_list)):
                    start = start_list[idx] + 1
                    item_str = ""
                    if idx + 1 == len(start_list):
                        item_str = items_str[start:]
                    else:
                        end = start_list[idx + 1] - 1
                        item_str = items_str[start:end]
                    item_result = p_item.parse(item_str)
                    if item_result["valid"] == "True":
                        s = {
                            'valid': item_result["valid"] == "True",
                            'confidence': item_result["confidence"] == "True",
                            'id': item_result["id"],
                            'x': item_result["x"],
                            'y': item_result["y"],
                            'size': item_result["size"],
                            'force': item_result["force"],
                            'max_force': item_result["max_force"],
                            'timestamp': timestamp,
                        }
                        # item.append(s)
                        response.append(s)
                # response.append(item)
            index += 1
        return response
    
    
def batch_analyze_touchpad_data(base_path="./captuer_data/devboard_100Hz", output_path="./result/devboard_100Hz"):
    """
    批量分析触控板数据并生成汇总报告
    
    参数:
    base_path: 数据文件夹路径
    output_path: 结果输出路径
    """
    # 创建输出目录
    Path(output_path).mkdir(parents=True, exist_ok=True)
    figures_path = Path(f"{output_path}/figures")
    figures_path.mkdir(parents=True, exist_ok=True)
    
    # 存储所有分析结果
    all_results = []
    
    # 遍历所有子目录
    base_dir = Path(base_path)
    for folder in base_dir.iterdir():
        if folder.is_dir():
            data_file = folder / "trackData.txt"
            if data_file.exists():
                print(f"正在分析: {folder.name}")
                
                try:
                    # 解析数据
                    data = parse_raw_touch_data(str(data_file))
                    
                    # 分析数据
                    results = analyze_touchpad_data_dev(data)
                    
                    # 提取关键指标
                    summary = {
                        '文件夹名称': folder.name,
                        '平均刷新率(Hz)': round(results['refresh_rate'], 2),
                        '丢包率(%)': round(results['packet_loss_rate'], 2),
                        '速度波动(units/ms)': round(results['velocity_std'], 4),
                        '加速度波动(units/ms²)': round(results['acceleration_std'], 6),
                        '线性度误差': round(results['linearity_error'], 4),
                        '圆形轨迹误差(units)': round(results['circle_fit_error'], 2) if results['circle_fit_error'] >= 0 else None,
                        '平均时间间隔(ms)': round(results['avg_interval'], 3),
                        '总数据点数': results['total_points'],
                        '移除异常点数': results['anomaly_info']['removed_points']
                    }
                    
                    all_results.append(summary)
                    
                    # 保存可视化图表
                    figure_path = figures_path / f"{folder.name}_analysis.png"
                    fig = visualize_results(results, str(figure_path))
                    plt.close(fig)  # 明确关闭图形以释放内存
                    
                    print(f"完成分析: {folder.name}")
                    
                except Exception as e:
                    print(f"分析 {folder.name} 时出错: {e}")
    
    # 生成汇总报告
    if all_results:
        df = pd.DataFrame(all_results)
        
        # 保存为CSV格式
        df.to_csv(f"{output_path}/summary_report.csv", index=False, encoding='utf-8-sig')
        
        # 生成Markdown表格
        try:
            markdown_table = df.to_markdown(index=False)
        except ImportError:
            # 如果没有安装tabulate，则手动创建markdown表格
            markdown_table = df.to_string(index=False)
        
        # 保存Markdown报告
        with open(f"{output_path}/summary_report.md", 'w', encoding='utf-8') as f:
            f.write("# 触控板测试汇总报告\n\n")
            f.write(str(markdown_table))
            f.write("\n")
        
        print(f"分析完成，共处理 {len(all_results)} 个数据集")
        print(f"结果已保存至: {output_path}")
        
        # 打印汇总表
        print("\n汇总结果:")
        print(markdown_table)
        
        return df
    else:
        print("未找到可分析的数据")
        return None


if __name__ == '__main__':
    print('start')
    # data = parse_raw_touch_data(
    #     r"./captuer_data/devboard_100Hz/record_20250805_beisi5/trackData.txt")

    # material_name="KB04122塑料2"
    # frequency = 100
    # results = analyze_touchpad_data_dev(data)
    # # 生成报告
    # report = generate_report(
    #     results, 
    #     material_name,
    #     thickness=0.3, 
    #     frequency = frequency
    # )

    # # 保存报告
    # with open(f"{material_name}_{frequency}_report.txt", 'w', encoding='utf-8') as f:
    #     f.write(report)

    # # 保存可视化图表
    # visualize_results(results)
    # plt.savefig('touchpad_analysis.png', dpi=300)
    print('开始批量分析')
    batch_results = batch_analyze_touchpad_data()
    print("批量分析结束")

    print("end")
