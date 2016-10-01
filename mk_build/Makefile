SRC_DIR = ../src
OBJ_DIR = ./obj
LIB_LOCAL_DIR := ../../../liblocal

LIB_FLTK_DIR := $(LIB_LOCAL_DIR)/libfltk_gcc
LIB_SQL_DIR := $(LIB_LOCAL_DIR)/libsqlite
LIB_XML_DIR := $(LIB_LOCAL_DIR)/libxml2_gcc

INC_SYS := $(LIB_LOCAL_DIR)

INC_FLTK := $(LIB_FLTK_DIR)/include
INC_SQL := $(LIB_SQL_DIR)/include
INC_XML := $(LIB_XML_DIR)/include/libxml2

OBJ := $(addprefix $(OBJ_DIR)/, main.o icmw.o gautier_rss_model.o)

LIB_FLTK := $(LIB_FLTK_DIR)/lib/libfltk.a
LIB_SQL := $(LIB_SQL_DIR)/lib/libsqlite3.a
LIB_XML := $(LIB_XML_DIR)/lib/libxml2.a

CPP_COMPILE := $(CXX) -c -std=c++14 -isystem $(INC_XML) -isystem $(INC_SYS) -isystem $(INC_FLTK)
CPP_LINK := $(CXX) -std=c++14

LIB_LINK := $(LIB_XML) $(LIB_SQL) $(LIB_FLTK) `$(LIB_FLTK_DIR)/bin/fltk-config --ldstaticflags`

gautier_rss : $(OBJ)
	$(CPP_LINK) -L$(LIB_XML_DIR)/lib -L$(LIB_SQL_DIR)/lib -L$(LIB_FLTK_DIR)/lib -o $@ $(OBJ) $(LIB_LINK)

$(OBJ_DIR)/icmw.o : $(SRC_DIR)/icmw.cxx \
 $(SRC_DIR)/icmw.hxx \
 $(SRC_DIR)/gautier_rss_model.hxx 
	$(CPP_COMPILE)  -o $@ $< 

$(OBJ_DIR)/gautier_rss_model.o : $(SRC_DIR)/gautier_rss_model.cxx \
 $(SRC_DIR)/gautier_rss_model.hxx 
	$(CPP_COMPILE) -I$(INC_XML) -I$(INC_SQL) -o $@ $< 

$(OBJ_DIR)/main.o : $(SRC_DIR)/main.cxx  \
	$(OBJ_DIR) 
	$(CPP_COMPILE) -o $@ $< 

all: $(OBJ_DIR)

$(OBJ): | $(OBJ_DIR)


$(OBJ_DIR): 
	mkdir $(OBJ_DIR) 

#/*Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.*/

