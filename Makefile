BIN=Craftmine

HEADER_PATHS=-iquote Build/Classes -I /usr/local/include \
	-isystem /usr/local/include/freetype2

LIBRARIES=enet freeimage freetype GLEW glfw3 icuuc noise SOIL vorbisfile
FRAMEWORKS=CoreFoundation OpenAL OpenGL
ITEMS_TO_COPY=Fonts Images Sounds Shaders BlockData config.conf
APP_DIRECTORIES=. MacOS Resources Frameworks

LIBRARY_FLAGS=$(addprefix -l,$(LIBRARIES))
FRAMEWORK_FLAGS=$(addprefix -framework ,$(FRAMEWORKS))
COMPILER_FLAGS=everything no-c++98-compat no-c++98-compat-pedantic no-float-equal \
	no-global-constructors no-exit-time-destructors no-newline-eof no-missing-prototypes \
	no-padded no-missing-braces

OBJECTS_FOLDER=Build/Data/Objects
APP_CONTENTS=Build/$(BIN).app/Contents

CPP_FLAGS=-arch x86_64 -std=gnu++14 -F Build
DEBUG_FLAGS=-O0 -g
RELEASE_FLAGS=-O3

LINKER_FLAGS=-arch x86_64 -o $(APP_CONTENTS)/MacOS/$(BIN) -L Build -F Build \
	-Xlinker -rpath -Xlinker $(APP_CONTENTS)/Frameworks -Xlinker -no_deduplicate \
	-Xlinker -dependency_info

CPP_FILES=$(wildcard Classes/*.cpp)
OBJ_FILES=$(patsubst Classes/%.cpp,$(OBJECTS_FOLDER)/%.o,$(CPP_FILES))

.SILENT: all clean

all: $(CPP_FILES) $(OBJ_FILES)
	$(info Creating directories...)
	for dir in $(APP_DIRECTORIES); do mkdir -p $(APP_CONTENTS)/$$dir; done
	\
	$(info Linking executable...)
	clang++ $(LINKER_FLAGS) -Xlinker $(OBJECTS_FOLDER)/$(BIN)_dependency_info.dat \
		-filelist Build/Data/$(BIN).LinkFileList -L /usr/local/lib $(LIBRARY_FLAGS) \
		$(FRAMEWORK_FLAGS)
	\
	$(info Copying libraries and files...)
	for LIB in $(LIBRARIES); do rsync --copy-links /usr/local/lib/lib$$LIB.dylib $(APP_CONTENTS)/Frameworks; done
	for ITEM in $(ITEMS_TO_COPY); do rsync -r --exclude .DS_Store --copy-links $$ITEM $(APP_CONTENTS)/Resources; done
	\
	$(info Signing libraries...)
	for LIB in $(LIBRARIES); do \
		install_name_tool -change /usr/local/lib/lib$$LIB.dylib \
			$(APP_CONTENTS)/Frameworks/lib$$LIB.dylib $(APP_CONTENTS)/MacOS/$(BIN); \
		\
		codesign --force --sign - --preserve-metadata=identifier,entitlements \
			$(APP_CONTENTS)/Frameworks/lib$$LIB.dylib; \
	done
	\
	$(info Touching app...)
	touch -c Build/$(BIN).app
	\
	$(info Done!)

$(OBJECTS_FOLDER)/%.o: Classes/%.cpp
	$(info Compiling $<...)
	@clang++ $(addprefix -W,$(COMPILER_FLAGS)) $(CPP_FLAGS) $(HEADER_PATHS) \
		$(DEBUG_FLAGS) -c $< -o $@

clean:
	$(info Removing $(BIN).app...)
	rm -rf Build/$(BIN).app
	\
	$(info Removing build data...)
	rm -f $(OBJECTS_FOLDER)/*
	\
	$(info Done!)