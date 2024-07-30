AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -I$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib

pkginclude_HEADERS = \
  R24hcalQA.h

ROOTDICTS = 

pcmdir = $(libdir)
nobase_dist_pcm_DATA = 

lib_LTLIBRARIES = \
  libr24hcalqa_io.la \
  libr24hcalqa.la

libr24hcalqa_io_la_SOURCES = \
  $(ROOTDICTS) 

libr24hcalqa_la_SOURCES = \
  R24hcalQA.cc

libr24hcalqa_io_la_LIBADD = \
  -lphool -lSubsysReco -lmbd_io -ljetbase

libr24hcalqa_la_LIBADD = \
  libr24hcalqa_io.la \
  -lcalo_io \
  -lSubsysReco -lphg4hit -lffaobjects -lmbd_io \
  -ljetbase -lglobalvertex -lcentrality -lcalotrigger 
#-lg4jets_io
%_Dict.cc: %.h %LinkDef.h
	rootcint -f $@ @CINTDEFS@ $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

#just to get the dependency
%_Dict_rdict.pcm: %_Dict.cc ;

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals_io \
  testexternals

testexternals_io_SOURCES = testexternals.cc
testexternals_io_LDADD   = libr24hcalqa_io.la

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libr24hcalqa.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f $(BUILT_SOURCES)
