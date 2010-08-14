###############################################################################
# @todo Documentation
# $Id$
###############################################################################

PROJ = TestLogger
OBJS = Logger.o TestLogger.o Time.o

SRC_DIR = ../src

LOG_DIR = $(SRC_DIR)/log
UTIL_DIR = $(SRC_DIR)/utility

INCLUDE = -I $(SRC_DIR) -I /cygdrive/f/Utilities/boost_1_33_0

LINKER_OPTIONS = 

CC_OPTIONS = -Wall -O3 $(INCLUDE)

($PROJ).exe : $(OBJS)
	g++ $(OBJS) $(LINKER_OPTIONS) -o $(PROJ).exe

Logger.o : $(LOG_DIR)/Logger.cpp $(UTIL_DIR)/Time.hpp
	g++ $(CC_OPTIONS) -c $(LOG_DIR)/Logger.cpp $(.TARGET)

Time.o : $(UTIL_DIR)/Time.cpp $(UTIL_DIR)/Time.hpp
	g++ $(CC_OPTIONS) -c $(UTIL_DIR)/Time.cpp $(.TARGET)

TestLogger.o : $(LOG_DIR)/Logger.hpp
	g++ $(CC_OPTIONS) -c $(LOG_DIR)/TestLogger.cpp $(.TARGET)

clean :
	rm $(OBJS) myrrh*.log
