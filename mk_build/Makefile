#Directories
SRC_DIR = ../src
OBJ_DIR := obj
LIB_DIR := lib
LIB_LOCAL_DIR := ../../../liblocal

#Third-party lib/object file directories.
#GNU GCC is the default compiler for this makefile.
LIB_FLTK_DIR := $(LIB_LOCAL_DIR)/libfltk_gcc
LIB_JSONCPP_DIR := $(LIB_LOCAL_DIR)/libjson
LIB_SQL_DIR := $(LIB_LOCAL_DIR)/libsqlite
LIB_XML_DIR := $(LIB_LOCAL_DIR)/libxml2_gcc

#Third-party lib/object file directories. Variations based on compiler used.
ifeq ($(CXX),clang++) 
LIB_FLTK_DIR := $(LIB_LOCAL_DIR)/libfltk_clang
LIB_JSONCPP_DIR := $(LIB_LOCAL_DIR)/libjson_clang
LIB_SQL_DIR := $(LIB_LOCAL_DIR)/libsqlite3_clang
LIB_XML_DIR := $(LIB_LOCAL_DIR)/libxml2_clang
endif

#Includes
#Will be treated as system include location.
INC_SYS := $(LIB_LOCAL_DIR)

#Includes for third-party libs and object files.
INC_FLTK := $(LIB_FLTK_DIR)/include
INC_JSON := $(LIB_JSONCPP_DIR)
INC_SQL := $(LIB_SQL_DIR)/include
INC_XML := $(LIB_XML_DIR)/include/libxml2

#Compiler Exceptions
#Do not want warnings presented on third-party inclusions.
#Intended to limit warnings and errors to application code only.
INC_SYS_FLAGS := -isystem $(INC_SYS) -isystem $(INC_FLTK) -isystem $(INC_JSON) -isystem $(INC_SQL) -isystem $(INC_XML) 

#All Compiled Objects, third-party and application level
#Michael Gautier 7/3/2016 -- removed the sqlite.o target to use static sqlite lib.
OBJ := $(addprefix $(OBJ_DIR)/, jsoncpp.o gautier_diagnostics.o gautier_rss_model.o gautier_screen_structure_build.o gautier_screen_structure_build_json.o gautier_visual_model.o gautier_visual_application_window.o gautier_rss_program.o gautier_rss.o)

#Third-party lib files.
LIB_FLTK := $(LIB_FLTK_DIR)/lib/libfltk.a
LIB_XML := $(LIB_XML_DIR)/lib/libxml2.a
LIB_SQL := $(LIB_SQL_DIR)/lib/libsqlite3.a

#Compiler parameters. Minimum standard.
#	Michael Gautier 20160703		GCC 6 defaults to C++14		CPP_STD := -std=c++11
#	Michael Gautier 20160703		GCC 6 defaults to C++14		C_STD := -std=c11
ifeq ($(CXX),clang++)
CPP_STD := -std=c++14
C_STD := -std=c11
endif
#OPTO_PARAM := -O3 #full general optimization. Use this for released versions.
#OPTO_PARAM := -Os #optimize to a small program size. Useful for comparison.
OPTO_PARAM := -g #full debugging. This should be default for testing and revision.
CPP_MIN := $(OPTO_PARAM) $(CPP_STD) -Wpedantic -Wno-system-headers 
C_MIN := $(OPTO_PARAM) $(C_STD) -Wpedantic -Wno-system-headers 
CPP_DBG := -fconstexpr-depth=512 -ffor-scope -fstrict-aliasing -fstrict-overflow -fvisibility-inlines-hidden -Wabi -Wall -Wcast-align -Wcast-qual -Wconversion -Wconversion-null -Wctor-dtor-privacy -Wdelete-non-virtual-dtor -Weffc++ -Wextra -Wfloat-equal -Wformat=2 -Wnarrowing -Wno-unused-function -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wreorder -Wshadow -Wsign-promo -Wstrict-overflow=5 

#Vary the compile flags based on the compiler used.
#Clang and GCC do not share all the same flags. 
#This is an extended subset recognized by GCC. 
ifeq ($(CXX),g++) 
CPP_DBG := $(CPP_DBG) -fabi-version=0 -fcheck-new -Wdouble-promotion -Wliteral-suffix -Wnoexcept -Wno-non-template-friend -Wno-pmf-conversions -Wstrict-aliasing=3 -Wstrict-null-sentinel -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wuseless-cast 
endif

