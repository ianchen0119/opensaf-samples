package main
/*
#cgo LDFLAGS: -L/usr/local/lib -lSaCkpt
#include "go-cpsv.h"

static void start(){
	cpsv_test_sync_app_process((void *)(long)1);
}
*/
import "C"

func main() {

	C.start()
}