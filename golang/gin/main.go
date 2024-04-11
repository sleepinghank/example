package main

import (
	"github.com/gin-gonic/gin"
	"net/http"
)

func main() {
	// 创建一个gin Engine，本质上是一个http Handler
	mux := gin.Default()

	mux.Use()

	//	注册一个path为 /ping的处理函数
	mux.POST("/ping", func(context *gin.Context) {
		context.JSON(http.StatusOK, "pone")
	})
	if err := mux.Run(":8080"); err != nil {
		panic(err)
	}

}
