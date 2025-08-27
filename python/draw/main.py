#!C:\pythonCode
# -*- coding: utf-8 -*-
# @Time : 2024/2/21 11:48
# @Author : hlx
# @File : main.py
# @Software: PyCharm

from matplotlib import pyplot as plt
import numpy as np

import matplotlib

def com_pol():
    # 创建实时绘制横纵轴变量
    arr_x = []
    arr_y = []
    arr_filter_y = []

    # 创建绘制实时损失的动态窗口
    plt.ion()

    from port.port_serial import get_port_serial

    pipe = get_port_serial('COM20', 115200)

    idx = 0
    # 从管道中读取数据并处理
    while True:
        if not pipe.empty():
            idx += 1

            data = pipe.get()
            # 初始化空列表来存储解析后的数据
            x = 0
            y = 0.0

            # 按行拆分文本数据并解析每个数据点
            for line in data.strip().split('\n'):
                values = line.split(',')
                if len(values) == 2:
                    x, y = float(values[0]), float(values[1])

            arr_filter_y.append(y)

            arr_x.append(idx)  # 添加i到x轴的数据中
            arr_y.append(x)  # 添加i的平方到y轴的数据中
            plt.clf()  # 清除之前画的图

            # 绘制散点图
            plt.scatter(arr_x, arr_y, color='blue', label='Scatter')
            # 绘制折线图
            plt.plot(arr_x, arr_filter_y, color='red', linestyle='-', marker='', markersize=0)
            plt.pause(0.001)  # 暂停一段时间，不然画的太快会卡住显示不出来
            plt.ioff()  # 关闭画图窗口

# 定义一个自定义函数来生成数据点
def custom_function(x):
    # x < 2750 y = 0
    # 2750 < x < 3350 y=32*x/25 - 3529
    # 3350 < x < 3650 y=1697*x/100 - 56070
    # 3650 < x < 4050 y=191*x/20 - 28997
    # 4050 < x < 4150 y=16*x/5 - 3280
    # 4150 < x  y=100
    y = np.piecewise(x, [x < 2750, (x >= 2750) & (x < 3350), (x >= 3350) & (x < 3650), (x >= 3650) & (x < 4050), (x >= 4050) & (x < 4150), x >= 4150],
                     [0, lambda x: (32 * x / 25 - 3529)/100, lambda x: (1697 * x / 100 - 56070)/100, lambda x: (191 * x / 20 - 28997)/100, lambda x: (16 * x / 5 - 3280)/100, 100])
    return y

matplotlib.use('TkAgg')

def test():
    # 根据函数绘制折线图
    # 生成数据点
    x = np.linspace(2750, 4250, 10000)
    y = custom_function(x)

    # y 轴反转

    # 绘制折线图
    plt.plot(y, x, label='sin(x) * exp(-x / 5)')
    plt.xlabel('X轴')
    plt.ylabel('Y轴')
    plt.title('自定义函数的折线图')
    plt.legend()
    plt.grid(True)

    # 显示图形
    plt.show()


def test_plot_coordinates(data_list):
    
    # 将一维数组转换为二维坐标
    x_coords = data_list[::2]  # 偶数索引为x坐标
    y_coords = data_list[1::2]  # 奇数索引为y坐标
    
    # x 轴翻转
    x_coords = x_coords[::-1]

    # 绘制折线图
    plt.figure(figsize=(10, 6))
    plt.plot(x_coords, y_coords, 'o-', linewidth=2, markersize=6)
    
    # 添加标题和标签
    plt.title('二维坐标折线图')
    plt.xlabel('X轴')
    plt.ylabel('Y轴')
    plt.grid(True)
    
    # 添加坐标点标签
    for i, (x, y) in enumerate(zip(x_coords, y_coords)):
        plt.annotate(f'P[i]', (x, y), textcoords="offset points", 
                    xytext=(0, 10), ha='center')
    
    # 显示图形
    plt.show()

