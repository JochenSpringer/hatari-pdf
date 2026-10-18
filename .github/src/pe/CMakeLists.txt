# Printer emulation core: renders printer data streams to PDF
# (NEC Pinwriter P6 and further printer models, see drivers/)
set(PE_SOURCES
	src/engine.c src/pdf.c src/font.c src/registry.c src/util.c
	src/drivers/escp.c src/drivers/p6.c
	src/fonts/font_draft9.c src/fonts/font_lq24.c
	src/fonts/font_prestige.c src/fonts/font_sans.c src/fonts/font_sans_ps.c
	src/fonts/font_script.c src/fonts/font_script_ps.c
	src/fonts/font_ocra.c src/fonts/font_ocrb.c)

add_library(Pe ${PE_SOURCES})
target_include_directories(Pe PUBLIC include PRIVATE src src/drivers)

if(ZLIB_FOUND)
	target_include_directories(Pe PRIVATE ${ZLIB_INCLUDE_DIR})
	target_compile_definitions(Pe PRIVATE PE_HAVE_ZLIB=1)
	target_link_libraries(Pe ${ZLIB_LIBRARY})
endif(ZLIB_FOUND)
