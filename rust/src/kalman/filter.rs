
const R: f64 = 0.01;

// 卡尔曼滤波 迭代计算
pub fn kalman_filter(x: f64, p: f64, z: f64) -> (f64, f64) {
    let x1 = x;
    let p1 = p;
    let k = p1 / (p1 + R);
    println!("k: {}", k);
    let x2 = x1 + k * (z - x1);
    let p2 = (1.0 - k) * p1;
    (x2, p2)
}

// 单元测试
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kalman_filter() {
        let (x, p) = kalman_filter(0.0, 1.0, 20.0);
        println!("x: {}, p: {}", x, p);
        let (x, p) = kalman_filter(x, p, 25.0);
        println!("x: {}, p: {}", x, p);
        assert_eq!(x, 22.388059701492537);
        assert_eq!(p, 0.004975124378109455);
    }
}