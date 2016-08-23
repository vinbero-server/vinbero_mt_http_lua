#include <err.h>
#include <libgonc/gonc_cast.h>
#include <libgonc/gonc_debug.h>
#include <libgonc/gonc_list.h>
#include <libgonc/gonc_nstrncmp.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tucube/tucube_module.h>
#include <tucube/tucube_cldata.h>
#include "tucube_epoll_http_lua.h"


int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list)
{
    struct tucube_module* module = malloc(1 * sizeof(struct tucube_module));
    GONC_LIST_ELEMENT_INIT(module);
    module->tlmodule_key = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlmodule_key, NULL);

    GONC_LIST_APPEND(module_list, module);

    return 0;
}

int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args)
{
    char* script_file = NULL;
    GONC_LIST_FOR_EACH(module_args, struct tucube_module_arg, module_arg)
    {
        if(strncmp("script-file", module_arg->name, sizeof("script-file")) == 0)
	     script_file = module_arg->value;
    }

    if(script_file == NULL)
    {
        warnx("%s: %u: Argument script-file is required", __FILE__, __LINE__);
        pthread_exit(NULL);
    }

    struct tucube_epoll_http_lua_tlmodule* tlmodule = malloc(sizeof(struct tucube_epoll_http_lua_tlmodule));

    tlmodule->L = luaL_newstate();
    luaL_openlibs(tlmodule->L);

    lua_newtable(tlmodule->L); // newtable
    lua_setglobal(tlmodule->L, "clients"); //

    if(luaL_loadfile(tlmodule->L, script_file) != LUA_OK) // file
    {
        warnx("%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    lua_pcall(tlmodule->L, 0, 0, 0); //

    pthread_setspecific(*module->tlmodule_key, tlmodule);

    return 0;
}

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_cldata_list* cldata_list, int* client_socket)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    struct tucube_cldata* cldata = malloc(sizeof(struct tucube_cldata));
    GONC_LIST_ELEMENT_INIT(cldata);
    cldata->pointer = malloc(1 * sizeof(struct tucube_epoll_http_lua_cldata));
    GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket = client_socket;
    GONC_LIST_APPEND(cldata_list, cldata);
    lua_getglobal(tlmodule->L, "clients"); // clients
    lua_pushinteger(tlmodule->L, *client_socket); // clients client_socket
    lua_newtable(tlmodule->L); // clients client_socket client
    lua_settable(tlmodule->L, -3); // clients
    lua_pop(tlmodule->L, 1); //

    return 0;
}

int tucube_epoll_http_module_on_method(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients"); // clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); // clients client_socket
    lua_gettable(tlmodule->L, -2); // clients client
    lua_pushstring(tlmodule->L, "REQUEST_METHOD"); // clients client REQUEST_METHOD
    lua_pushlstring(tlmodule->L, token, token_size); // clients client REQUEST_METHOD token
    lua_settable(tlmodule->L, -3); // clients client 
    lua_pop(tlmodule->L, 2); 

    return 0;
}

int tucube_epoll_http_module_on_uri(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket);
    lua_gettable(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "REQUEST_URI");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 2);

    return 0;
}

int tucube_epoll_http_module_on_version(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket);
    lua_gettable(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "SERVER_PROTOCOL");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 2);

    return 0;
}

