bin_PROGRAMS = video_effects

video_effects_SOURCES = \
	main.c \
	video-effects.c \
	cmdline.c \
	effect_1.c \
	effect_2.c \
	effect_3.c \
	region/region.c

include_HEADERS = \
	video-effects.h \
	cmdline.h \
	effect.h \
	region/region.h

video_effects_CFLAGS = $(GLIB_CFLAGS) $(FFMPEG_CFLAGS)
video_effects_CFLAGS += -Wno-deprecated-declarations
video_effects_LDADD = $(GLIB_LIBS) $(FFMPEG_LIBS) -lm -ldl
