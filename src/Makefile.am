# what flags you want to pass to the C compiler & linker
# CFLAGS = --pedantic -Wall -std=c99 -O2
# LDFLAGS =

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
BUILT_SOURCES = nexusParser.h
AM_YFLAGS = -d
AM_LFLAGS = -l -i 
AM_CFLAGS = @GLU_CFLAGS@ @GLUT_CFLAGS@
AM_LDFLAGS = @GLU_LIBS@ @GLUT_LIBS@
bin_PROGRAMS = ceiba 
lib_LTLIBRARIES = libstructures.la libnexusParser.la
libstructures_la_SOURCES = my_hash.c my_hash_fixed.c my_hgraph.c my_queue.c my_slist.c my_vector.c my_structures.c
libstructures_la_LDFLAGS = -version-info 0:0:0
libnexusParser_la_SOURCES = nexusParser.y nexusLexer.l yywrap.c mySmallTreeLib.c main.c
libnexusParser_la_LDFLAGS = -version-info 0:0:0
# nexusParser_SOURCES = nexusParser.y nexusLexer.l yywrap.c mySmallTreeLib.c main.c
# nexusParser_LDADD = libstructures.la
ceiba_SOURCES = tr.c tree_openGL.c fonts.c label.c touch.c fdp.c getXYZ.c getSTL.c getPPM.c frustum_cull.c menu.c picking.c branch.c layout.c 
ceiba_LDADD = libstructures.la libnexusParser.la