int tucube_epoll_http_module_on_header_field(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients"); // clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); //clients client_socket
    lua_gettable(tlmodule->L, -2); // clients client
    lua_pushstring(tlmodule->L, "recent_header_field"); // clients client recent_header_field

    for(ssize_t index = 0; index < token_size; ++index)
    {
        if('a' <= token[index] && token[index] <= 'z')
            token[index] -= ('a' - 'A');
        else if(token[index] == '-')
            token[index] = '_';
    }

    if(gonc_nstrncmp("CONTENT_TYPE", sizeof("CONTENT_TYPE") - 1, token, token_size) == 0 ||
         gonc_nstrncmp("CONTENT_LENGTH", sizeof("CONTENT_LENGTH") - 1, token, token_size) == 0)
    {
        lua_pushlstring(tlmodule->L, token, token_size); // clients client recent_header_field token
    }
    else if(gonc_nstrncmp("X_SCRIPT_NAME", sizeof("X_SCRIPT_NAME") - 1, token, token_size) == 0)
    {
        lua_pushstring(tlmodule->L, "SCRIPT_NAME"); // clients client recent_header_field token
    }
    else
    {
        char* header_field;
        header_field = malloc(sizeof("HTTP_") - 1 + token_size);
        memcpy(header_field, "HTTP_", sizeof("HTTP_") - 1);
        memcpy(header_field + sizeof("HTTP_") - 1, token, token_size);
        lua_pushlstring(tlmodule->L, header_field, sizeof("HTTP_") - 1 + token_size); // clients client recent_header_field token
        free(header_field);
    }
    lua_settable(tlmodule->L, -3); //clients client
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_on_header_value(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients"); // clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); // clients client_socket
    lua_gettable(tlmodule->L, -2); // clients client
    lua_pushstring(tlmodule->L, "recent_header_field"); // clients client recent_header_field
    lua_gettable(tlmodule->L, -2); // clients client header_field
    lua_pushlstring(tlmodule->L, token, token_size); // clients client header_field token
    lua_settable(tlmodule->L, -3); // clients client
    lua_pushstring(tlmodule->L, "recent_header_field"); // clients client recent_header_field
    lua_pushnil(tlmodule->L); // clients client recent_header_field nil
    lua_settable(tlmodule->L, -3); // clients client
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_on_headers_finish(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "on_headers_finish"); // on_headers_finish
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1);
        return 0;
    }
    lua_pcall(tlmodule->L, 0, 0, 0);
    return 0;
}

int tucube_epoll_http_module_get_content_length(struct tucube_module* module, struct tucube_cldata* cldata, ssize_t* content_length)
{
    GONC_DEBUG("get_content_length()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    lua_getglobal(tlmodule->L, "get_content_length"); // get_content_length
    if(lua_isnil(tlmodule->L, -1))
    {
        *content_length = 0;
        lua_pop(tlmodule->L, 1); //
        return 0;
    }

    lua_getglobal(tlmodule->L, "clients"); // get_content_length clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); // get_content_length clients client_socket
    lua_gettable(tlmodule->L, -2); // get_content_length clients client
    lua_remove(tlmodule->L, -2); // get_content_length client
    lua_pcall(tlmodule->L, 1, 1, 0); // content_length
    if(lua_isnil(tlmodule->L, -1))
    {
        *content_length = 0;
        lua_pop(tlmodule->L, 1); //
        return 0;
    }
    *content_length = lua_tointeger(tlmodule->L, -1); // content_length
    lua_pop(tlmodule->L, 1); //
    return 1;
}

int tucube_epoll_http_module_on_body_chunk(struct tucube_module* module, struct tucube_cldata* cldata, char* body_chunk, ssize_t body_chunk_size)
{
    GONC_DEBUG("on_body_chunk()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    lua_getglobal(tlmodule->L, "on_body_chunk"); // on_body_chunk
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1);
        return 0;
    }

    lua_getglobal(tlmodule->L, "clients"); // on_body_chunk clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); // on_body_chunk clients client_socket
    lua_gettable(tlmodule->L, -2); // on_body_chunk clients client
    lua_remove(tlmodule->L, -2); // on_body_chunk client
    lua_pushlstring(tlmodule->L, body_chunk, body_chunk_size); // on_body_chunk client body_chunk
    lua_pcall(tlmodule->L, 2, 0, 0); //
        lua_pop(tlmodule->L, 1);

    return 0;
}

