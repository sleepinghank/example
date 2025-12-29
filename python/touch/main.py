from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime
import logging
import matplotlib.pyplot as plt
from pathlib import Path
import json
import numpy as np  # 添加 numpy 导入
from parse import compile, findall
# 配置日志
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# 创建result文件夹
result_dir = Path('./result')
result_dir.mkdir(exist_ok=True)

app = Flask(__name__)
# 添加CORS支持
CORS(app, resources={
    r"/touch": {
        "origins": "*",  # 允许所有来源
        "methods": ["POST", "OPTIONS"],  # 允许的方法
        "allow_headers": ["Content-Type"]  # 允许的请求头
    }
})


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
                        item.append(s)
                response.append(item)
            index += 1
        return response

def plot_movement_data(touch_data):
    plt.figure(figsize=(12, 8), dpi=100)
    plt.style.use('seaborn')
    
    # 提取数据并归一化时间戳（从0开始）
    timestamps = [event['timeStamp'] for event in touch_data]
    start_time = min(timestamps)  # 获取最小时间戳
    normalized_timestamps = [t - start_time for t in timestamps]  # 归一化时间戳
    movement_x = [event['movementX'] for event in touch_data]
    movement_y = [event['movementY'] for event in touch_data]
    
    # 计算时间范围
    time_range = max(normalized_timestamps)
    
    # 设置合适的刻度间隔
    ax = plt.gca()
    
    # 主刻度
    major_interval = time_range / 20  # 显示20个主刻度
    ax.xaxis.set_major_locator(plt.MultipleLocator(major_interval))
    
    # 次刻度
    minor_interval = major_interval / 5  # 每个主刻度之间显示4个次刻度
    ax.xaxis.set_minor_locator(plt.MultipleLocator(minor_interval))
    
    # 绘制数据
    plt.plot(normalized_timestamps, movement_x, color='red', alpha=0.3, linestyle='-', linewidth=1)
    plt.plot(normalized_timestamps, movement_y, color='green', alpha=0.3, linestyle='-', linewidth=1)
    
    plt.scatter(normalized_timestamps, movement_x, color='red', label='movementX', 
               alpha=0.6, s=50)
    plt.scatter(normalized_timestamps, movement_y, color='green', label='movementY', 
               alpha=0.6, s=50)
    
    # 设置网格
    plt.grid(True, which='major', linestyle='-', alpha=0.7)
    plt.grid(True, which='minor', linestyle=':', alpha=0.4)
    
    # 格式化刻度标签
    def format_timestamp(x, p):
        return f'{x:.2f}'
    ax.xaxis.set_major_formatter(plt.FuncFormatter(format_timestamp))
    
    # 旋转刻度标签
    plt.xticks(rotation=45, ha='right')
    
    # 设置标题和标签
    plt.title('Touch Movement Analysis', fontsize=14, pad=20)
    plt.xlabel('Relative Time (ms)', fontsize=12)  # 更改为相对时间
    plt.ylabel('Movement Distance', fontsize=12)
    plt.legend(fontsize=10)
    
    # 调整布局
    plt.tight_layout()
    
    filename = f'movement_plot_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png'
    filepath = result_dir / filename
    plt.savefig(filepath, bbox_inches='tight')
    plt.close()
    
    # 准备JSON数据
    json_data = {
        'metadata': {
            'start_time': start_time,
            'total_points': len(timestamps),
            'duration': max(normalized_timestamps)
        },
        'data': [
            {
                'original_timestamp': orig_t,
                'relative_timestamp': rel_t,
                'movement_x': mx,
                'movement_y': my
            }
            for orig_t, rel_t, mx, my in zip(timestamps, normalized_timestamps, movement_x, movement_y)
        ]
    }
    
    # 保存JSON数据
    data_path = result_dir / f'{filename}.json'
    with open(data_path, 'w', encoding='utf-8') as f:
        json.dump(json_data, f, indent=2)


    return filename


