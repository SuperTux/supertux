if(EXISTS "${OUTPUT}")
  return()
endif()
execute_process(COMMAND "${DLLTOOL_PATH}" -k -d "${DEFINITIONS}" -l "${OUTPUT}")
