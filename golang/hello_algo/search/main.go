package main

import (
	"fmt"
)

// binarySearch 二分查找（双闭区间）
func binarySearch(nums []int, target int) int{
	if len(nums) == 0 {
		return -1
	}
	// 队首 队尾 指针
	i ,j := 0,len(nums) - 1
	// 当i > j 时未找到跳出
	for i <= j {
		// 指向中间节点
		m := (i + j) / 2 
		if nums[m] == target {
			// 找到时退出
			return m
		} else if nums[m] > target {
			// 当前节点偏大。在前半部分。由于当前节点已经比较过了，所以下次寻找不包含当前节点
			j = m - 1
		} else {
			// 当前节点偏小。在后半部分
			i = m + 1
		}
	}
	return -1
}

func main(){
	fmt.Printf("begin-111-----")
}