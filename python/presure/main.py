
import json
import numpy as np  # 添加 numpy 导入
import matplotlib.pyplot as plt  # 添加 matplotlib 导入
from pathlib import Path

from gplearn.genetic import SymbolicRegressor
from sklearn.model_selection import train_test_split

data_dir = Path('../data/presure')


# 中值滤波
def median_filter(signal, window_size=3, initial_value=None):
    if initial_value is None:
        initial_value = signal[0]  # 默认用第一个值填充
    buffer = [initial_value] * window_size
    result = []
    for i in range(len(signal)):
        buffer[i % window_size] = signal[i]
        sorted_buffer = sorted(buffer)
        result.append(sorted_buffer[window_size // 2])
    return result

def low_pass_filter(raw, alpha):
    filtered = np.zeros_like(raw)
    filtered[0] = raw[0]  # 初始化首值
    for i in range(1, len(raw)):
        filtered[i] = alpha * raw[i] + (1-alpha) * filtered[i-1]
    return filtered


def filter_all_data(filename):
    with open(data_dir / file_name, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # 提取数据
    x_values = []
    force_l_values = []
    force_r_values = []

    for item in data:
        raw_data = item['raw_data']
        if raw_data['x'] > 1400:
            break
        x_values.append(raw_data['x'])
        force_l_values.append(raw_data['force_l'])
        force_r_values.append(raw_data['force_r'])

    # 组合滤波

    force_l_fil_values = low_pass_filter(median_filter(force_l_values, window_size=2, initial_value=force_l_values[0]),
                                         alpha=0.3)
    force_r_fil_values = low_pass_filter(median_filter(force_r_values, window_size=2, initial_value=force_r_values[0]),
                                         alpha=0.3)
    return x_values, force_l_fil_values, force_r_fil_values

def draw_data(file_name):
    with open(data_dir / file_name, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 提取数据
    x_values = []
    force_l_values = []
    force_r_values = []
    
    for item in data:
        raw_data = item['raw_data']
        if raw_data['x'] > 1400:
            break
        x_values.append(raw_data['x'])
        force_l_values.append(raw_data['force_l'])
        force_r_values.append(raw_data['force_r'])

    print(f"文件 {file_name} 的 force_l_values:")
    print("{" + ", ".join(map(str, force_l_values)) + "};")
    print(f"文件 {file_name} 的 force_r_values:")
    print("{" + ", ".join(map(str, force_r_values)) + "};")
    # 组合滤波
    # force_l_fil_values = iir_lowpass( median_filter(force_l_values, window_size=3), alpha=0.1)
    # force_r_fil_values = iir_lowpass( median_filter(force_r_values, window_size=3), alpha=0.1)
    # force_l_fil_values = median_filter(force_l_values, window_size=3)
    # force_r_fil_values = median_filter(force_r_values, window_size=3)
    force_l_fil_values = low_pass_filter(median_filter(force_l_values, window_size=2,initial_value=force_l_values[0]), alpha=0.3)
    force_r_fil_values = low_pass_filter(median_filter(force_r_values, window_size=2,initial_value=force_r_values[0]), alpha=0.3)

    # 创建图形
    plt.figure(figsize=(12, 8))
    
    # 绘制force_l的折线图
    plt.subplot(2, 1, 1)
    plt.plot(x_values, force_l_values, 'b-', linewidth=1, label='Force Left')
    plt.plot(x_values, force_l_fil_values, 'r-', linewidth=1, label='Force Left Fil')
    plt.xlabel('X坐标')
    plt.ylabel('Force Left')
    plt.title(f'{file_name} - Force Left vs X')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    # 绘制force_r的折线图
    plt.subplot(2, 1, 2)
    plt.plot(x_values, force_r_values, 'r-', linewidth=1, label='Force Right')
    plt.plot(x_values, force_r_fil_values, 'g-', linewidth=1, label='Force Right Fil')
    plt.xlabel('X坐标')
    plt.ylabel('Force Right')
    plt.title(f'{file_name} - Force Right vs X')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    # 保存图片
    output_path = Path('result') / f'{file_name.replace(".json", "_force_plot.png")}'
    output_path.parent.mkdir(exist_ok=True)
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.show()
    
    print(f"图表已保存到: {output_path}")
    
    return x_values, force_l_values, force_r_values


def parse_raw_data():
    # 从data\presure\目录下读取四个文件夹内所有数据，分为四个数组

    x_list = []
    y_list = []

    # datafile_name = ['data_20250715_143108_670_10size.json', 'data_20250715_143412_338_10plus11size.json', 'data_20250715_145929_635_10plus15.json', 'data_20250715_151434_628_10plus11plus.json']
    datafile_name = [ 'data_20250715_151434_628_10plus11plus.json']
    for i, item in enumerate(datafile_name):
        with open(data_dir / item, 'r', encoding='utf-8') as f:
            data = json.load(f)
            
            # 为每个文件收集原始数据
            temp_x_values = []
            temp_force_l_values = []
            temp_force_r_values = []
            
            for data_item in data:
                raw_data = data_item['raw_data']
                if raw_data['x'] > 1400:
                    break
                temp_x_values.append(raw_data['x'])
                temp_force_l_values.append(raw_data['force_l'])
                temp_force_r_values.append(raw_data['force_r'])
            
            
            
            # 对force_l和force_r进行滤波处理
            if len(temp_force_l_values) > 0:
                # 输出原始数据
                print(f"文件 {item} 的 temp_force_l_values:")
                print("{" + ", ".join(map(str, temp_force_l_values)) + "};")
                print(f"文件 {item} 的 temp_force_r_values:")
                print("{" + ", ".join(map(str, temp_force_r_values)) + "};")
                
                force_l_filtered = low_pass_filter(
                    median_filter(temp_force_l_values, window_size=2, initial_value=temp_force_l_values[0]), 
                    alpha=0.3
                )
                force_r_filtered = low_pass_filter(
                    median_filter(temp_force_r_values, window_size=2, initial_value=temp_force_r_values[0]), 
                    alpha=0.3
                )
                # force_l_filtered = {618,612,607,603,574,520,506,506,518,512,506,504,502,504,560,598,604,602,603,608,616,618,632,639,654,660,662,661,659,657,655,653,651,649,645,639,633,625,621,635,636,633,633,623,634,645,647,646,645,642,641,642,641,640,642,644,649,658,676,683,693,713,745,824,820,758,731,698,678,664,658,654,650,644,646,650,652,651,644,642,640,640,642,645,648,654,660,664,664,661,657,653,656,659,661,662,667,685,718,800,913,990,1139,1200,1235,1244,1241,1212,1052,962,828,771,730,705,695,691,684,675,671,673,678,680,679,677,671,668,661,667,669,672,674,676,678,683,687,686,685,687,694,708,736,760,914,1146,1236,1219,1180,1036,945,783,737,707,694,688};
                # force_r_filtered = {8,49,65,134,232,547,657,694,737,775,768,740,685,627,316,159,108,96,90,64,79,97,260,342,430,519,620,695,749,763,749,715,657,577,476,353,261,115,75,26,21,31,32,38,25,8,6,15,25,40,46,48,48,47,49,58,64,62,75,99,146,221,307,481,430,283,210,156,124,94,77,67,57,47,14,7,9,11,27,34,32,31,31,32,31,12,1,-2,0,9,21,23,23,21,21,21,23,16,9,25,75,102,151,169,178,180,178,168,93,64,49,43,34,29,25,23,19,19,19,5,-9,-14,-12,-7,3,9,11,10,9,7,8,6,-8,-25,-24,-8,0,1,0,-3,-9,-17,-71,-119,-104,-105,-105,-84,-77,-42,-31,-13,-7,-5}
                # 使用滤波后的数据构建特征
                for j in range(len(temp_x_values)):
                    feature = [
                        # raw_data['y'],
                        temp_x_values[j],
                        force_l_filtered[j],
                        force_r_filtered[j]
                    ]
                    x_list.append(feature)
                    
                    # if i == 0:
                    #     y_list.append(13.2)
                    # elif i == 1:
                    #     y_list.append(13.2 + 15.9)
                    # elif i == 2:
                    #     y_list.append(13.2 + 37)
                    # else:
                    y_list.append(13.2 + 15.9 + 121.4)
    
    X_train = np.array(x_list)
    y_train = np.array(y_list)
    return X_train, y_train


def train_model(X_data, y_data):
    # 将数据区分为训练集和测试集
    X_train, X_test, y_train, y_test = train_test_split(X_data, y_data, test_size=0.2, random_state=42)

    for i, feature in enumerate(X_train[:200]):
        print(f"数据{i + 1}: {feature} , {y_train[i]}")
    est_gp = SymbolicRegressor(population_size=5000,
                           generations=30, stopping_criteria=0.01,
                           p_crossover=0.7, p_subtree_mutation=0.1,
                           p_hoist_mutation=0.05, p_point_mutation=0.1,
                           max_samples=0.9, verbose=1,
                           parsimony_coefficient=0.01, random_state=0)
    est_gp.fit(X_train, y_train)

     # 根据测试集测试得分

    score_gp = est_gp.score(X_test, y_test)
    print("score_gp", score_gp)
    y_pred = est_gp.predict(X_test)
    for i, item in enumerate(y_pred):
        print(f"{i}:{item},{y_test[i]}")
    print(est_gp)

if __name__ == '__main__':
    print("start")

    # draw_data('data_20250716_161838_319_2870998448.json');
    # # # 绘制数据图表
    # # datafile_name = ['data_20250715_143108_670_10size.json', 'data_20250715_143412_338_10plus11size.json', 'data_20250715_145929_635_10plus15.json', 'data_20250715_151434_628_10plus11plus.json']
    # #
    # # print("开始绘制折线图...")
    # # for file_name in datafile_name:
    # #     print(f"正在处理文件: {file_name}")
    # #     try:
    # #         x_values, force_l_values, force_r_values = filter_all_data(file_name)
    # #         print(f"成功绘制 {file_name}，数据点数量: {len(x_values)}")
    # #     except Exception as e:
    # #         print(f"处理文件 {file_name} 时出错: {e}")
    # #
    print("开始训练模型...")
    X_train, y_train = parse_raw_data()
    print("len X_train", len(X_train))
    train_model(X_train, y_train)
