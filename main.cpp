int main()
{
    MemoryTrace TraceInfo;
    lua_State *L = luaL_newstate();
    TraceInfo.L = L;
    TraceInfo.TraceList = new std::map<char *, int>();
    lua_setallocf (L, my_alloc, &TraceInfo);
    luaL_openlibs(L);  //加载Lua基本库
    
    const char *p = "hello world";
    lua_pushlstring(L, p, strlen(p));
    lua_pushlstring(L, p, strlen(p));
    lua_newtable(L);

    luaL_dofile(L, "./test.lua");

    lua_newtable(L);
    lua_newtable(L);

    DumpTraceInfo(&TraceInfo);
}
