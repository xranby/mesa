include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc

AM_CFLAGS = \
	-Wno-packed-bitfield-compat \
	-I$(top_builddir)/src/glsl/nir \
	$(GALLIUM_DRIVER_CFLAGS) \
	$(LIMA_CFLAGS)

noinst_LTLIBRARIES = liblima.la

liblima_la_SOURCES = \
	$(C_SOURCES)
