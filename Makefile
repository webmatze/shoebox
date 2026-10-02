## Haiku Generic Makefile v2.6 ##

NAME = Shoebox
TYPE = APP
APP_MIME_SIG = application/x-vnd.user-Shoebox

#%{
# @src->@
SRCS = \
	src/App.cpp \
	src/MainWindow.cpp \
	src/SidebarView.cpp \
	src/PhotoGridView.cpp \
	src/MetadataPanel.cpp \
	src/QueryModel.cpp \
	src/IndexSetup.cpp \
	src/ThumbnailCache.cpp \
	src/Importer.cpp \
	src/TagModel.cpp

RDEFS =
RSRCS =
# @<-src@
#%}

LIBS = be tracker translation $(STDCPPLIBS)

LIBPATHS =
SYSTEM_INCLUDE_PATHS =
LOCAL_INCLUDE_PATHS = src

OPTIMIZE :=
LOCALES =
DEFINES =
WARNINGS = ALL
SYMBOLS := TRUE
DEBUGGER := TRUE
COMPILER_FLAGS =
LINKER_FLAGS =
APP_VERSION :=
INSTALL_DIR =
TARGET_DIR = .

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
