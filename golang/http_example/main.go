package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
)

func main() {
	println("begin")
	//http.HandleFunc("/ping", func(writer http.ResponseWriter, request *http.Request) {
	//	writer.Write([]byte("pong"))
	//})
	//http.ListenAndServe(":8091", nil)
	context.Background()
	reqBody, _ := json.Marshal(map[string]string{"key1": "val1", "key2": "val2"})

	resp, er := http.Post(":8091", "application/json", bytes.NewReader(reqBody))
	defer resp.Body.Close()
	if er != nil {
		println("%s", er)
	}

	respBody, _ := io.ReadAll(resp.Body)
	fmt.Printf("resp:%s", respBody)

}
