#!C:\pythonCode
# -*- coding: utf-8 -*-
# @Time : 2024/2/21 11:48
# @Author : hlx
# @File : main.py
# @Software: PyCharm

from matplotlib import pyplot as plt
import numpy as np

import matplotlib

matplotlib.use('TkAgg')
if __name__ == '__main__':
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
