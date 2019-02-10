add_library(vinbero_mt_http_lua MODULE vinbero_mt_http_lua.c)
set_target_properties(vinbero_mt_http_lua PROPERTIES PREFIX "")
set_target_properties(vinbero_mt_http_lua PROPERTIES VERSION ${vinbero_mt_http_lua_version}
                                                     SOVERSION ${vinbero_mt_http_lua_soversion}
                                                     PUBLIC_HEADER "")
find_library(fastdl libfastdl.so REQUIRED)
find_library(gaio libgaio.so.0.3 REQUIRED)
find_library(vinbero_com libvinbero_com.so REQUIRED)
find_library(lua NAMES liblua5.3.so liblua-5.3.so liblua-5.3.so.0 liblua.a REQUIRED)
target_link_libraries(vinbero_mt_http_lua ${vinbero_com} ${fastdl} ${gaio} ${lua})
target_include_directories(vinbero_mt_http_lua PRIVATE /usr/include/lua5.3)

install(TARGETS vinbero_mt_http_lua
        LIBRARY DESTINATION lib/vinbero
        PUBLIC_HEADER DESTINATION include/vinbero)