def plot_coordinates(coordinates):
    """
    绘制二维坐标折线图
    
    参数:
    coordinates -- 二维数组，每个元素是一个坐标点 [x, y]，例如 [[1125, 1321], [1200, 1400], ...]
    """
    # 提取x和y坐标
    x_coords = [point[0] for point in coordinates]
    y_coords = [point[1] for point in coordinates]
    
    # 绘制折线图
    plt.figure(figsize=(10, 6))
    plt.plot(x_coords, y_coords, 'o-', linewidth=2, markersize=6)
    
    # 添加标题和标签
    plt.title('二维坐标折线图')
    plt.xlabel('X轴')
    plt.ylabel('Y轴')
    plt.grid(True)
    
    # 添加坐标点标签
    # for i, (x, y) in enumerate(zip(x_coords, y_coords)):
    #     plt.annotate(f'P[i]', (x, y), textcoords="offset points",
    #                 xytext=(0, 10), ha='center')
    
    # 显示图形
    plt.show()


import json
import matplotlib.pyplot as plt
import numpy as np

def read_yuan_data(file_path):
    """
    读取yuan.json文件中的数据
    
    参数:
    file_path -- JSON文件路径
    
    返回:
    data -- 解析后的JSON数据
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        print(f"成功读取数据，共{len(data)}个数据点")
        return data
    except FileNotFoundError:
        print(f"错误：找不到文件 {file_path}")
        return None
    except json.JSONDecodeError as e:
        print(f"错误：JSON解析失败 - {e}")
        return None
    except Exception as e:
        print(f"错误：读取文件时发生异常 - {e}")
        return None

def plot_yuan_data(data):
    """
    绘制yuan数据的折线图
    
    参数:
    data -- JSON数据列表
    """
    if not data:
        print("没有数据可以绘制")
        return
    
    # 提取x和y坐标
    x_coords = [point['x'] for point in data]
    y_coords = [point['y'] for point in data]
    scan_times = [point['scan_time'] for point in data]
    # contact_counts = [point['contact_count'] for point in data]
    
    # 创建图形
    plt.figure(figsize=(15, 10))
    
    # 创建子图1：x-y坐标折线图
    plt.subplot(2, 2, 1)
    plt.plot(x_coords, y_coords, 'o-', linewidth=2, markersize=4, color='blue', alpha=0.7)
    plt.title('X-Y坐标折线图')
    plt.xlabel('X坐标')
    plt.ylabel('Y坐标')
    plt.grid(True, alpha=0.3)
    
    # 创建子图2：x坐标随时间变化
    plt.subplot(2, 2, 2)
    plt.plot(scan_times, x_coords, 'o-', linewidth=2, markersize=4, color='red', alpha=0.7)
    plt.title('X坐标随时间变化')
    plt.xlabel('扫描时间 (ms)')
    plt.ylabel('X坐标')
    plt.grid(True, alpha=0.3)
    
    # 创建子图3：y坐标随时间变化
    plt.subplot(2, 2, 3)
    plt.plot(scan_times, y_coords, 'o-', linewidth=2, markersize=4, color='green', alpha=0.7)
    plt.title('Y坐标随时间变化')
    plt.xlabel('扫描时间 (ms)')
    plt.ylabel('Y坐标')
    plt.grid(True, alpha=0.3)
    
    # 创建子图4：接触计数统计
    plt.subplot(2, 2, 4)
    contact_count_hist = {}
    # for count in contact_counts:
    #     contact_count_hist[count] = contact_count_hist.get(count, 0) + 1
    #
    # plt.bar(contact_count_hist.keys(), contact_count_hist.values(), color='orange', alpha=0.7)
    plt.title('接触计数统计')
    plt.xlabel('接触计数')
    plt.ylabel('频次')
    plt.grid(True, alpha=0.3)
    
    # 调整子图间距
    plt.tight_layout()
    
    # 显示图形
    plt.show()
    
    # 打印数据统计信息
    print(f"\n数据统计信息:")
    print(f"数据点总数: {len(data)}")
    print(f"X坐标范围: {min(x_coords)} - {max(x_coords)}")
    print(f"Y坐标范围: {min(y_coords)} - {max(y_coords)}")
    print(f"扫描时间范围: {min(scan_times)} - {max(scan_times)} ms")
    print(f"总扫描时间: {max(scan_times) - min(scan_times)} ms")
    print(f"接触计数统计: {contact_count_hist}")

def plot_simple_xy(data):
    """
    绘制简单的x-y坐标折线图
    
    参数:
    data -- JSON数据列表
    """
    if not data:
        print("没有数据可以绘制")
        return
    
    # 提取x和y坐标
    x_coords = [point['x'] for point in data]
    y_coords = [point['y'] for point in data]
    
    # 创建图形
    plt.figure(figsize=(12, 8))
    
    # 绘制折线图
    plt.plot(x_coords, y_coords, 'o-', linewidth=2, markersize=4, color='blue', alpha=0.7)
    
    # 添加标题和标签
    plt.title('X-Y坐标折线图 (yuan.json数据)', fontsize=14)
    plt.xlabel('X坐标', fontsize=12)
    plt.ylabel('Y坐标', fontsize=12)
    plt.grid(True, alpha=0.3)
    
    # 添加数据点数量标注
    plt.text(0.02, 0.98, f'数据点数量: {len(data)}', 
             transform=plt.gca().transAxes, fontsize=10,
             verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    # 显示图形
    plt.show()


if __name__ == '__main__':
    # coordinates = [[360,136],[357,129],[351,121],[345,113],[339,105],[331,99],[323,94],[315,88],[307,83],[299,78],[291,75],[283,74],[275,74],[267,73],[259,73],[251,73],[243,73],[235,73],[227,73],[219,74],[211,75],[203,77],[195,80],[187,83],[179,87],[171,92],[163,96],[155,104],[149,112],[143,120],[138,128],[136,136],[135,144],[134,152],[135,160],[135,168],[137,176],[140,184],[147,192],[155,200],[163,207],[171,213],[179,218],[187,222],[195,225],[203,227],[211,229],[219,232],[227,234],[235,235],[243,238],[251,240],[259,242],[267,244],[275,246],[283,248],[291,250],[299,252],[307,254],[315,257],[323,260],[331,264],[339,269],[347,275],[355,283],[362,291],[368,299],[372,307],[374,315],[376,323],[377,331],[377,339],[377,347],[375,355],[371,363],[367,371],[362,379],[357,387],[351,395],[344,401],[336,407],[328,412],[320,415],[312,418],[304,422],[296,423],[288,425],[280,426],[272,426],[264,427],[256,427],[248,427],[240,426],[232,426],[224,424],[216,422],[208,420],[200,417],[192,414],[184,411],[176,408],[168,404],[160,399],[152,393],[145,385],[138,377],[133,369],[129,361],[125,353],[123,345]]
    # coordinates = [[-0.222236,-0.368792],[0.007756,-0.368792],[0.235138,-0.333119],[0.061991,-0.101096],[-0.124497,0.130926],[-0.201064,0.340326],[0.030958,0.345837],[0.262981,0.345837]]
    # plot_coordinates(coordinates)
     # 读取JSON文件
    # 读取JSON文件
    json_file_path = 'data/yuan.json'
    data = read_yuan_data(json_file_path)

    if data:
        # 绘制详细的四子图
        print("绘制详细分析图...")
        plot_yuan_data(data)

        # 绘制简单的x-y折线图
        print("\n绘制简单X-Y折线图...")
        plot_simple_xy(data)
    # import math
    #
    # # 参数配置
    # total_time = 1000  # 总时间(ms)
    # interval = 15  # 间隔(ms)
    # radius = 360  # 半径(单位100)
    # points = 360  # 总点数
    #
    # # 生成圆坐标数组
    # circle_points = []
    # for i in range(points):
    #     # 计算当前角度 (0 到 2π)
    #     angle = 2 * math.pi * i / points
    #
    #     # 计算坐标 (单位100)
    #     x = radius * math.cos(angle)
    #     y = radius * math.sin(angle)
    #
    #     # 转换为整数 (单位100)
    #     x_int = int(round(x))
    #     y_int = int(round(y))
    #
    #     circle_points.append((x_int, y_int))
    #
    # # 输出C数组代码
    # print("const int16_t circle_points[][2] = {")
    # for i, (x, y) in enumerate(circle_points):
    #     print(f"    {{{x}, {y}}},", end='')
    #     if i % 3 == 2:  # 每3个点换行
    #         print()
    # print("};")