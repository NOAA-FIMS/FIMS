# h/t to @jimhester and @yihui for this parse block:
# https://github.com/yihui/knitr/blob/dc5ead7bcfc0ebd2789fe99c527c7d91afb3de4a/Makefile#L1-L4
# Note the portability change as suggested in the manual:
# https://cran.r-project.org/doc/manuals/r-release/R-exts.html#Writing-portable-packages
PKGNAME = `sed -n "s/Package: *\([^ ]*\)/\1/p" DESCRIPTION`
PKGVERS = `sed -n "s/Version: *\([^ ]*\)/\1/p" DESCRIPTION`

R=R

all:
	make build-package
	make install


namespace-update :: NAMESPACE
NAMESPACE:	R/*.R
	R -e "library(roxygen2);roxygenize(\".\",roclets = c(\"namespace\"))"


build-package:
	$(R) CMD build --no-build-vignettes --no-manual .

install:
	$(R) CMD INSTALL --preclean --no-multiarch --with-keep.source .

cran-check:
	$(R) CMD check --as-cran ${PKGNAME}_${PKGVERS}.tar.gz

clean:
	@rm -rf ${PKGNAME}_${PKGVERS}.tar.gz $(PKGNAME).Rcheck
