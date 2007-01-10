#
#  Copyright (c) 2006, Peter Kümmel, 
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  
#  1. Redistributions of source code must retain the copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products 
#     derived from this software without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  

if (ICONV_INCLUDE_DIR)
  # Already in cache, be silent
  set(ICONV_FIND_QUIETLY TRUE)
endif (ICONV_INCLUDE_DIR)

FIND_PATH(ICONV_INCLUDE_DIR iconv.h
 /usr/include
 /usr/local/include
)

set(POTENTIAL_ICONV_LIBS iconv libiconv libiconv2)
FIND_LIBRARY(ICONV_LIBRARY NAMES ${POTENTIAL_ICONV_LIBS}
PATHS
 /usr/lib
 /usr/local/lib
)

if(WIN32)
	set(ICONV_DLL_NAMES iconv.dll  libiconv.dll libiconv2.dll)
	FIND_FILE(ICONV_DLL   
					NAMES ${ICONV_DLL_NAMES}
					PATHS ENV PATH
					NO_DEFAULT_PATH)
	FIND_FILE(ICONV_DLL_HELP   
					NAMES ${ICONV_DLL_NAMES}
					PATHS ENV PATH
					${ICONV_INCLUDE_DIR}/../bin)
	IF(ICONV_FIND_REQUIRED)
		IF(NOT ICONV_DLL AND NOT ICONV_DLL_HELP)
			MESSAGE(FATAL_ERROR "Could not find iconv.dll, please add correct your PATH environment variable")
		ENDIF(NOT ICONV_DLL AND NOT ICONV_DLL_HELP)
		IF(NOT ICONV_DLL AND ICONV_DLL_HELP)
			GET_FILENAME_COMPONENT(ICONV_DLL_HELP ${ICONV_DLL_HELP} PATH)
			MESSAGE(STATUS)
			MESSAGE(STATUS "Could not find iconv.dll in standard search path, please add ")
			MESSAGE(STATUS "${ICONV_DLL_HELP}")
			MESSAGE(STATUS "to your PATH environment variable.")
			MESSAGE(STATUS)
			MESSAGE(FATAL_ERROR "exit cmake")
		ENDIF(NOT ICONV_DLL AND ICONV_DLL_HELP)
	ENDIF(ICONV_FIND_REQUIRED)
ELSE(WIN32)
	set(ICONV_DLL TRUE)
endif(WIN32)


IF (ICONV_INCLUDE_DIR AND ICONV_LIBRARY AND ICONV_DLL)
   SET(ICONV_FOUND TRUE)
ENDIF (ICONV_INCLUDE_DIR AND ICONV_LIBRARY AND ICONV_DLL)

IF (ICONV_FOUND)
   IF (NOT ICONV_FIND_QUIETLY)
      MESSAGE(STATUS "Found iconv library: ${ICONV_LIBRARY}")
      #MESSAGE(STATUS "Found iconv   dll  : ${ICONV_DLL}")
   ENDIF (NOT ICONV_FIND_QUIETLY)
ELSE (ICONV_FOUND)
   IF (ICONV_FIND_REQUIRED)
      MESSAGE(STATUS "Looked for iconv library named ${POTENTIAL_ICONV_LIBS}.")
      MESSAGE(STATUS "Found no acceptable iconv library. This is fatal.")
      MESSAGE(STATUS "iconv header: ${ICONV_INCLUDE_DIR}")
      MESSAGE(STATUS "iconv lib   : ${ICONV_LIBRARY}")
      MESSAGE(FATAL_ERROR "Could NOT find iconv library")
   ENDIF (ICONV_FIND_REQUIRED)
ENDIF (ICONV_FOUND)

MARK_AS_ADVANCED(ICONV_LIBRARY ICONV_INCLUDE_DIR)

