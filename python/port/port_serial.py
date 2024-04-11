#!C:\pythonCode
# -*- coding: utf-8 -*-
# @Time : 2024/2/21 13:23
# @Author : hlx
# @File : port_serial.py
# @Software: PyCharm

import threading
import queue
import serial


def get_port_serial(port, baud_rate):
    # 串口配置
    ser = serial.Serial(port, baud_rate)  # 替换 'COM1' 为你连接的串口号，9600 是波特率

    # 管道
    pipe = queue.Queue()

    # 读取串口数据并放入管道
    def read_serial_data():
        while True:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()
                pipe.put(data)
                print("Received data:", data)
    # 启动线程
    serial_thread = threading.Thread(target=read_serial_data)
    serial_thread.daemon = True
    serial_thread.start()

    return pipe


if __name__ == '__main__':
    pipe = get_port_serial('COM20', 115200)

    # 从管道中读取数据并处理
    while True:
        if not pipe.empty():
            data = pipe.get()
            print("Received data:", data)
