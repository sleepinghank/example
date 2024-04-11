#!C:\pythonCode
# -*- coding: utf-8 -*-
# @Time : 2024/2/21 13:38
# @Author : hlx
# @File : filter.py
# @Software: PyCharm

R = 1


def kalman_filter(x, p, z):
    k = p / (p + R)
    x2 = x + k * (z - x)
    p2 = (1.0 - k) * p
    return x2, p2


if __name__ == '__main__':

    import matplotlib
    from matplotlib import pyplot as plt
    import numpy as np

    matplotlib.use('TkAgg')

    from filterpy.kalman import KalmanFilter
    from filterpy.common import Q_discrete_white_noise

    from port.port_serial import get_port_serial

    pipe = get_port_serial('COM20', 115200)

    # 创建实时绘制横纵轴变量
    arr_x = []
    arr_y = []
    arr_filter_y = []

    # pressure,压力值
    my_filter = KalmanFilter(dim_x=2, dim_z=1)
    my_filter.x = np.array([[0.],
                            [0.]])  # initial state (location and velocity) 先验估计初始化

    my_filter.F = np.array([[1., 1.],
                            [0., 1.]])  # state transition matrix 状态转移矩阵 和 实际运行函数相关

    my_filter.H = np.array([[1., 0.]])  # Measurement function 测量方程
    my_filter.P *= 1.  # coariance matrixv
    my_filter.R = 14.9  # state uncertainty
    my_filter.Q = Q_discrete_white_noise(dim=2, dt=0.1, var=0.1)  # process uncertainty
    print(f"my_filter.Q:{my_filter.Q}")
    # 创建绘制实时损失的动态窗口
    plt.ion()
    i = 0

    arr_data = []
    # 从管道中读取数据并处理
    while True:
        if not pipe.empty():
            data = int(pipe.get())
            # arr_data.append(data)
            #
            # if len(arr_data) >= 1000:
            #     # 将列表转换为 NumPy 数组
            #     data_array = np.array(arr_data)
            #
            #     # 计算数据的标准差
            #     covariance_matrix = np.cov(data_array)
            #
            #     print("测量值的协方差矩阵：", covariance_matrix)
            #     break
            my_filter.predict()
            my_filter.update(data)

            # do something with the output

            x = my_filter.x[0][0]
            print(f"data:{data},x:{x}")
            arr_filter_y.append(x)

            i += 1
            arr_x.append(i)  # 添加i到x轴的数据中
            arr_y.append(data)  # 添加i的平方到y轴的数据中
            plt.clf()  # 清除之前画的图

            # 绘制散点图
            plt.scatter(arr_x, arr_y, color='blue', label='Scatter')
            # 绘制折线图
            plt.plot(arr_x, arr_filter_y, color='red', linestyle='-', marker='', markersize=0)
            plt.pause(0.001)  # 暂停一段时间，不然画的太快会卡住显示不出来
            plt.ioff()  # 关闭画图窗口
