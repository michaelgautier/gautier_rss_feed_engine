#Directories
LIB_LOCAL_DIR := ../../../liblocal

LIB_SQLITE3_DIR := $(LIB_LOCAL_DIR)/libsqlite3_gcc
LIB_XERCES_DIR := $(LIB_LOCAL_DIR)/libxerces_gcc

SRC_DIR = ../src
OBJ_DIR := obj
LIB_DIR := lib

#Includes
INC_SYS := $(LIB_LOCAL_DIR)

INC_SQLITE3 := $(LIB_SQLITE3_DIR)/include
INC_XERCES := $(LIB_XERCES_DIR)/include

#Compiler Exceptions
INC_SYS_FLAGS := -isystem $(INC_SYS) -isystem $(INC_SQLITE3) -isystem $(INC_XERCES) 

#Compiled Objects
OBJ := $(OBJ_DIR)/sqlite_20150910.o $(OBJ_DIR)/gautier_diagnostics.o $(OBJ_DIR)/gautier_rss_program.o $(OBJ_DIR)/gautier_rss.o $(OBJ_DIR)/gautier_rss_model.o 

#Libraries
LIB_XERCES := $(LIB_XERCES_DIR)/lib/libxerces-c.a

#Compiler parameters. Minimum standard.
CPP_STD := -std=c++11
#OPTO_PARAM := -O3 #full general optimization. Use this for released versions.
#OPTO_PARAM := -Os #optimize to a small program size. Useful for comparison.
OPTO_PARAM := -g #full debugging. This should be default for testing and revision.
CPP_MIN := $(OPTO_PARAM) $(CPP_STD) -Wpedantic -Wno-system-headers 
CPP_DBG := -fconstexpr-depth=512 -ffor-scope -fstrict-aliasing -fstrict-overflow -fvisibility-inlines-hidden -Wabi -Wall -Wcast-align -Wcast-qual -Wconversion -Wconversion-null -Wctor-dtor-privacy -Wdelete-non-virtual-dtor -Weffc++ -Wextra -Wfloat-equal -Wformat=2 -Wnarrowing -Wno-unused-function -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wreorder -Wshadow -Wsign-promo -Wstrict-overflow=5 

ifeq ($(CXX),g++) 
	CPP_DBG := $(CPP_DBG) -fabi-version=0 -fcheck-new -Wdouble-promotion -Wliteral-suffix -Wnoexcept -Wno-non-template-friend -Wno-pmf-conversions -Wstrict-aliasing=3 -Wstrict-null-sentinel -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wuseless-cast 
endif

#Builders
CPP_COMPILE := $(CPP_MIN) -c $(CPP_DBG) 
CPP_LINK := $(CXX) $(CPP_STD) $(OPTO_PARAM) 

#Linker parameters
LIB_LINK := $(LIB_XERCES) -lpthread -ldl 

#---------------------------------------------------------------------------------------------START

#Build the program
gautier_rss_engine : $(OBJ)
	$(CPP_LINK) -L$(LIB_XERCES_DIR)/lib -o $@ $(OBJ) $(LIB_LINK)

#at this stage in the project, all should compile if any header change
#beyond that, object files should change if its implementation changes.

#Build the object files
$(OBJ_DIR)/gautier_rss.o: $(SRC_DIR)/main.cxx \
 $(SRC_DIR)/gautier_rss_program.hxx 
	$(CXX) $(CPP_COMPILE) -I$(INC_SQLITE3) -I$(INC_XERCES) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_rss_program.o: $(SRC_DIR)/gautier_rss_program.cxx \
 $(SRC_DIR)/gautier_rss_program.hxx \
 $(SRC_DIR)/gautier_rss_model.hxx 
	$(CXX) $(CPP_COMPILE) -I$(INC_SQLITE3) -I$(INC_XERCES) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_rss_model.o: $(SRC_DIR)/gautier_rss_model.cxx \
 $(SRC_DIR)/gautier_rss_model.hxx \
 $(SRC_DIR)/gautier_diagnostics.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/gautier_diagnostics.o : $(SRC_DIR)/gautier_diagnostics.cxx \
 $(SRC_DIR)/gautier_diagnostics.hxx 
	$(CXX) $(CPP_COMPILE) $(INC_SYS_FLAGS) -o $@ $< 

$(OBJ_DIR)/sqlite_20150910.o : infrastructure_build 
	cp --update $(LIB_SQLITE3_DIR)/lib/sqlite_20150910.o $(OBJ_DIR)/

#Setup prerequisite directories and files.
infrastructure_build: $(LIB_XERCES) $(LIB_SQLITE3_DIR)/lib/sqlite_20150910.o 
	mkdir -p $(OBJ_DIR)

.PHONY : clean

clean : 
	rm -r obj/
	rm gautier_*
	rm *~

touch :
	touch ../src/*.hxx
	touch ../src/main.cxx

#---------------------------------------------------------------------------------------------END

#/*Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.*/

