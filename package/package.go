package charon

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	denv "github.com/jurgen-kluft/ccode/denv"
	ccore "github.com/jurgen-kluft/ccore/package"
	cfile "github.com/jurgen-kluft/cfile/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

const (
	repo_path = "github.com\\jurgen-kluft\\"
	repo_name = "charon"
)

// GetPackage returns the package object of 'charon'
func GetPackage() *denv.Package {
	name := repo_name

	// Dependencies
	unittestpkg := cunittest.GetPackage()
	filepkg := cfile.GetPackage()
	basepkg := cbase.GetPackage()
	corepkg := ccore.GetPackage()

	// The main (charon) package
	mainpkg := denv.NewPackage(name)
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(filepkg)
	mainpkg.AddPackage(basepkg)
	mainpkg.AddPackage(corepkg)

	// 'charon' library
	mainlib := denv.SetupDefaultCppLibProject(name, repo_path+name)
	mainlib.Dependencies = append(mainlib.Dependencies, filepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, corepkg.GetMainLib())

	// 'charon' unittest project
	maintest := denv.SetupDefaultCppTestProject(name+"_test", repo_path+name)
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