int tucube_epoll_http_module_on_body_finish(struct tucube_module* module, struct tucube_cldata* cldata)
{
    GONC_DEBUG("on_body_finish");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    lua_getglobal(tlmodule->L, "on_body_finish"); // on_body_finish
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1);
        return 0;
    }

    lua_getglobal(tlmodule->L, "clients"); // on_body_finish clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); // on_body_finish clients client_socket
    lua_gettable(tlmodule->L, -2); // on_body_finish clients client
    lua_remove(tlmodule->L, -2); // on_body_finish client
    lua_pcall(tlmodule->L, 1, 0, 0); //

    return 0;
}

int tucube_epoll_http_module_on_request_finish(struct tucube_module* module, struct tucube_cldata* cldata)
{
    GONC_DEBUG("on_request_finish()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients"); // clients
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket); // clients client_socket
    lua_gettable(tlmodule->L, -2); // clients client


    const char* request_uri;
    const char* script_name;
    size_t script_name_size;
    const char* path_info;
    const char* query_string;

    lua_pushstring(tlmodule->L, "REQUEST_URI"); // clients client REQUEST_URI
    lua_gettable(tlmodule->L, -2); //clients client request_uri
    request_uri = lua_tostring(tlmodule->L, -1); // clients client request_uri
    lua_pop(tlmodule->L, 1); // clients client

    lua_pushstring(tlmodule->L, "SCRIPT_NAME"); // clients client SCRIPT_NAME
    lua_gettable(tlmodule->L, -2); // clients client script_name

    if(lua_isnil(tlmodule->L, -1)) // clients client script_name
    {
        lua_pop(tlmodule->L, 1); // clients client
	lua_pushstring(tlmodule->L, "SCRIPT_NAME"); // clients client SCRIPT_NAME
        script_name = "";
        script_name_size = sizeof("") - 1;
        lua_pushstring(tlmodule->L, ""); // clients client SCRIPT_NAME script_name
        lua_settable(tlmodule->L, -3); // cients client
    }
    else
    {
        script_name = lua_tolstring(tlmodule->L, -1, &script_name_size); // clients client script_name
        if(strncmp(script_name + script_name_size - 1, "/", sizeof("/") - 1) == 0)
            return -1;
        lua_pop(tlmodule->L, 1); // clients client
    }

    if((path_info = strstr(request_uri, script_name)) != request_uri) // if request uri doesn't begin with script name
        return -1;

    path_info += script_name_size;

    if((query_string = strstr(path_info, "?")) != NULL) // check if there is query string
    {
        ++query_string; // query string begins after the question mark
        if(strstr(query_string, "?") != NULL) // check if there is unnecessary question mark after query string
            return -1;

        lua_pushstring(tlmodule->L, "PATH_INFO"); // clients client PATH_INFO

        if(query_string - path_info - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlmodule->L, path_info, query_string - path_info - 1); // clients client PATH_INFO path_info
        else
            lua_pushstring(tlmodule->L, "/"); // clients client PATH_INFO path_info
        lua_settable(tlmodule->L, -3); // clients client
        lua_pushstring(tlmodule->L, "QUERY_STRING"); // clients client QUERY_STRING
        lua_pushstring(tlmodule->L, query_string); // clients client QUERY_STRING query_string
        lua_settable(tlmodule->L, -3); // clients client
    }
    else
    {
        lua_pushstring(tlmodule->L, "PATH_INFO"); // clients client PATH_INFO path_info
        if(strlen(path_info) != 0) // check if path info is not empty string
            lua_pushstring(tlmodule->L, path_info); // clients client PATH_INFO path_info
        else
            lua_pushstring(tlmodule->L, "/"); // clients client PATH_INFO path_info
        lua_settable(tlmodule->L, -3); // clients client
    }

    lua_getglobal(tlmodule->L, "on_request_finish"); // clients client on_request_finish
    if(lua_isnil(tlmodule->L, -1))
    {
        warnx("%s: %u: on_request_finish() is not found in the script", __FILE__, __LINE__);
        return -1;
    }
    lua_pushvalue(tlmodule->L, -2); // clients client on_request_finish client
    lua_pcall(tlmodule->L, 1, 3, 0); // clients client status_code headers body
    lua_remove(tlmodule->L, -4); // client status_code headers body
    lua_remove(tlmodule->L, -4); // status_code headers body
    return 0;
}

