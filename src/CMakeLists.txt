SET(LIB_NAME ${PROJECT_NAME})

INCLUDE_DIRECTORIES(../include
		    ./include
                    ./ui
                    ./ui/elementary
                    ./graphics)

SET(LIB_SRCS
	 ui/efl_assist.c
	 ui/efl_assist_events.c
	 ui/efl_assist_screen_reader.c
	 ui/efl_assist_theme.c
	 ui/efl_assist_theme_color.c
	 ui/efl_assist_collect_color.c
	 ui/efl_assist_theme_font.c
	 ui/elementary/efl_assist_popup.c
	 ui/elementary/efl_assist_editfield.c
	 graphics/efl_assist_image_effect.c)

ADD_LIBRARY(${LIB_NAME} SHARED ${LIB_SRCS})
ADD_DEFINITIONS("-DEXPORT_API=__attribute__((visibility(\"default\")))")
PKG_CHECK_MODULES(LIB_PKGS REQUIRED elementary ecore-x vconf tts dlog cairo capi-media-image-util
native-buffer)

FOREACH(flag ${LIB_PKGS_CFLAGS})
	SET(LIB_CFLAGS "${LIB_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "${LIB_CFLAGS}")
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${VERSION})
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${LIB_PKGS_LDFLAGS} ${LIB_TARGET_PKGS_LDFLAGS})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib)

