use std::f64::consts::PI;
use std::env;

pub struct GaussianNB {
    priors: Vec<f64>,
    sigmas: Vec<Vec<f64>>,
    thetas: Vec<Vec<f64>>,
}

impl GaussianNB {
    pub fn new(priors: Vec<f64>, sigmas: Vec<Vec<f64>>, thetas: Vec<Vec<f64>>) -> Self {
        GaussianNB {
            priors,
            sigmas,
            thetas,
        }
    }

    fn find_max(&self, nums: &[f64]) -> usize {
        let mut max_index = 0;
        for (i, &num) in nums.iter().enumerate() {
            if num > nums[max_index] {
                max_index = i;
            }
        }
        max_index
    }

    fn log_sum_exp(&self, nums: &[f64]) -> f64 {
        let max = nums[self.find_max(nums)];
        let sum: f64 = nums.iter().map(|&x| (x - max).exp()).sum();
        max - sum.ln()
    }

    pub fn compute(&self, features: &[f64]) -> Vec<f64> {
        let mut likelihoods = vec![0.0; self.sigmas.len()];

        for (i, (sigma_row, theta_row)) in self.sigmas.iter().zip(&self.thetas).enumerate() {
            // 计算正态分布的对数概率密度
            let mut log_prob = 0.0;

            // 第一部分: 常数项和方差项
            for j in 0..sigma_row.len() {
                log_prob += (2.0 * PI * sigma_row[j]).ln();
            }
            log_prob = -0.5 * log_prob;

            // 第二部分: 马氏距离
            let mut sum = 0.0;
            for j in 0..features.len() {
                let diff = features[j] - theta_row[j];
                sum += diff * diff / sigma_row[j];
            }
            log_prob -= 0.5 * sum;

            // 加上先验概率的对数
            likelihoods[i] = self.priors[i].ln() + log_prob;
        }
        likelihoods
    }

    pub fn predict(&self, features: &[f64]) -> usize {
        self.find_max(&self.compute(features))
    }

    pub fn predict_proba(&self, features: &[f64]) -> Vec<f64> {
        let mut jll = self.compute(features);
        let log_sum = self.log_sum_exp(&jll);

        for prob in &mut jll {
            *prob = (*prob - log_sum).exp();
        }
        jll
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_gaussian_nb() {
        // Model data:
        let priors = vec![0.4939106901217862, 0.5060893098782138];
        let sigmas = vec![
            vec![209672.5458046464, 46273.3704997112, 4.629272464034061, 1916.1953599100862, 1520.0326276901556],
            vec![241136.3079324748, 46298.57549088368, 7.322438179891191, 81691.8801249882, 100694.46098231901]
        ];
        let thetas = vec![
            vec![939.6821917808219, 355.7287671232877, 8.6, 36.26575342465753, 8.378082191780821],
            vec![787.7219251336899, 356.20588235294116, 9.109625668449198, 216.63101604278074, 254.2620320855615]
        ];
        // Estimator:
        let clf = GaussianNB::new(priors, sigmas, thetas);
        let features:Vec<f64> = vec![451 as f64, 277 as f64, 11 as f64, -3 as f64, 88 as f64];
        // Get class prediction:
        let prediction = clf.predict(features.as_slice());
        println!("Predicted class: #{}", prediction);

        // Get class probabilities:
        let probabilities = clf.predict_proba(features.as_slice());
        for (i, &prob) in probabilities.iter().enumerate() {
            println!("Probability of class #{} : {}", i, prob);
        }
    }

    #[test]
    fn test_find_max() {
        let clf = GaussianNB::new(vec![], vec![], vec![]);
        let nums = vec![1.0, 3.0, 2.0, 4.0];
        assert_eq!(clf.find_max(&nums), 3);
    }

    #[test]
    fn test_log_sum_exp() {
        let clf = GaussianNB::new(vec![], vec![], vec![]);
        let nums = vec![1.0, 2.0, 3.0];
        let result = clf.log_sum_exp(&nums);
        assert!((result - 3.4076059644443806).abs() < 1e-10);
    }
} 