def plot_pressure_data(filename, start_percent=0.2, end_percent=0.8):
    """
    读取JSON文件并绘制选定区间的压力数据分析图
    Args:
        filename: 包含触摸数据的JSON文件名
        start_percent: 数据开始位置的百分比（默认0.2，即跳过前20%的数据）
        end_percent: 数据结束位置的百分比（默认0.8，即跳过后20%的数据）
    Returns:
        生成的压力分析图文件名和分析结果
    """
    # 读取JSON数据
    json_path = result_dir / f'{filename}.json'
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 提取数据
    all_points = data['data']
    total_points = len(all_points)
    
    # 计算选取范围
    start_idx = int(total_points * start_percent)
    end_idx = int(total_points * end_percent)
    
    # 选取稳定区间的数据
    selected_data = all_points[start_idx:end_idx]
    
    # 提取时间和移动数据
    time_data = [point['relative_timestamp'] for point in selected_data]
    movement_x = [point['movement_x'] for point in selected_data]
    movement_y = [point['movement_y'] for point in selected_data]
    
    # 计算移动距离和速度
    movement_magnitude = [(x**2 + y**2)**0.5 for x, y in zip(movement_x, movement_y)]
    
    # 计算统计指标
    avg_magnitude = sum(movement_magnitude) / len(movement_magnitude)
    max_magnitude = max(movement_magnitude)
    min_magnitude = min(movement_magnitude)
    std_magnitude = (sum((x - avg_magnitude) ** 2 for x in movement_magnitude) / len(movement_magnitude)) ** 0.5
    
    # 创建图表
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 15))
    
    # 1. 移动轨迹图
    ax1.plot(movement_x, movement_y, 'b-', alpha=0.6, label='Movement Path')
    ax1.scatter(movement_x, movement_y, color='blue', alpha=0.4, s=30)
    ax1.set_title('Touch Movement Path')
    ax1.set_xlabel('X Movement')
    ax1.set_ylabel('Y Movement')
    ax1.grid(True)
    ax1.axis('equal')  # 保持横纵比例相同
    
    # 2. 移动距离时序图
    ax2.plot(time_data, movement_magnitude, 'g-', alpha=0.6, label='Movement Magnitude')
    ax2.scatter(time_data, movement_magnitude, color='green', alpha=0.4, s=30)
    ax2.axhline(y=avg_magnitude, color='r', linestyle='--', label=f'Average: {avg_magnitude:.2f}')
    ax2.fill_between(time_data, 
                     [avg_magnitude - std_magnitude] * len(time_data),
                     [avg_magnitude + std_magnitude] * len(time_data),
                     alpha=0.2, color='red', label=f'Std: ±{std_magnitude:.2f}')
    ax2.set_title('Movement Magnitude Over Time')
    ax2.set_xlabel('Relative Time (ms)')
    ax2.set_ylabel('Movement Magnitude')
    ax2.grid(True)
    ax2.legend()
    
    # 3. 移动方向分布图（极坐标）
    angles = [np.arctan2(y, x) for x, y in zip(movement_x, movement_y)]
    magnitudes = movement_magnitude
    ax3 = plt.subplot(313, projection='polar')
    ax3.scatter(angles, magnitudes, alpha=0.4)
    ax3.set_title('Movement Direction Distribution')
    
    # 添加统计信息
    stats_text = f"""
    Statistics (selected {end_percent-start_percent:.0%} of data):
    - Average Magnitude: {avg_magnitude:.2f}
    - Max Magnitude: {max_magnitude:.2f}
    - Min Magnitude: {min_magnitude:.2f}
    - Standard Deviation: {std_magnitude:.2f}
    """
    plt.figtext(0.1, 0.02, stats_text, fontsize=10, bbox=dict(facecolor='white', alpha=0.8))
    
    # 调整布局
    plt.tight_layout()
    
    # 保存图表
    pressure_filename = f'pressure_analysis_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png'
    pressure_filepath = result_dir / pressure_filename
    plt.savefig(pressure_filepath, bbox_inches='tight')
    plt.close()
    
    # 返回文件名和分析结果
    analysis_results = {
        'filename': pressure_filename,
        'stats': {
            'average_magnitude': avg_magnitude,
            'max_magnitude': max_magnitude,
            'min_magnitude': min_magnitude,
            'std_magnitude': std_magnitude,
            'data_points': len(selected_data),
            'selected_range': f"{start_percent:.0%} - {end_percent:.0%}"
        }
    }
    
    return analysis_results
