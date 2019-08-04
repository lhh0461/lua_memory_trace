#include <lua.hpp>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>

struct MemHeader {
    char module[128];
    int type;
    int bytes;
};

struct MemTrace {
    lua_State *L;
    std::map<char *, int> *TraceList;
};

void * normal_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud; (void)osize;  /* not used */
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else
        return realloc(ptr, nsize);
}

void * TraceAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    MemTrace *TraceInfo = (MemyTrace *)ud;
    std::map<char *, int> *pTraceList = TraceInfo->TraceList;

    auto it = pTraceList->find((char *)ptr);
    if (ptr && it != pTraceList->end()) {
        MemoryHdr *pHdr = (MemoryHdr *)ptr - 1;
        if (nsize == 0) {
            pTraceList->erase(it);
            free(ptr);
            return NULL;
        }
        else {
            char * newHdr = (char *)realloc(pHdr, nsize);
            pHdr->bytes = nsize;
            if (newHdr != (char *)pHdr) {
                pTraceList->erase(it);
                pTraceList->insert(std::make_pair(newHdr + sizeof(MemoryHdr), 1));
                return newHdr + sizeof(MemoryHdr);
            } else {
                return ptr;
            }
        }
    }

    //分配新数据
    if (osize > 0 && ptr == NULL) {
        lua_Debug ar;
        memset(&ar, 0, sizeof(ar));
        lua_State *L = TraceInfo->L;
        lua_getstack(L, 0, &ar);
        if (ar.i_ci != NULL) {
            int ret = lua_getinfo(L, "nlS", &ar);
            if (strcmp(ar.what, "C") != 0) {
                std::cout << "file:" << ar.short_src << "(" << ar.currentline << "), function:" << (ar.name ? ar.name : "?") << std::endl;
                std::cout << "my_alloc " <<" " << ptr << " " << osize << " " << nsize << std::endl;
                char *ptr = (char *)malloc(sizeof(MemoryHdr) + nsize);
                MemoryHdr *pHdr = (MemoryHdr *)ptr;
                strncpy(pHdr->module, ar.short_src, sizeof(pHdr->module));
                pHdr->type = osize;
                pHdr->bytes = nsize;
                pTraceList->insert(std::make_pair((char *)(pHdr + 1), 1));
                return (char *)ptr + sizeof(MemoryHdr);
            }
        }
    }

    return normal_alloc(ud, ptr, osize, nsize);
}

void DumpTraceInfo(MemTrace * pTraceInfo)
{
    std::map<string, <int, size_t>> mpResult;
    auto it = pTraceInfo->TraceList->begin();
    for (;it != pTraceInfo->TraceList->end();it++) {
        MemoryHdr *pHdr = ((MemoryHdr *)(it->first)) -1;
        std::cout << "module:" << pHdr->module << ",type:" << pHdr->type << ",bytes:" << pHdr->bytes << std::endl;
    }
}

int InitTrace(lua_State *L)
{

}
