package main

import (
	"github.com/jurgen-kluft/ccode"
	pkg "github.com/jurgen-kluft/cgamedata/package"
)

func main() {
	ccode.Init()
	ccode.GenerateFiles()
	ccode.Generate(pkg.GetPackage())
}