def calculate_performance_score(time_data, movement_x, movement_y):
    """
    计算触控板性能得分
    Args:
        time_data: 时间序列数据
        movement_x: X轴移动距离
        movement_y: Y轴移动距离
    Returns:
        得分（0-10分）和详细分析数据
    """
    # 1. 计算基础指标
    time_intervals = np.diff(time_data)  # 时间间隔
    dx = np.diff(movement_x)  # X轴位移
    dy = np.diff(movement_y)  # Y轴位移

    min_interval = np.min(time_intervals)
    min_interval_count = np.sum(time_intervals == min_interval)
    print(f"BLE最小间隔: {min_interval:.2f} 毫秒, 次数: {min_interval_count}")
    # 最大间隔，以及最大间隔次数
    max_interval = np.max(time_intervals)
    max_interval_count = np.sum(time_intervals == max_interval)
    print(f"BLE最大间隔: {max_interval:.2f} 毫秒, 次数: {max_interval_count}")
    # 间隔标准差
    ble_interval_stddev = np.std(time_intervals)
    print(f"BLE间隔标准差: {ble_interval_stddev:.2f} 毫秒")

    # 计算位移和速度
    displacements = np.sqrt(dx**2 + dy**2)  # 位移大小
    velocities = np.where(time_intervals > 0, displacements / time_intervals, 0)  # 速度
    
    # 2. 计算统计指标
    avg_velocity = np.mean(velocities)
    std_velocity = np.std(velocities)
    cv = std_velocity / avg_velocity if avg_velocity > 0 else float('inf')  # 变异系数
    
    # 3. 检测异常事件
    # 定义阈值
    STALL_TIME_THRESHOLD = 50  # ms，停顿时间阈值
    STALL_DISPLACEMENT_THRESHOLD = 0.1  # 停顿位移阈值
    JUMP_VELOCITY_THRESHOLD = avg_velocity + 2 * std_velocity  # 突跳速度阈值
    
    # 检测停顿
    stalls = np.sum((time_intervals > STALL_TIME_THRESHOLD) & 
                    (displacements < STALL_DISPLACEMENT_THRESHOLD))
    
    # 检测突跳
    jumps = np.sum(velocities > JUMP_VELOCITY_THRESHOLD)
    
    # 计算异常事件比例
    total_points = len(time_intervals)
    abnormal_ratio = (stalls + jumps) / total_points
    
    # 4. 计算得分
    # 速度稳定性得分 (60%)
    CV_MAX = 1.0
    stability_score = 10 * max(0, 1 - cv / CV_MAX)
    
    # 异常事件得分 (40%)
    abnormal_score = 10 * (1 - min(1, abnormal_ratio))
    
    # 最终得分
    final_score = 0.6 * stability_score + 0.4 * abnormal_score
    
    # 准备详细分析结果
    analysis_results = {
        'final_score': round(final_score, 2),
        'stability_score': round(stability_score, 2),
        'abnormal_score': round(abnormal_score, 2),
        'metrics': {
            'cv': round(cv, 3),
            'avg_velocity': round(avg_velocity, 3),
            'std_velocity': round(std_velocity, 3),
            'stall_count': int(stalls),
            'jump_count': int(jumps),
            'abnormal_ratio': round(abnormal_ratio, 3),
            'total_points': total_points
        }
    }
    
    return analysis_results

