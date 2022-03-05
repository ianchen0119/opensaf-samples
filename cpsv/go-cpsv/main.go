package main

/*
#cgo LDFLAGS: -L/usr/local/lib -lSaCkpt
#include "go-cpsv.h"

typedef int GoInt32;

static void start(int rw, char* input){
	cpsv_sync_app_process((void *)(long)rw, input);
}
*/
import "C"
import (
	"strconv"
	"os"
	"fmt"
)

func main() {
	rw, err := strconv.Atoi(os.Args[1])
	if err != nil {
		fmt.Println("Wrong Arguments USAGE: ./go-cpsv <1(Writer)/0(Reader)>")
		fmt.Println(err)
		os.Exit(2)
	}
	testInput := "Hello world!"
	C.start(C.int(rw), C.CString(testInput))
}