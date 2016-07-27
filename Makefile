OS=$(shell uname)
BIN=Craftmine
DIRECTORY=$(shell pwd)

OBJECTS_FOLDER=Build/Data/Objects
APP_CONTENTS=Build/$(BIN).app/Contents

./BlockScripts/Load_Scripts.sh

ifeq ($(OS),Linux)
	CXX=clang++-3.8
	WIN_ROOT=C:/Users/andre/AppData/Local/lxss/rootfs
	CPP_FLAGS=-std=gnu++14 -stdlib=libc++
	HEADER_PATHS=-iquote Build/Classes -I /usr/local/include -isystem /usr/local/include/freetype2

	LIB_PATHS=-L /usr/local/lib -L /usr/local/lib/boost
	LIBRARIES=boost_filesystem boost_system enet freeimage freetype GLEW glfw3 icuuc noise OpenAL32 SOIL vorbisfile

	LINKER_FLAGS=-o Build/$(BIN) -Xlinker -rpath -Xlinker -no_deduplicate -Xlinker -dependency_info

else
	CXX=clang++
	CPP_FLAGS=-arch x86_64 -std=gnu++14 -F Build
	HEADER_PATHS=-iquote Build/Classes -I /usr/local/include -isystem /usr/local/include/freetype2

	LIB_PATHS=-L /usr/local/lib -L /usr/local/lib/boost
	LIBRARIES=boost_filesystem boost_system enet freeimage freetype GLEW glfw3 icuuc noise SOIL vorbisfile

	LINKER_FLAGS=-arch x86_64 -o $(APP_CONTENTS)/MacOS/$(BIN) -L Build -F Build \
		-Xlinker -rpath -Xlinker $(APP_CONTENTS)/Frameworks -Xlinker -no_deduplicate \
		-Xlinker -dependency_info
endif

FRAMEWORKS=CoreFoundation OpenAL OpenGL
ITEMS_TO_COPY=BlockData Fonts Images Shaders Sounds Worlds config.conf
APP_DIRECTORIES=. MacOS Resources Frameworks

LIBRARY_FLAGS=$(addprefix -l,$(LIBRARIES))
FRAMEWORK_FLAGS=$(addprefix -framework ,$(FRAMEWORKS))
COMPILER_FLAGS=everything no-c++98-compat no-c++98-compat-pedantic no-float-equal \
	no-global-constructors no-exit-time-destructors no-newline-eof no-missing-prototypes \
	no-padded no-missing-braces no-undef

DEBUG_FLAGS=-O0 -g
RELEASE_FLAGS=-O3

CPP_FILES=$(wildcard Classes/*.cpp)
OBJ_FILES=$(patsubst Classes/%.cpp,$(OBJECTS_FOLDER)/%.o,$(CPP_FILES))

.SILENT: clean

# Mac compilation
ifeq ($(OS),Darwin)

all: $(CPP_FILES) $(OBJ_FILES)
	$(info Creating object file list...)
	:> Build/Data/$(BIN).LinkFileList
	for OBJ in $(OBJ_FILES); do echo $(DIRECTORY)/$$OBJ >> Build/Data/$(BIN).LinkFileList; done
	\
	$(info Creating directories...)
	for dir in $(APP_DIRECTORIES); do mkdir -p $(APP_CONTENTS)/$$dir; done
	\
	$(info Linking executable...)
	$(CXX) $(LINKER_FLAGS) -Xlinker $(OBJECTS_FOLDER)/$(BIN)_dependency_info.dat -filelist Build/Data/$(BIN).LinkFileList $(LIB_PATHS) $(LIBRARY_FLAGS) $(FRAMEWORK_FLAGS)
	\
	$(info Copying libraries and files...)
	for LIB in $(LIBRARIES); do rsync --copy-links /usr/local/lib/lib$$LIB.dylib $(APP_CONTENTS)/Frameworks; done
	for ITEM in $(ITEMS_TO_COPY); do rsync -r --exclude .DS_Store --copy-links $$ITEM $(APP_CONTENTS)/Resources; done
	\
	$(info Signing libraries...)
	for LIB in $(LIBRARIES); do \
		install_name_tool -change /usr/local/lib/lib$$LIB.dylib $(APP_CONTENTS)/Frameworks/lib$$LIB.dylib $(APP_CONTENTS)/MacOS/$(BIN); \
		codesign --force --sign - --preserve-metadata=identifier,entitlements $(APP_CONTENTS)/Frameworks/lib$$LIB.dylib; \
	done
	\
	$(info Touching app...)
	touch -c Build/$(BIN).app
	\
	$(info Done!)

# Linux (subsystem) compilation
else

all: $(CPP_FILES) $(OBJ_FILES)
	$(info Linking executable...)
	$(CXX) $(LINKER_FLAGS) $(LIB_PATHS) $(LIBRARY_FLAGS) -Xlinker $(OBJECTS_FOLDER)/$(BIN)_dependency_info.dat $(OBJ_FILES) $(LIB_PATHS)
	\
	$(info Done!)

endif

$(OBJECTS_FOLDER)/%.o: Classes/%.cpp
	$(info Compiling $<...)
	@$(CXX) $(addprefix -W,$(COMPILER_FLAGS)) $(CPP_FLAGS) $(HEADER_PATHS) $(DEBUG_FLAGS) -c $< -o $@

clean:
	$(info Removing $(BIN).app...)
	rm -rf Build/$(BIN).app
	\
	$(info Removing build data...)
	rm -f $(OBJECTS_FOLDER)/*
	\
	$(info Done!)
