package cgamedata

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	ccore "github.com/jurgen-kluft/ccore/package"
	denv "github.com/jurgen-kluft/ccode/denv"
	cfile "github.com/jurgen-kluft/cfile/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

const (
	repo_path = "github.com\\jurgen-kluft\\"
	repo_name = "cgamedata"
)

// GetPackage returns the package object of 'cgamedata'
func GetPackage() *denv.Package {
	name := repo_name

	// Dependencies
	unittestpkg := cunittest.GetPackage()
	filepkg := cfile.GetPackage()
	basepkg := cbase.GetPackage()
	corepkg := ccore.GetPackage()

	// The main (cgamedata) package
	mainpkg := denv.NewPackage(name)
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(filepkg)
	mainpkg.AddPackage(basepkg)
	mainpkg.AddPackage(corepkg)

	// 'cgamedata' library
	mainlib := denv.SetupDefaultCppLibProject(name, repo_path+name)
	mainlib.Dependencies = append(mainlib.Dependencies, filepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, corepkg.GetMainLib())

	// 'cgamedata' unittest project
	maintest := denv.SetupDefaultCppTestProject(name+"_test", repo_path+name)
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, filepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, basepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, corepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
