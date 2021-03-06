package main

import (
	"fmt"

	"github.com/sebito91/sedgies/golang/avl/avl"
)

func main() {
	a := avl.New()
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

	a = avl.Insert(10, a)
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

	a = avl.Insert(3, a)
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

	a = avl.Insert(11, a)
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

	a = avl.Insert(12, a)
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

	a = avl.Insert(13, a)
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

	a = avl.Insert(14, a)
	fmt.Printf("DEBUG -- avl is: %+v\n", a)
	a.DumpAVL()

}
