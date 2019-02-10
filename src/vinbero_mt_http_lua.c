#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <gaio.h>
#include <libgenc/genc_Cast.h>
#include <libgenc/genc_List.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vinbero_com/vinbero_com_Log.h>
#include <vinbero_com/vinbero_com_Error.h>
#include <vinbero_com/vinbero_com_Status.h>
#include <vinbero_com/vinbero_com_Config.h>
#include <vinbero_com/vinbero_com_Module.h>
#include <vinbero_com/vinbero_com_TlModule.h>
#include <vinbero_com/vinbero_com_ClModule.h>
#include <vinbero/vinbero_iface_MODULE.h>
#include <vinbero/vinbero_iface_CLOCAL.h>
#include <vinbero/vinbero_iface_TLOCAL.h>
#include <vinbero/vinbero_iface_HTTP.h>
#include "vinbero_mt_http_lua_Version.h"

struct vinbero_mt_http_lua_TlModule {
   lua_State* L;
};

struct vinbero_mt_http_lua_ClModule {
    int clientId;
    struct vinbero_iface_HTTP_Response* response;
    lua_State* L;
};

VINBERO_IFACE_MODULE_FUNCTIONS;
VINBERO_IFACE_TLOCAL_FUNCTIONS;
VINBERO_IFACE_CLOCAL_FUNCTIONS;
VINBERO_IFACE_HTTP_FUNCTIONS;

