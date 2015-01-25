
add_executable(templight-convert "templight_convert.cpp")
templight_setup_tool_program(templight-convert)
target_link_libraries(templight-convert templight)

