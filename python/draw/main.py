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

if __name__ == '__main__':
    # coordinates = [[360,136],[357,129],[351,121],[345,113],[339,105],[331,99],[323,94],[315,88],[307,83],[299,78],[291,75],[283,74],[275,74],[267,73],[259,73],[251,73],[243,73],[235,73],[227,73],[219,74],[211,75],[203,77],[195,80],[187,83],[179,87],[171,92],[163,96],[155,104],[149,112],[143,120],[138,128],[136,136],[135,144],[134,152],[135,160],[135,168],[137,176],[140,184],[147,192],[155,200],[163,207],[171,213],[179,218],[187,222],[195,225],[203,227],[211,229],[219,232],[227,234],[235,235],[243,238],[251,240],[259,242],[267,244],[275,246],[283,248],[291,250],[299,252],[307,254],[315,257],[323,260],[331,264],[339,269],[347,275],[355,283],[362,291],[368,299],[372,307],[374,315],[376,323],[377,331],[377,339],[377,347],[375,355],[371,363],[367,371],[362,379],[357,387],[351,395],[344,401],[336,407],[328,412],[320,415],[312,418],[304,422],[296,423],[288,425],[280,426],[272,426],[264,427],[256,427],[248,427],[240,426],[232,426],[224,424],[216,422],[208,420],[200,417],[192,414],[184,411],[176,408],[168,404],[160,399],[152,393],[145,385],[138,377],[133,369],[129,361],[125,353],[123,345]]
    coordinates = [[-0.222236,-0.368792],[0.007756,-0.368792],[0.235138,-0.333119],[0.061991,-0.101096],[-0.124497,0.130926],[-0.201064,0.340326],[0.030958,0.345837],[0.262981,0.345837]]
    plot_coordinates(coordinates)
    