# 仅用于在用 clion 查看代码时更好地跳转
# 尚无法用于构建项目
cmake_minimum_required(VERSION 3.0)
project(cmd)

file(GLOB SOURCE_FILES nt4/private/windows/cmd/*.c)

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES(header_list)

include_directories(${header_list})

add_executable(cmd ${SOURCE_FILES})
