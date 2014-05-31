SET(LIB_SRCS
	 efl_assist.c
	 efl_assist_cutlink.c
	 efl_assist_editfield.c
	 efl_assist_events.c
	 efl_assist_screen_reader.c
	 efl_assist_popup.c
	 efl_assist_fastscroll.c
	 efl_assist_image_effect.c)

ADD_LIBRARY(${LIB_NAME} SHARED ${LIB_SRCS})

ADD_DEFINITIONS("-DEXPORT_API=__attribute__((visibility(\"default\")))")
PKG_CHECK_MODULES(LIB_PKGS REQUIRED elementary vconf tts dlog cairo)

FOREACH(flag ${LIB_PKGS_CFLAGS})
	SET(LIB_CFLAGS "${LIB_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "${LIB_CFLAGS}")
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${VERSION})
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${LIB_PKGS_LDFLAGS} ${LIB_TARGET_PKGS_LDFLAGS})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib)

