# Printer emulation core: renders printer data streams to PDF
# (NEC P6, Epson LQ / LQ-800 / FX-80, Panasonic KX-P2023, Star LC24-10,
#  Schneider/Amstrad NLQ401 - see drivers/)
file(GLOB PE_SOURCES src/*.c src/drivers/*.c src/fonts/*.c)

add_library(Pe ${PE_SOURCES})
target_include_directories(Pe PUBLIC include PRIVATE src src/drivers)

if(ZLIB_FOUND)
	target_include_directories(Pe PRIVATE ${ZLIB_INCLUDE_DIR})
	target_compile_definitions(Pe PRIVATE PE_HAVE_ZLIB=1)
	target_link_libraries(Pe ${ZLIB_LIBRARY})
endif(ZLIB_FOUND)
