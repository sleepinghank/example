package main

import (
	"fmt"
)

// 二叉树节点结构体
type TreeNode struct{
	Val int 
	Left *TreeNode
	Right *TreeNode
}

// 构造方法
func NewTreeNode(v int) *TreeNode{
	return &TreeNode{
		Left: nil,
		Right: nil,
		Val: v,
	}
}

func main(){
	fmt.Printf("begin-111-----")
}