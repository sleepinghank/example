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
if __name__ == '__main__':
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
    