#Builders
CPP_COMPILE := $(CPP_MIN) -c $(CPP_DBG) 
CPP_LINK := $(CXX) $(CPP_STD) $(OPTO_PARAM) 
C_COMPILE := $(CC) $(C_MIN) 

#Linker parameters
#No pkg-config for LibXML as it shares some dependencies with FLTK.
#This line is not necessary if compiling using the shared libraries.
#This is for static versions of the libraries.
LIB_LINK := $(LIB_XML) $(LIB_SQL) $(LIB_FLTK) `$(LIB_FLTK_DIR)/bin/fltk-config --ldstaticflags`
# -lz
#---------------------------------------------------------------------------------------------START

#Build the program
gautier_rss : $(OBJ)
	$(CPP_LINK) -L$(LIB_FLTK_DIR)/lib -L$(LIB_SQL_DIR)/lib -o $@ $(OBJ) $(LIB_LINK)

#at this stage in the project, all should compile if any header change
#beyond that, object files should change if its implementation changes.

#Build the object files
$(OBJ_DIR)/gautier_diagnostics.o : $(SRC_DIR)/gautier_diagnostics.cxx \
 $(SRC_DIR)/gautier_diagnostics.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/jsoncpp.o : $(LIB_JSONCPP_DIR)/jsoncpp.cpp \
 $(INC_JSON)/json/json.h 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -w -o $@ $< 

#Michael Gautier 20160703 - changed over to ./configure and make with sqlite3. Libs are statically built into liblocal
#$(OBJ_DIR)/sqlite.o : $(INC_SQL)/sqlite3.c \
# $(INC_SQL)/sqlite3.h 
#	$(C_COMPILE) $(INC_SYS_FLAGS) -c -o $@ $< 

$(OBJ_DIR)/gautier_rss_model.o: $(SRC_DIR)/gautier_rss_model.cxx \
 $(SRC_DIR)/gautier_rss_model.hxx \
 $(SRC_DIR)/gautier_diagnostics.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_screen_structure_build_json.o: $(SRC_DIR)/gautier_screen_structure_build_json.cxx \
 $(SRC_DIR)/gautier_screen_structure_build_json.hxx \
 $(SRC_DIR)/gautier_visual_model.hxx 
	$(CXX) $(CPP_COMPILE) -I$(INC_JSON) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_screen_structure_build.o: $(SRC_DIR)/gautier_screen_structure_build.cxx \
 $(SRC_DIR)/gautier_screen_structure_build.hxx \
 $(SRC_DIR)/gautier_screen_structure_build_json.hxx \
 $(SRC_DIR)/gautier_visual_model.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_visual_model.o: $(SRC_DIR)/gautier_visual_model.cxx \
 $(SRC_DIR)/gautier_rss_program.hxx \
 $(SRC_DIR)/gautier_visual_model.hxx \
 $(SRC_DIR)/gautier_visual_application_window.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_visual_application_window.o: $(SRC_DIR)/gautier_visual_application_window.cxx \
 $(SRC_DIR)/gautier_visual_model.hxx \
 $(SRC_DIR)/gautier_visual_application_window.hxx \
 $(SRC_DIR)/gautier_screen_structure_build.hxx \
 $(SRC_DIR)/gautier_screen_structure_build_json.hxx 
	$(CXX) $(CPP_COMPILE) -I$(INC_FLTK) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_rss_program.o: $(SRC_DIR)/gautier_rss_program.cxx \
 $(SRC_DIR)/gautier_rss_program.hxx \
 $(SRC_DIR)/gautier_visual_model.hxx \
 $(SRC_DIR)/gautier_visual_application_window.hxx \
 $(SRC_DIR)/gautier_screen_structure_build.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_rss.o: $(SRC_DIR)/main.cxx \
 $(SRC_DIR)/gautier_rss_program.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

all: $(OBJ_DIR)

$(OBJ): | $(OBJ_DIR)


$(OBJ_DIR): 
	mkdir $(OBJ_DIR) 

clean : 
	rm -r obj/
	rm gautier_*
	rm *~

touch :
	touch ../src/*.hxx
	touch ../src/main.cxx

#---------------------------------------------------------------------------------------------END

#/*Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.*/

