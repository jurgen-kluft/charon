package cgamedata

import (
	"github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	"github.com/jurgen-kluft/cfile/package"
)

// GetPackage returns the package object of 'cgamedata'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := cunittest.GetPackage()
	filepkg := cfile.GetPackage()
	basepkg := cbase.GetPackage()

	// The main (cgamedata) package
	mainpkg := denv.NewPackage("cgamedata")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(filepkg)
	mainpkg.AddPackage(basepkg)

	// 'cgamedata' library
	mainlib := denv.SetupDefaultCppLibProject("cgamedata", "github.com\\jurgen-kluft\\cgamedata")
	mainlib.Dependencies = append(mainlib.Dependencies, filepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())

	// 'cgamedata' unittest project
	maintest := denv.SetupDefaultCppTestProject("cgamedata_test", "github.com\\jurgen-kluft\\cgamedata")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, filepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, basepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
