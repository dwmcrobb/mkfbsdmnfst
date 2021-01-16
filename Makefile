include ./Makefile.vars

CXXFLAGS = -std=c++17
INCS     = -I/usr/include/private/sqlite3 -I.
LIBS     = ${OSLIBS}
OBJFILES = DwmFreeBSDPkgManifestLex.o \
	   DwmFreeBSDPkgManifestParse.o \
	   mkfbsdmnfst.o
OBJDEPS  = $(OBJFILES:%.o=deps/%_deps)
PKGTARGETS = ${STAGING}${PREFIXDIR}/bin/mkfbsdmnfst \
	     ${STAGING}${PREFIXDIR}/man/mkfbsdmnfst.1

mkfbsdmnfst: ${OBJFILES}
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}

package:: pkgprep
	./mkfbsdmnfst -r ./fbsd_manifest -s staging > staging/+MANIFEST
	pkg create -o . -r staging -m staging

pkgprep: ${PKGTARGETS}

${STAGING}${PREFIXDIR}/bin/mkfbsdmnfst: mkfbsdmnfst
	./install-sh -s -c -m 555 $< $@

${STAGING}${PREFIXDIR}/man/mkfbsdmnfst.1: mkfbsdmnfst.1
	./install-sh -c -m 644 $< $@

DwmFreeBSDPkgManifestLex.cc: DwmFreeBSDPkgManifestLex.ll
	flex $<

DwmFreeBSDPkgManifestParse.hh: DwmFreeBSDPkgManifestParse.cc

DwmFreeBSDPkgManifestParse.cc: DwmFreeBSDPkgManifestParse.yy
	bison -d -o$@ $<

#  dependency rule
deps/%_deps: %.cc
	@echo "making dependencies for $<"
	@set -e; \
	${CXX} -MM ${CXXFLAGS} ${INCS} -c $< | \
	 sed 's/\($*\)\.o[ :]*/\1.o $(@D)\/$(@F) : /g' > $@ ; [ -s $@ ] || \
	 rm -f $@

#  only include dependency makefiles if target is not 'clean' or 'distclean'
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include ${OBJDEPS}
endif
endif

%.o: %.cc deps/%_deps
	${CXX} ${CXXFLAGS} ${INCS} -c $< -o $@

clean::
	rm -f ${PKGTARGETS}
	rm -Rf ${STAGING}/*
	rm -f ${OBJFILES} ${OBJDEPS} mkfbsdmnfst
	rm -f DwmFreeBSDPkgManifestLex.cc DwmFreeBSDPkgManifestParse.hh \
	  DwmFreeBSDPkgManifestParse.cc

