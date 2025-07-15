
import json
import numpy as np  # 添加 numpy 导入
from pathlib import Path

from gplearn.genetic import SymbolicRegressor
from sklearn.model_selection import train_test_split


def parse_raw_data():
    # 从data\presure\目录下读取四个文件夹内所有数据，分为四个数组
    data_dir = Path('../data/presure')
    x_list = []
    y_list = []

    datafile_name = ['data_20250715_143108_670_10size.json', 'data_20250715_143412_338_10plus11size.json', 'data_20250715_145929_635_10plus15.json', 'data_20250715_151434_628_10plus11plus.json']
    for i, item in enumerate(datafile_name):
        with open(data_dir / item, 'r', encoding='utf-8') as f:
            data = json.load(f)
            for data_item in data:
                raw_data = data_item['raw_data']
                feature = [
                    # raw_data['y'],
                    raw_data['x'],
                    raw_data['force_l'],
                    raw_data['force_r']
                ]
                x_list.append(feature)
                if i == 0:
                    y_list.append(13.2)
                elif i == 1:
                    y_list.append(13.2 + 15.9)
                elif i == 2:
                    y_list.append(13.2 + 37)
                else:
                    y_list.append(13.2 + 15.9 + 121.4)
    X_train = np.array(x_list)
    y_train = np.array(y_list)
    return X_train, y_train


if __name__ == '__main__':
    print("start")

    X_train, y_train = parse_raw_data()
    print("len X_train", len(X_train))
    # 将数据区分为训练集和测试集
    X_train, X_test, y_train, y_test = train_test_split(X_train, y_train, test_size=0.2, random_state=42)

    for i, feature in enumerate(X_train[:200]):
        print(f"数据{i + 1}: {feature} , {y_train[i]}")
    est_gp = SymbolicRegressor(population_size=5000,
                           generations=100, stopping_criteria=0.01,
                           p_crossover=0.7, p_subtree_mutation=0.1,
                           p_hoist_mutation=0.05, p_point_mutation=0.1,
                           max_samples=0.9, verbose=1,
                           parsimony_coefficient=0.01, random_state=0)
    est_gp.fit(X_train, y_train)

     # 根据测试集测试得分

    score_gp = est_gp.score(X_test, y_test)
    print("score_gp", score_gp)
    # y_pred = est_gp.predict(X_test)
    # for i, item in y_pred:
    #     print(f"{i}:{item},{y_test[i]}")
    print(est_gp)