int vinbero_iface_MODULE_init(struct vinbero_com_Module* module) {
    VINBERO_COM_LOG_TRACE2();
    vinbero_com_Module_init(module, "vinbero_mt_http_lua", VINBERO_MT_HTTP_LUA_VERSION, false);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_MODULE_rInit(struct vinbero_com_Module* module) {
    VINBERO_COM_LOG_TRACE2();
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeBytes(lua_State* L) {
    // * response bytes 
    const char* bytes;
    size_t bytesSize;
    bytes = lua_tolstring(L, -1, &bytesSize); // * response bytess
    lua_pushstring(L, "cObject"); // * bytes string "cObject"
    lua_gettable(L, -3); // * response bytes cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response bytes cObject
    response->methods->writeBytes(response, bytes, bytesSize);
    lua_pop(L, 1); // * response bytes 
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeIo(lua_State* L) {
    // * response file
    luaL_Stream* file = lua_touserdata(L, -1); // * response file 
    lua_pushstring(L, "cObject"); // * response file "cObject"
    lua_gettable(L, -3); // * response file cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response file cObject
    struct gaio_Io io;
    struct gaio_Methods ioMethods;
    GAIO_METHODS_NOP_INIT(&ioMethods);
    ioMethods.read = gaio_Fd_read;
    ioMethods.fileno = gaio_Fd_fileno;
    io.object.integer = fileno(file->f);
    io.methods = &ioMethods;
    struct stat statBuffer;
    fstat(io.object.integer, &statBuffer);
    response->methods->writeIo(response, &io, statBuffer.st_size);
    fclose(file->f);
    file->closef = NULL;
    lua_pop(L, 1); // * response file 
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeCrLf(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response cObject
    response->methods->writeCrLf(response);
    lua_pop(L, 1); // * response
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeVersion(lua_State* L) {
    // * response major minor
    int major = lua_tointeger(L, -2); // * response major minor
    int minor = lua_tointeger(L, -1); // * response major minor
    lua_pushstring(L, "cObject"); // * response major minor "cObject"
    lua_gettable(L, -4); // * response major minor cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response major minor cObject
    response->methods->writeVersion(response, major, minor);
    lua_pop(L, 1); // * response major minor
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeStatusCode(lua_State* L) {
    // * response statusCode
    int statusCode = lua_tointeger(L, -1); // * response statusCode
    lua_pushstring(L, "cObject"); // * response statusCode "cObject"
    lua_gettable(L, -3); // * response statusCode cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response statusCode cObject
    response->methods->writeStatusCode(response, statusCode);
    lua_pop(L, 1); // * response statusCode
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeHeader(lua_State* L) {
    // * response headerField headerValue
    const char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    const char* headerValue;
    size_t headerValueSize;
    headerValue = lua_tolstring(L, -1, &headerValueSize); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->methods->writeStringHeader(response, headerField, headerFieldSize, headerValue, headerValueSize);
    lua_pop(L, 1); // * response headerField headerValue
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeBody(lua_State* L) {
    // * response body
    const char* body;
    size_t bodySize;
    body = lua_tolstring(L, -1, &bodySize);
    lua_pushstring(L, "cObject"); // * response body "cObject"
    lua_gettable(L, -3); // * response body cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeStringBody(response, body, bodySize);
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeIoBody(lua_State* L) {
    // * response fileBody
    luaL_Stream* fileBody = lua_touserdata(L, -1);
    lua_pushstring(L, "cObject"); // * response fileBody "cObject"
    lua_gettable(L, -3); // * response fileBody cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1); // * response fileBody cObject
    struct gaio_Io io;
    struct gaio_Methods ioMethods;
    GAIO_METHODS_NOP_INIT(&ioMethods);
    ioMethods.read = gaio_Fd_read;
    ioMethods.fileno = gaio_Fd_fileno;
    io.object.integer = fileno(fileBody->f);
    io.methods = &ioMethods;
    struct stat statBuffer;
    fstat(io.object.integer, &statBuffer);
    response->methods->writeIoBody(response, &io, statBuffer.st_size);
    fclose(fileBody->f);
    fileBody->closef = NULL;
    lua_pop(L, 1); // * response fileBody
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeChunkedBodyStart(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeChunkedBodyStart(response);
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeChunkedBody(lua_State* L) {
    // * response chunkedBody
    const char* chunkedBody;
    size_t chunkedBodySize;
    chunkedBody = lua_tolstring(L, -1, &chunkedBodySize);
    lua_pushstring(L, "cObject"); // * response chunkedBody "cObject"
    lua_gettable(L, -3); // * response chunkedBody cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeChunkedBody(response, chunkedBody, chunkedBodySize);
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_writeChunkedBodyEnd(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct vinbero_iface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeChunkedBodyEnd(response);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_TLOCAL_init(struct vinbero_com_TlModule* tlModule) {
    VINBERO_COM_LOG_TRACE2();
    const char* scriptFile;
    int ret;
    if(vinbero_com_Config_getRequiredConstring(tlModule->module->config, tlModule->module, "vinbero_mt_http_lua.scriptFile", &scriptFile) == false)
        return VINBERO_COM_ERROR_INVALID_CONFIG;
    tlModule->localTlModule.pointer = malloc(sizeof(struct vinbero_mt_http_lua_TlModule));

    struct vinbero_mt_http_lua_TlModule* localTlModule = tlModule->localTlModule.pointer;
    localTlModule->L = luaL_newstate();
    luaL_openlibs(localTlModule->L);
    lua_newtable(localTlModule->L); // vinbero
    lua_pushstring(localTlModule->L, "arg"); // vinbero "arg"
    const char* scriptArg;
    vinbero_com_Config_getConstring(tlModule->module->config, tlModule->module, "vinbero_mt_http_lua.scriptArg", &scriptArg, NULL);
    if(scriptArg != NULL)
        lua_pushstring(localTlModule->L, scriptArg); // vinbero "arg" arg
    else
        lua_pushnil(localTlModule->L); // vinbero "arg" nil
    lua_settable(localTlModule->L, -3); // vinbero 
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_newtable(localTlModule->L); // vinbero "clients" clients
    lua_settable(localTlModule->L, -3); // vinbero
    lua_pushstring(localTlModule->L, "responseMethods"); // vinbero "responseMethods"
    lua_newtable(localTlModule->L); // vinbero "responseMethods" reponseMethods
    lua_pushstring(localTlModule->L, "__index"); // vinbero "responseMethods" responseMethods "__index"
    lua_pushvalue(localTlModule->L, -2); // vinbero "responseMethods" responseMethods "__index" responseMethods
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeBytes"); // vinbero "responseMethods" responseMethods "writeBytes"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeBytes); // vinbero "responseMethods" responseMethods "writeBytes" writeBytes
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeIo"); // vinbero "responseMethods" responseMethods "writeIo"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeIo); // vinbero "responseMethods" responseMethods "writeIo" writeIo
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeCrLf"); // vinbero "responseMethods" responseMethods "writeCrLf"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeCrLf); // vinbero "responseMethods" responseMethods "writeCrLf" writeCrLf"
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods 
    lua_pushstring(localTlModule->L, "writeVersion"); // vinbero "responseMethods" responseMethods "writeVersion"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeVersion); // vinbero "responseMethods" responseMethods "writeVersion" writeVersion
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeStatusCode"); // vinbero "responseMethods" responseMethods "writeStatusCode"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeStatusCode); // vinbero "responseMethods" responseMethods "writeStatusCode" writeStatusCode
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeHeader"); // vinbero "responseMethods" responseMethods "writeHeader" writeHeader
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeHeader); // vinbero "responseMethods" responseMethods "writeHeader" writeHeader
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeBody"); // vinbero "responseMethods" responseMethods "writeBody"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeBody); // vinbero "responseMethods" responseMethods "writeBody" writeBody
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeIoBody"); // vinbero "responseMethods" responseMethods "writeIoBody"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeIoBody); // vinbero "responseMethods" responseMethods "writeIoBody" writeIoBody
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeChunkedBodyStart"); // vinbero "responseMethods" responseMethods "writeChunkedBodyStart"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeChunkedBodyStart); // vinbero "responseMethods" responseMethods "writeChunkedBodyStart" writeChunkedBodyStart
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeChunkedBody"); // vinbero "responseMethods" responseMethods "writeChunkedBody"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeChunkedBody); // vinbero "responseMethods" responseMethods "writeChunkedBody" writeChunkedBody
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(localTlModule->L, "writeChunkedBodyEnd"); // vinbero "responseMethods" responseMethods "writeChunkedBodyEnd"
    lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_writeChunkedBodyEnd); // vinbero "responseMethods" responseMethods "writeChunkedBodyEnd" writeChunkedBodyEnd
    lua_settable(localTlModule->L, -3); // vinbero "responseMethods" responseMethods

    lua_settable(localTlModule->L, -3); // vinbero
    lua_setglobal(localTlModule->L, "vinbero"); //
    if(luaL_loadfile(localTlModule->L, scriptFile) != LUA_OK) { // chunk
        VINBERO_COM_LOG_ERROR("luaL_loadfile() failed");
        return VINBERO_COM_ERROR_UNKNOWN;
    }
    if(lua_pcall(localTlModule->L, 0, 0, 0) != 0) { // errorString 
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // errorString
        lua_pop(localTlModule->L, 1); //
        return VINBERO_COM_ERROR_UNKNOWN;
    }
    lua_getglobal(localTlModule->L, "onInit"); // onInit
    if(lua_isnil(localTlModule->L, -1)) // nil
        lua_pop(localTlModule->L, 1); //
    else { // onInit
        if(lua_pcall(localTlModule->L, 0, 0, 0) != 0) { //
            VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // errorString
            lua_pop(localTlModule->L, 1); //
            return VINBERO_COM_ERROR_UNKNOWN;
        }
    }
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_TLOCAL_rInit(struct vinbero_com_TlModule* tlModule) {
    VINBERO_COM_LOG_TRACE2();
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_CLOCAL_init(struct vinbero_com_ClModule* clModule) {
    VINBERO_COM_LOG_TRACE2();
    clModule->localClModule.pointer = malloc(1 * sizeof(struct vinbero_mt_http_lua_ClModule));
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    localClModule->clientId = gaio_Fd_fileno(GENC_CAST(clModule->arg, struct vinbero_iface_HTTP_Response*)->io);
    localClModule->response = clModule->arg;
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestStart(struct vinbero_com_ClModule* clModule) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_newtable(localTlModule->L); // vinbero clients clientId client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients clientId client "request"
    lua_newtable(localTlModule->L); // vinbero clients clientId client "request" request
    lua_pushstring(localTlModule->L, "headers"); // vinbero clients clientId client "request" request "headers"
    lua_newtable(localTlModule->L); // vinbero clients clientId client "request" request "headers" headers
    lua_settable(localTlModule->L, -3); // vinbero clients clientId client "request" request
    lua_pushstring(localTlModule->L, "body"); // vinbero clients clientId client "request" request "body"
    lua_newtable(localTlModule->L); // vinbero clients clientId client "request" request "body" body
    lua_settable(localTlModule->L, -3); // vinbero clients clientId client "request" request
    lua_pushstring(localTlModule->L, "contentLength"); // vinbero clients clientId client "request" request "contentLength"
    lua_pushinteger(localTlModule->L, 0); // vinbero clients clientId client "request" request "contentLength" 0
    lua_settable(localTlModule->L, -3); // vinbero clients clientId client "request" request
    lua_settable(localTlModule->L, -3); // vinbero clients clientId client
    lua_pushstring(localTlModule->L, "response"); // vinbero clients clientId client "response"
    lua_newtable(localTlModule->L); // vinbero clients clientId client "response" response
    lua_pushstring(localTlModule->L, "cObject"); // vinbero clients clientId client "response" response "cObject"
    lua_pushlightuserdata(localTlModule->L, localClModule->response); // vinbero clients clientId client "response" response "cObject" cObject
    lua_settable(localTlModule->L, -3); // vinbero clients clientId client "response" response
    lua_pushstring(localTlModule->L, "responseMethods"); // vinbero clients clientId client "response" response "responseMethods"
    lua_gettable(localTlModule->L, -7); // vinbero clients clientId client "response" response responseMethods
    lua_setmetatable(localTlModule->L, -2); // vinbero clients clientId client "response" response
    lua_settable(localTlModule->L, -3); // vinbero clients clientId client
    lua_settable(localTlModule->L, -3); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_remove(localTlModule->L, -2); // vinbero client
    lua_getglobal(localTlModule->L, "onRequestStart"); // vinbero client onRequestStart
    if(lua_isnil(localTlModule->L, -1)) { // vinbero client nil
        lua_pop(localTlModule->L, 3); //
        assert(lua_gettop(localTlModule->L) == 0);
        return VINBERO_COM_STATUS_SUCCESS;
    }
    lua_pushvalue(localTlModule->L, -2); // vinbero client onRequestStart client
    if(lua_pcall(localTlModule->L, 1, 0, 0) != 0) { // vinbero client errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // vinbero client errorString
        lua_pop(localTlModule->L, 3); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pop(localTlModule->L, 2); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestMethod(struct vinbero_com_ClModule* clModule, const char* token, size_t tokenSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "method"); // vinbero clients client request "method"
    lua_pushlstring(localTlModule->L, token, tokenSize); // vinbero clients client request "method" method 
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); // 
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestUri(struct vinbero_com_ClModule* clModule, const char* token, size_t tokenSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "requestUri"); // vinbero clients client request "requestUri"
    lua_pushlstring(localTlModule->L, token, tokenSize); // vinbero clients client request "requestUri" requestUri 
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestVersionMajor(struct vinbero_com_ClModule* clModule, int major) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "versionMajor"); // vinbero clients client request "versionMajor"
    lua_pushinteger(localTlModule->L, major); // vinbero clients client request "versionMajor" versionMajor 
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestVersionMinor(struct vinbero_com_ClModule* clModule, int minor) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "versionMinor"); // vinbero clients client request "versionMinor"
    lua_pushinteger(localTlModule->L, minor); // vinbero clients client request "versionMinor" versionMinor
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestContentLength(struct vinbero_com_ClModule* clModule, size_t contentLength) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "contentLength"); // vinbero clients client request "contentLength"
    lua_pushinteger(localTlModule->L, contentLength); // vinbero clients client request "contentLength" contentLength
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); // 
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestContentType(struct vinbero_com_ClModule* clModule, const char* token, size_t tokenSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "contentType"); // vinbero clients client request "contentType"
    lua_pushlstring(localTlModule->L, token, tokenSize); // vinbero clients client request "contentType" contentType
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestScriptPath(struct vinbero_com_ClModule* clModule, const char* token, size_t tokenSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clientss clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client 
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "scriptPath"); // vinbero clients client request "scriptPath"
    lua_pushlstring(localTlModule->L, token, tokenSize); // vinbero clients client request "scriptPath" scriptPath
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestKeepAlive(struct vinbero_com_ClModule* clModule, bool isKeepAlive) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "isKeepAlive"); // vinbero clients client request "isKeepAlive"
    lua_pushboolean(localTlModule->L, isKeepAlive); // vinbero clients client request "isKeepAlive" isKeepAlive
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); // 
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestHeaderField(struct vinbero_com_ClModule* clModule, const char* token, size_t tokenSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "recentHeaderField"); // vinbero clients client request "recentHeaderField"
    char* headerField = malloc(tokenSize * sizeof(char));
    memcpy(headerField, token, tokenSize);
    for(size_t index = 0; index != tokenSize; ++index)
        headerField[index] = toupper(token[index]);
    lua_pushlstring(localTlModule->L, headerField, tokenSize); // vinbero clients client request "recentHeaderField" recentHeaderField
    free(headerField);
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestHeaderValue(struct vinbero_com_ClModule* clModule, const char* token, size_t tokenSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    lua_pushstring(localTlModule->L, "recentHeaderField"); // vinbero clients client request "recentHeaderField"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request recentHeaderField
    lua_pushvalue(localTlModule->L, -2); // vinbero clients client request recentHeaderField request
    lua_pushstring(localTlModule->L, "headers"); // vinbero clients client request recentHeaderField request "headers"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request recentHeaderField request headers
    lua_pushvalue(localTlModule->L, -3); // vinbero clients client request recentHeaderField request headers requestHeaderField
    lua_pushlstring(localTlModule->L, token, tokenSize); // vinbero clients client request recentHeaderField request headers recentHeaderField token
    lua_settable(localTlModule->L, -3); // vinbero clients client request recentHeaderField request headers
    lua_pop(localTlModule->L, 3); // vinbero clients client request
    lua_pushstring(localTlModule->L, "recentHeaderField"); // vinbero clients client request "recentHeaderField"
    lua_pushnil(localTlModule->L); // vinbero clients client request "recentHeaderField" nil
    lua_settable(localTlModule->L, -3); // vinbero clients client request
    lua_pop(localTlModule->L, 4); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestHeadersFinish(struct vinbero_com_ClModule* clModule) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_getglobal(localTlModule->L, "onRequestHeadersFinish"); // vinbero clients client onRequestHeadersFinish
    if(lua_isnil(localTlModule->L, -1)) { // vinbero clients client nil
        lua_pop(localTlModule->L, 4); //
        assert(lua_gettop(localTlModule->L) == 0);
        return VINBERO_COM_STATUS_SUCCESS;
    }
    lua_pushvalue(localTlModule->L, -2); // vinbero clients client onRequestHeadersFinish client
    if(lua_pcall(localTlModule->L, 1, 0, 0) != 0) { // vinbero clients client errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1));
        lua_pop(localTlModule->L, 4); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pop(localTlModule->L, 1); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestBodyStart(struct vinbero_com_ClModule* clModule) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_getglobal(localTlModule->L, "onRequestBodyStart"); // vinbero clients client onRequestBodyStart
    if(lua_isnil(localTlModule->L, -1)) { // vinbero clients client nil
        lua_pop(localTlModule->L, 4); //
        return VINBERO_COM_STATUS_SUCCESS;
    }
    lua_pushvalue(localTlModule->L, -2); // vinbero clients client onRequestBodyStart client
    if(lua_pcall(localTlModule->L, 1, 0, 0) != 0) { // vinbero clients client errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // vinbero clients client errorString
        lua_pop(localTlModule->L, 4); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pop(localTlModule->L, 3); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_onRequestBody(lua_State* L) {
    // * client bodyChunk
    lua_pushstring(L, "request"); // * client bodyChunk "request"
    lua_gettable(L, -3); // * client bodyChunk request
    lua_getglobal(L, "table"); // * client bodyChunk request table
    lua_pushstring(L, "insert"); // * client bodyChunk request table "insert"
    lua_gettable(L, -2); // * client bodyChunk request table insert
    lua_pushvalue(L, -3); // * client bodyChunk request table insert request
    lua_pushstring(L, "body"); // * client bodyChunk request table insert request "body"
    lua_gettable(L, -2); // * client bodyChunk request table insert request body
    lua_remove(L, -2); // * client bodyChunk request table insert body
    lua_pushvalue(L, -5); // * client bodyChunk request table insert body bodyChunk
    if(lua_pcall(L, 2, 0, 0) != 0) { // * client bodyChunk request table errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(L, -1));
        lua_pop(L, 3); // * client bodyChunk
        luaL_error(L, "vinbero_mt_http_lua_onRequestBody() failed"); //
    }
    lua_pop(L, 2); // * client bodyChunk
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestBody(struct vinbero_com_ClModule* clModule, const char* bodyChunk, size_t bodyChunkSize) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_getglobal(localTlModule->L, "onRequestBody"); // vinbero clients onRequestBody
    if(lua_isnil(localTlModule->L, -1)) { // vinbero clients nil
        lua_pop(localTlModule->L, 1); //  vinbero clients
        lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_onRequestBody); // vinbero clients onRequestBody
    }
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients onRequestBody clientId
    lua_gettable(localTlModule->L, -3); // vinbero clients onRequestBody client
    lua_pushlstring(localTlModule->L, bodyChunk, bodyChunkSize); // vinbero clients onRequestBody client bodyChunk
    if(lua_pcall(localTlModule->L, 2, 0, 0) != 0) { // vinbero clients errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // vinbero clients errorString
        lua_pop(localTlModule->L, 3); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pop(localTlModule->L, 2); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}
static int vinbero_mt_http_lua_onRequestBodyFinish(lua_State* L) {
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "body"); // * client request "body"
    lua_gettable(L, -2); // * client request body
    lua_getglobal(L, "table"); // * client request body table
    lua_pushstring(L, "concat"); // * client request body table "concat"
    lua_gettable(L, -2); // * client request body table concat
    lua_pushvalue(L, -3); // * client request body table concat body
    if(lua_pcall(L, 1, 1, 0) != 0) { // * client request body table errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(L, -1)); // * client request body table errorString
        lua_pop(L, 4); // * client
        luaL_error(L, "vinbero_mt_http_lua_onRequestBodyFinish() failed");
    }
    lua_pushvalue(L, -4); // * client request body table result request
    lua_pushstring(L, "body"); // * client request body table result request "body"
    lua_pushvalue(L, -3); // * client request body table result request "body" result
    lua_settable(L, -3); // * client request body table result request
    lua_pop(L, 5); // * client
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestBodyFinish(struct vinbero_com_ClModule* clModule) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_getglobal(localTlModule->L, "onRequestBodyFinish"); // vinbero clients onRequestBodyFinish
    if(lua_isnil(localTlModule->L, -1)) { // vinbero clients nil
        lua_pop(localTlModule->L, 1); // vinbero clients
        lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_onRequestBodyFinish); // vinbero clients onRequestBodyFinish
    } 
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients onRequestBodyFinish clientId
    lua_gettable(localTlModule->L, -3); // vinbero clients onRequestBodyFinish client
    if(lua_pcall(localTlModule->L, 1, 0, 0) != 0) { // vinbero clients errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // vinbero clients errorString
        lua_pop(localTlModule->L, 3); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pop(localTlModule->L, 2); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onGetRequestIntHeader(struct vinbero_com_ClModule* clModule, const char* headerField, int* headerValue) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // * vinbero
    lua_pushstring(localTlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(localTlModule->L, -2); // * vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // * vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // * vinbero clients client 
    lua_pushstring(localTlModule->L, "request"); // * vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request
    lua_pushstring(localTlModule->L, "headers"); // * vinbero clients client request "headers"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers
    lua_getglobal(localTlModule->L, "string"); // * vinbero clients client request headers string
    lua_pushstring(localTlModule->L, "upper"); // * vinbero clients client request headers string "upper"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers string upper
    lua_pushstring(localTlModule->L, headerField); // * vinbero clients client request headers string upper headerField
    if(lua_pcall(localTlModule->L, 1, 1, 0) != 0) { // * vinbero clients client request headers string errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // * vinbero clients client request headers string errorString
        lua_pop(localTlModule->L, 7); // *
        return -1;
    }
    lua_remove(localTlModule->L, -2); // * vinbero clients client request headers upperedHeaderField
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers headerValue
    if(lua_isnil(localTlModule->L, -1)) { // * vinbero clients client request headers headerValue
        VINBERO_COM_LOG_ERROR("Request header %s not found", headerField);
        lua_pop(localTlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tointeger(localTlModule->L, -1);
    lua_pop(localTlModule->L, 6); // *
    return VINBERO_COM_STATUS_SUCCESS;
}
int vinbero_iface_HTTP_onGetRequestDoubleHeader(struct vinbero_com_ClModule* clModule, const char* headerField, double* headerValue) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // * vinbero
    lua_pushstring(localTlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(localTlModule->L, -2); // * vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // * vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // * vinbero clients client 
    lua_pushstring(localTlModule->L, "request"); // * vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request
    lua_pushstring(localTlModule->L, "headers"); // * vinbero clients client request "headers"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers
    lua_getglobal(localTlModule->L, "string"); // * vinbero clients client request headers string
    lua_pushstring(localTlModule->L, "upper"); // * vinbero clients client request headers string "upper"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers string upper
    lua_pushstring(localTlModule->L, headerField); // * vinbero clients client request headers string upper headerField
    if(lua_pcall(localTlModule->L, 1, 1, 0) != 0) { // * vinbero clients client request headers string errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // * vinbero clients client request headers string errorString
        lua_pop(localTlModule->L, 7); // *
        return -1;
    }
    lua_remove(localTlModule->L, -2); // * vinbero clients client request headers upperedHeaderField
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers headerValue
    if(lua_isnil(localTlModule->L, -1)) { // * vinbero clients client request headers headerValue
        VINBERO_COM_LOG_ERROR("Request header %s not found", headerField);
        lua_pop(localTlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tonumber(localTlModule->L, -1);
    lua_pop(localTlModule->L, 6); // *
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onGetRequestStringHeader(struct vinbero_com_ClModule* clModule, const char* headerField, const char** headerValue) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // * vinbero
    lua_pushstring(localTlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(localTlModule->L, -2); // * vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // * vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // * vinbero clients client 
    lua_pushstring(localTlModule->L, "request"); // * vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request
    lua_pushstring(localTlModule->L, "headers"); // * vinbero clients client request "headers"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers
    lua_getglobal(localTlModule->L, "string"); // * vinbero clients client request headers string
    lua_pushstring(localTlModule->L, "upper"); // * vinbero clients client request headers string "upper"
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers string upper
    lua_pushstring(localTlModule->L, headerField); // * vinbero clients client request headers string upper headerField
    if(lua_pcall(localTlModule->L, 1, 1, 0) != 0) { // * vinbero clients client request headers string errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); // * vinbero clients client request headers string errorString
        lua_pop(localTlModule->L, 7); // *
        return -1;
    }
    lua_remove(localTlModule->L, -2); // * vinbero clients client request headers upperedHeaderField
    lua_gettable(localTlModule->L, -2); // * vinbero clients client request headers headerValue
    if(lua_isnil(localTlModule->L, -1)) { // * vinbero clients client request headers headerValue
        VINBERO_COM_LOG_WARN("Request header %s not found", headerField);
        lua_pop(localTlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tostring(localTlModule->L, -1);
    lua_pop(localTlModule->L, 6); // *
    return VINBERO_COM_STATUS_SUCCESS;
}

static int vinbero_mt_http_lua_onGetRequestContentLength(lua_State* L) {
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "contentLength"); // * client request "contentLength"
    lua_gettable(L, -2); // * client request contentLength
    lua_remove(L, -2); // * client contentLength
    return 1;
}

int vinbero_iface_HTTP_onGetRequestContentLength(struct vinbero_com_ClModule* clModule, size_t* contentLength) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // * vinbero
    lua_pushstring(localTlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(localTlModule->L, -2); // * vinbero clients
    lua_getglobal(localTlModule->L, "onGetRequestContentLength"); // * vinbero clients onGetRequestContentLength
    if(lua_isnil(localTlModule->L, -1)) { // * vinbero clients nil 
        lua_pop(localTlModule->L, 3); //
        lua_pushcfunction(localTlModule->L, vinbero_mt_http_lua_onGetRequestContentLength); // * vinbero clients onGetRequestContentLength
    }
    lua_pushinteger(localTlModule->L, localClModule->clientId); // * vinbero clients onGetRequestContentLength clientId
    lua_gettable(localTlModule->L, -3); // * vinbero clients onGetRequestContentLength client
    if(lua_pcall(localTlModule->L, 1, 1, 0) != 0) { // * vinbero clients errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1));
        lua_pop(localTlModule->L, 3); //
        return -1;
    }
    if(lua_isnil(localTlModule->L, -1)) { // * vinbero clients nil 
        lua_pop(localTlModule->L, 3); // *
        return -1;
    }
    *contentLength = lua_tointeger(localTlModule->L, -1); // * vinbero clients requestContentLength
    lua_pop(localTlModule->L, 3); // *
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_HTTP_onRequestFinish(struct vinbero_com_ClModule* clModule) {
    int ret;
    if((ret = vinbero_iface_HTTP_onRequestBodyFinish(clModule)) < VINBERO_COM_STATUS_SUCCESS)
        return ret;
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    struct vinbero_mt_http_lua_ClModule* localClModule = clModule->localClModule.pointer;
    lua_getglobal(localTlModule->L, "vinbero"); // vinbero
    lua_pushstring(localTlModule->L, "clients"); // vinbero "clients"
    lua_gettable(localTlModule->L, -2); // vinbero clients
    lua_pushinteger(localTlModule->L, localClModule->clientId); // vinbero clients clientId
    lua_gettable(localTlModule->L, -2); // vinbero clients client
    lua_pushstring(localTlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request
    const char* requestUri;
    const char* scriptPath;
    size_t scriptPathSize;
    const char* pathInfo;
    const char* queryString;
    lua_pushstring(localTlModule->L, "requestUri"); // vinbero clients client request "requestUri"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request requestUri
    if(lua_isnil(localTlModule->L, -1)) // I don't know why this can be happening
        requestUri = "/";
    else
        requestUri = lua_tostring(localTlModule->L, -1);
    lua_pop(localTlModule->L, 1); // vinbero clients client request
    lua_pushstring(localTlModule->L, "scriptPath"); // vinbero clients client request "scriptPath"
    lua_gettable(localTlModule->L, -2); // vinbero clients client request scriptPath
    if(lua_isnil(localTlModule->L, -1)) { // vinbero clients client request nil
        lua_pop(localTlModule->L, 1); // vinbero clients client request
        lua_pushstring(localTlModule->L, "scriptPath"); // vinbero clients client request "scriptPath"
        scriptPath = "";
        scriptPathSize = sizeof("") - 1;
        lua_pushstring(localTlModule->L, ""); // vinbero clients client request "scriptPath" "" 
        lua_settable(localTlModule->L, -3); // vinbero clients client request
    } else { // vinbero clients client request scriptPath
        scriptPath = lua_tolstring(localTlModule->L, -1, &scriptPathSize);
        if(strncmp(scriptPath + scriptPathSize - 1, "/", sizeof("/") - 1) == 0) {
            VINBERO_COM_LOG_ERROR("scriptPath should not  end with /");
            lua_pop(localTlModule->L, 5); //
            assert(lua_gettop(localTlModule->L) == 0);
            return -1;
        }
        lua_pop(localTlModule->L, 1); // vinbero clients client request
    }
    if((pathInfo = strstr(requestUri, scriptPath)) != requestUri) { // if request uri doesn't begin with script name
        VINBERO_COM_LOG_ERROR("Request uri doesn't begin with script name");
        lua_pop(localTlModule->L, 4); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    pathInfo += scriptPathSize;
    if((queryString = strstr(pathInfo, "?")) != NULL) { // check if there is query string
        ++queryString; // query string begins after the question mark
        if(strstr(queryString, "?") != NULL) { // check if there is unnecessary question mark after query string
            VINBERO_COM_LOG_ERROR("Unnecessary question mark after query string");
            lua_pop(localTlModule->L, 4); //
            assert(lua_gettop(localTlModule->L) == 0);
            return -1;
        }
 
        lua_pushstring(localTlModule->L, "pathInfo"); // vinbero clients client request "pathInfo"
        if(queryString - pathInfo - 1 != 0) // check if path info is not empty string
            lua_pushlstring(localTlModule->L, pathInfo, queryString - pathInfo - 1); // vinbero clients client request "pathInfo" pathInfo
        else
            lua_pushstring(localTlModule->L, "/"); // vinbero clients client request "pathInfo" pathInfo 
        lua_settable(localTlModule->L, -3); // vinbero clients client request
        lua_pushstring(localTlModule->L, "queryString"); // vinbero clients client request "queryString"
        lua_pushstring(localTlModule->L, queryString); // vinbero clients client request "queryString" queryString 
        lua_settable(localTlModule->L, -3); // vinbero clients client request
    } else {
        lua_pushstring(localTlModule->L, "queryString"); // vinbero clients client request "queryString"
        lua_pushstring(localTlModule->L, ""); // vinbero clients client request "queryString" ""
        lua_settable(localTlModule->L, -3); // vinbero clients client request
        lua_pushstring(localTlModule->L, "pathInfo"); // vinbero clients client request "pathInfo"
        if(strlen(pathInfo) != 0) // check if path info is not empty string
            lua_pushstring(localTlModule->L, pathInfo); // vinbero clients client request "pathInfo" pathInfo 
        else
            lua_pushstring(localTlModule->L, "/"); // vinbero clients client request "pathInfo" pathInfo 
        lua_settable(localTlModule->L, -3); // vinbero clients client request
    }
    lua_pop(localTlModule->L, 1); // vinbero clients client 
    lua_getglobal(localTlModule->L, "onRequestFinish"); // vinbero clients client onRequestFinish
    if(lua_isnil(localTlModule->L, -1)) { // vinbero clients client nil
        VINBERO_COM_LOG_ERROR("onRequestFinish() is not found in the script");
        lua_pop(localTlModule->L, 4); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pushvalue(localTlModule->L, -2); // vinbero clients client onRequestFinish client
    if(lua_pcall(localTlModule->L, 1, 0, 0) != 0) { // vinbero clients client errorString
        VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1));
        lua_pop(localTlModule->L, 4); //
        assert(lua_gettop(localTlModule->L) == 0);
        return -1;
    }
    lua_pop(localTlModule->L, 3); //
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_CLOCAL_destroy(struct vinbero_com_ClModule* clModule) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = clModule->tlModule->localTlModule.pointer;
    free(clModule->localClModule.pointer);
    assert(lua_gettop(localTlModule->L) == 0);
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_TLOCAL_destroy(struct vinbero_com_TlModule* tlModule) {
    VINBERO_COM_LOG_TRACE2();
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_TLOCAL_rDestroy(struct vinbero_com_TlModule* tlModule) {
    VINBERO_COM_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* localTlModule = tlModule->localTlModule.pointer;
    if(localTlModule != NULL) {
        lua_getglobal(localTlModule->L, "onDestroy"); // onDestroy
        if(lua_isnil(localTlModule->L, -1))
            lua_pop(localTlModule->L, 1); //
        else {
            if(lua_pcall(localTlModule->L, 0, 0, 0) != 0) {
                VINBERO_COM_LOG_ERROR("%s", lua_tostring(localTlModule->L, -1)); //errorString
                lua_pop(localTlModule->L, 1); //
            }
        }
        lua_close(localTlModule->L);
    }
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_MODULE_destroy(struct vinbero_com_Module* module) {
    VINBERO_COM_LOG_TRACE2();
    return VINBERO_COM_STATUS_SUCCESS;
}

int vinbero_iface_MODULE_rDestroy(struct vinbero_com_Module* module) {
    VINBERO_COM_LOG_TRACE2();
    return VINBERO_COM_STATUS_SUCCESS;
}