int tucube_epoll_http_module_get_status_code(struct tucube_module* module, struct tucube_cldata* cldata, int* status_code)
{
    GONC_DEBUG("get_status_code()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *status_code = lua_tointeger(tlmodule->L, -3); // status_code headers body

    return 0;
}

int tucube_epoll_http_module_prepare_get_header(struct tucube_module* module, struct tucube_cldata* cldata)
{
    GONC_DEBUG("prepare_get_header()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pushnil(tlmodule->L); // status_code headers body nil
    lua_next(tlmodule->L, -3); // status_code headers body new_header_field new_header_value
    return 0;
}

int tucube_epoll_http_module_get_header(struct tucube_module* module, struct tucube_cldata* cldata, const char** header_field, size_t* header_field_size, const char** header_value, size_t* header_value_size)
{
    GONC_DEBUG("get_header()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *header_field = lua_tolstring(tlmodule->L, -2, header_field_size); // status_code headers body header_field header_value
    *header_value = lua_tolstring(tlmodule->L, -1, header_value_size); // status_code headers body header_field header_value
    lua_pop(tlmodule->L, 1); // status_code headers body header_field
    if(lua_next(tlmodule->L, -3) != 0) // status_code headers body header_field ->  status_code headers body (new_header_field new_header_value || empty)
        return 1;

    return 0;
}

int tucube_epoll_http_module_prepare_get_body(struct tucube_module* module, struct tucube_cldata* cldata)
{
    GONC_DEBUG("prepare_get_body()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(lua_isfunction(tlmodule->L, -1))
    {
        GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L = lua_newthread(tlmodule->L); // status_code headers body thread
        return 0;
    }
    else if(lua_isstring(tlmodule->L, -1))
    {
        GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L = NULL;
        return 0;
    }
    return -1;
}

int tucube_epoll_http_module_get_body(struct tucube_module* module, struct tucube_cldata* cldata, const char** body, size_t* body_size)
{
    GONC_DEBUG("get_body()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    if(lua_isfunction(tlmodule->L, -2)) // status_code headers body thread
    {
        if(lua_gettop(GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L) > 1)
            lua_pop(GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L, 1);
        int coroutine_result;
        lua_pushvalue(tlmodule->L, -2); // status_code headers body thread body
        lua_xmove(tlmodule->L, GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L, 1); // status_code headers body thread
        coroutine_result = lua_resume(GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L, tlmodule->L, 0); // status_code headers body thread
        *body = lua_tolstring(GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->L, -1, body_size); // status_code headers body thread
        if(coroutine_result == LUA_YIELD)
            return 1; // status_code headers body thread
        lua_pop(tlmodule->L, 1); // status_code headers body
        if(coroutine_result == 0)
            return 0;
        return -1;
    }
    else if(lua_isstring(tlmodule->L, -1)) // status_code headers body
    {
        *body = lua_tolstring(tlmodule->L, -1, body_size);
        return 0;
    }

    return -1;
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_cldata* cldata)
{
    GONC_DEBUG("cldestroy()");
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pop(tlmodule->L, lua_gettop(tlmodule->L)); //
    close(*GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket);
    *GONC_CAST(cldata->pointer, struct tucube_epoll_http_lua_cldata*)->client_socket = -1;
    free(cldata->pointer);
    free(cldata);

    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(tlmodule != NULL)
    {
        lua_close(tlmodule->L);
        free(tlmodule);
    }

    return 0;
}

int tucube_epoll_http_module_destroy(struct tucube_module* module)
{
    pthread_key_delete(*module->tlmodule_key);
    free(module->tlmodule_key);
    free(module);

    return 0;
}
