# definitions for cross compiling

PREFIX 	=
CC 	= $(PREFIX)gcc
CXX 	= $(PREFIX)g++
AS 	= $(PREFIX)as
LD 	= $(PREFIX)ld
AR 	= $(PREFIX)ar
RANLIB	= $(PREFIX)ranlib
OBJCOPY = $(PREFIX)objcopy
STRIP 	= $(PREFIX)strip

# compiler flags
CFLAGS	 = -O2
CXXFLAGS = -O2