def analysis_pressure_data(filename, start_percent=0.2, time_window=5000):
    """
    分析压力数据
    Args:
        filename: 包含触摸数据的JSON文件名
        start_percent: 起始位置的百分比（默认0.2）
        time_window: 要分析的时间窗口，单位为毫秒（默认5000ms）
    Returns:
        JSON格式分析结果
    """
    # 读取JSON数据
    json_path = result_dir / f'{filename}.json'
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 提取数据
    all_points = data['data']
    total_points = len(all_points)
    
    # 计算起始索引
    start_idx = int(total_points * start_percent)
    
    # 获取起始时间
    start_time = all_points[start_idx]['relative_timestamp']
    end_time = start_time + time_window
    
    # 选取时间窗口内的数据
    selected_data = [
        point for point in all_points[start_idx:]
        if start_time <= point['relative_timestamp'] <= end_time
    ]
    
    # 提取时间和移动数据
    time_data = [point['relative_timestamp'] for point in selected_data]
    movement_x = [point['movement_x'] for point in selected_data]
    movement_y = [point['movement_y'] for point in selected_data]
    
    # 计算性能得分
    performance_score = calculate_performance_score(time_data, movement_x, movement_y)
    print(performance_score)

     # 创建图形和坐标轴
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 12))

    # 上半部分：移动轨迹图
    ax1.scatter(time_data, movement_x, color='red', label='Movement X', zorder=3)
    ax1.scatter(time_data, movement_y, color='green', label='Movement Y', zorder=3)
    for x, y in zip(time_data, movement_x):
        ax1.vlines(x=x, ymin=0, ymax=y, colors='gray', linestyles='--', alpha=0.3, zorder=1)
    for x, y in zip(time_data, movement_y):
        ax1.vlines(x=x, ymin=0, ymax=y, colors='gray', linestyles='--', alpha=0.3, zorder=1)
    ax1.axhline(y=0, color='black', linestyle='-', alpha=0.3, zorder=2)
    ax1.set_xlabel('相对时间 (ms)')
    ax1.set_ylabel('移动距离')
    ax1.set_title(f'移动轨迹分析 (时间窗口: {time_window}ms)')
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # 下半部分：性能评分信息
    score_text = f"""
    触控板性能评分: {performance_score['final_score']}/10

    详细评分:
    - 速度稳定性得分: {performance_score['stability_score']}/10
    - 异常事件得分: {performance_score['abnormal_score']}/10

    关键指标:
    - 变异系数 (CV): {performance_score['metrics']['cv']}
    - 平均速度: {performance_score['metrics']['avg_velocity']}
    - 速度标准差: {performance_score['metrics']['std_velocity']}
    - 停顿次数: {performance_score['metrics']['stall_count']}
    - 突跳次数: {performance_score['metrics']['jump_count']}
    - 异常事件比例: {performance_score['metrics']['abnormal_ratio']}
    - 总采样点数: {performance_score['metrics']['total_points']}
    """
    ax2.text(0.05, 0.95, score_text,
             transform=ax2.transAxes,
             verticalalignment='top',
             fontsize=10,
             family='monospace')
    ax2.axis('off')

    # 保存图表
    output_filename = f'movement_analysis_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png'
    save_path = result_dir / output_filename
    plt.savefig(save_path, bbox_inches='tight', dpi=100)
    plt.close()
    
    # 返回分析结果
    return {
        'filename': output_filename,
        'performance_score': performance_score,
        'analysis': {
            'start_time': start_time,
            'end_time': end_time,
            'time_window': time_window,
            'data_points': len(selected_data)
        }
    }


@app.route('/touch', methods=['POST'])
def handle_touch():
    try:
        touch_data = request.get_json()
        print(touch_data)
        # 验证数据格式
        if not isinstance(touch_data, list):
            return jsonify({"error": "Invalid data format - expected list"}), 400
            
        # 记录接收到的数据
        logger.info(f"Received {len(touch_data)} touch events at {datetime.now()}")
        
        # 这里可以添加数据处理逻辑
        # for event in touch_data:
        #     logger.debug(f"Processing event: timestamp={event.get('timeStamp')}, "
        #                 f"movementX={event.get('movementX')}, "
        #                 f"movementY={event.get('movementY')}")
          # 绘制散点图并保存
        # filename = plot_movement_data(touch_data)
        # 将json存储到文件,文件名为时间戳 YYYY-MM-DD_HH-MM-SS
        filename = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        with open(f"{filename}.json", "w") as f:
            json.dump(touch_data, f)
        
        logger.info(f"Plot saved as {filename}")


        return jsonify({"status": "success", "message": f"Processed {len(touch_data)} events"}), 200
        
    except Exception as e:
        logger.error(f"Error processing request: {str(e)}")
        return jsonify({"error": str(e)}), 500




if __name__ == '__main__':
    # print('start')
    # app.run(host='0.0.0.0', port=5000, debug=True)

    # 使用默认范围（中间60%的数据）
    # results = plot_pressure_data("movement_plot_20250513_183932.png",0.7,0.8)
    # print(results)
    #
    #
    result = analysis_pressure_data("baseus",0.68,3000)
    print(result)

    # 解析触控板原始数据
    # result = parse_raw_touch_data(r"D:\Code\VScode\example\python\touch\touch_raw_data\record_20250804_143434\trackData.txt")
    # for point in result:
    #     print(point)

    print('end')