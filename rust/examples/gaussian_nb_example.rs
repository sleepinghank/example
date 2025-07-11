use crate::gaussian_nb::GaussianNB;

fn main() {
    println!("=== 高斯朴素贝叶斯分类器示例 ===");
    
    // 创建模型参数
    let priors = vec![0.5, 0.5]; // 两个类别的先验概率
    let sigmas = vec![
        vec![1.0, 1.0], // 类别0的特征方差
        vec![1.0, 1.0]  // 类别1的特征方差
    ];
    let thetas = vec![
        vec![0.0, 0.0], // 类别0的特征均值
        vec![2.0, 2.0]  // 类别1的特征均值
    ];
    
    // 创建分类器
    let clf = GaussianNB::new(priors, sigmas, thetas);
    
    // 测试数据
    let test_features = vec![
        vec![0.1, 0.1],   // 应该预测为类别0
        vec![2.1, 2.1],   // 应该预测为类别1
        vec![1.0, 1.0],   // 边界情况
    ];
    
    println!("\n--- 预测结果 ---");
    for (i, features) in test_features.iter().enumerate() {
        let prediction = clf.predict(features);
        let probabilities = clf.predict_proba(features);
        
        println!("测试样本 {}: {:?}", i + 1, features);
        println!("  预测类别: {}", prediction);
        println!("  类别概率: {:?}", probabilities);
        println!();
    }
    
    // 演示数值稳定性
    println!("--- 数值稳定性测试 ---");
    let large_features = vec![1000.0, 1000.0];
    let probabilities = clf.predict_proba(&large_features);
    println!("大数值特征: {:?}", large_features);
    println!("概率结果: {:?}", probabilities);
    println!("概率和: {:.6}", probabilities.iter().sum::<f64>());
    
    // 演示JSON存储的临时路径生成
    println!("\n--- JSON存储临时路径生成演示 ---");
    use rust::json_storage::JsonStorage;
    
    for i in 1..=3 {
        let temp_path = JsonStorage::generate_temp_path("temp", "session", "json");
        println!("临时路径 {}: {}", i, temp_path);
    }
    
    let temp_storage = JsonStorage::new_temp("temp", "cache", "json");
    println!("临时存储路径: {}", temp_storage.get_file_path());
    
    // 保存一些数据到临时文件
    let cache_data = vec!["缓存项1", "缓存项2", "缓存项3"];
    temp_storage.save(&cache_data).unwrap();
    println!("数据已保存到临时文件");
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_basic_classification() {
        let priors = vec![0.5, 0.5];
        let sigmas = vec![
            vec![1.0, 1.0],
            vec![1.0, 1.0]
        ];
        let thetas = vec![
            vec![0.0, 0.0],
            vec![2.0, 2.0]
        ];
        
        let clf = GaussianNB::new(priors, sigmas, thetas);
        
        // 测试靠近类别0的样本
        let features = vec![0.1, 0.1];
        let prediction = clf.predict(&features);
        assert_eq!(prediction, 0);
        
        // 测试靠近类别1的样本
        let features = vec![2.1, 2.1];
        let prediction = clf.predict(&features);
        assert_eq!(prediction, 1);
    }
    
    #[test]
    fn test_probability_sum() {
        let priors = vec![0.3, 0.7];
        let sigmas = vec![
            vec![1.0, 1.0],
            vec![1.0, 1.0]
        ];
        let thetas = vec![
            vec![0.0, 0.0],
            vec![1.0, 1.0]
        ];
        
        let clf = GaussianNB::new(priors, sigmas, thetas);
        let features = vec![0.5, 0.5];
        let probabilities = clf.predict_proba(&features);
        
        // 概率和应该接近1.0
        let sum: f64 = probabilities.iter().sum();
        assert!((sum - 1.0).abs() < 1e-10);
    }
} 