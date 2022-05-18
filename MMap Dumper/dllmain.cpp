#include "native.h"
#include <future>
#include <fstream>
#include <string>
std::future<int> main_future;

std::vector<std::int32_t> get_modules()
{
	std::vector<std::int32_t> smodules;
	auto peb = reinterpret_cast<native::_PEB*>(__readfsdword(0x30));
	if (!peb)
		return smodules;


	if (!peb->Ldr->InMemoryOrderModuleList.Flink)
		return smodules;
	auto* list = &peb->Ldr->InMemoryOrderModuleList;

	for (auto i = list->Flink; i != list; i = i->Flink) {
		auto entry = CONTAINING_RECORD(i, native::LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
		if (!entry)
			continue;
		smodules.push_back(entry->DllBase);

	}
	return smodules;
}

void unloaddll(HMODULE hModule)
{
    FreeConsole();
    FreeLibraryAndExitThread(hModule, -1);
}

void dump_module(std::int32_t addr, size_t module_size)
{
    BYTE* module_bytes = new BYTE[module_size];


    memcpy(module_bytes, (void*)addr, module_size);



    std::ofstream of;
    of.open(std::to_string(addr) + ".yeetdmp");

    of.write((char*)module_bytes, module_size);
    of.close();
}
int main(HMODULE hModule)
{
    AllocConsole();
    SetConsoleTitleA("Yeet Dumper");

    freopen("CONOUT$", "r", stdout);
    freopen("CONOUT$", "w", stdout);

    auto module_list = get_modules();

    MEMORY_BASIC_INFORMATION info;
    DWORD m_currentaddr = 0x00;
    DWORD m_endaddr = 0x7FF00000;
    
    while (m_currentaddr < m_endaddr)
    {
        if (VirtualQueryEx(GetCurrentProcess(), (PVOID)m_currentaddr, &info, sizeof(info)) <= 0)
        {
            std::cout << "Failed to get page" << std::endl;
            continue;
        }
        PVOID dwCurrentPage = info.BaseAddress;
        DWORD dwAllocProt = info.AllocationProtect;

        if (info.RegionSize > 0x1000 && (dwAllocProt == PAGE_EXECUTE_READWRITE || dwAllocProt == PAGE_EXECUTE_READ || dwAllocProt == PAGE_EXECUTE_WRITECOPY))
        {
           if(std::find(module_list.begin(), module_list.end(), (std::int32_t)info.AllocationBase) == module_list.end())
           { 
               std::cout << "found mmapped dll" << std::endl;
               dump_module((std::int32_t)info.AllocationBase, info.RegionSize);
           }
        }
        m_currentaddr = (DWORD)dwCurrentPage + info.RegionSize;
    }
    
    std::cout << "done scanning" << std::endl;
    system("pause");
    unloaddll(hModule);
    return 0;

}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        main_future = std::async(std::launch::async, main,hModule);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

