package main

func GoodTriplets(nums1 []int, nums2 []int) int64 {
	var count int64 = 0
	numsLen := len(nums1)
	//for i := 0; i < numsLen; i++ {
	//	for j := i + 1; j < numsLen; j++ {
	//		for k := j + 1; k < numsLen; k++ {
	//			idx1, idx2, idx3 := 0, 0, 0
	//			for kk := 0; kk < numsLen; kk++ {
	//				if nums2[kk] == nums1[i] {
	//					idx1 = kk
	//				} else if nums2[kk] == nums1[j] {
	//					idx2 = kk
	//				} else if nums2[kk] == nums1[k] {
	//					idx3 = kk
	//				}
	//			}
	//			if idx1 < idx2 && idx2 < idx3 {
	//				count++
	//			}
	//		}
	//	}
	//}

	//var idxs1 = make([]int, numsLen)
	var idxs2 = make([]int, numsLen)
	for i := 0; i < numsLen; i++ {
		//idxs1[nums1[i]] = i
		idxs2[nums2[i]] = i
	}
	for i := 0; i < numsLen; i++ {
		idx1 := idxs2[nums1[i]]
		for j := i + 1; j < numsLen; j++ {
			idx2 := idxs2[nums1[j]]
			if idx1 >= idx2 {
				continue
			}
			for k := j + 1; k < numsLen; k++ {
				if idx2 < idxs2[nums1[k]] {
					count++
				}

			}
		}
	}
	return count
}

func main() {

}
