#include "Drawing.hpp"
#include "Bridge.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <iterator>
#include <fstream>
#include "r_lua.h"
#include <intrin.h>
#include <Tlhelp32.h>
#include <CommCtrl.h>
#include <Wininet.h>
#include "BitLibrary.h"
#pragma comment(lib, "wininet.lib")
using namespace std;

DWORD ScriptContext;
HANDLE HandleContext;

using Bridge::m_rL; // This is the DWORD.
using Bridge::m_L; // This is a Lua State.

std::string replace_all(std::string subject, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

static int UserDataGC(lua_State* Gay) { // Should help the lua execution.
	void* UD = lua_touserdata(Gay, 1);
	if (Bridge::m_rL) {

		r_lua_rawgeti(Bridge::m_rL, LUA_REGISTRYINDEX, (int)UD);
		if (r_lua_type(Bridge::m_rL, -1) <= R_LUA_TNIL) {
			lua_pushnil(Gay);
			lua_rawseti(Gay, LUA_REGISTRYINDEX, (int)UD);
		}
	}
	return 0;
}

void Execute(std::string Script) { // What we are executing (script)
	Script = "spawn(function() script=Instance.new(\"LocalScript\") " + Script + "\r\nend)"; // Set Script String to "content".
	if (luaL_loadbuffer(m_L, Script.c_str(), Script.size(), "@Axon")) // Change it to whatever "name of exploit".
	{
		/*	
		    In Lua, It would be Something like: warn(tostring)
		    In Lua C, it would be global warn, pushstring tostring
		*/
		r_lua_getglobal(m_rL, "warn"); // Warn the error out.
		r_lua_pushstring(m_rL, lua_tostring(m_L, -1)); // Get error in string.
		r_lua_pcall(m_rL, 1, 0, 0); // We always use this?
		return;
	}
	else
	{
		lua_pcall(m_L, 0, 0, 0);
	}
	UserDataGC(m_L); // Helps the execution.
	GarbageCollector(m_L); // Helps the memory leaks.
	
}

DWORD WINAPI input(PVOID lvpParameter) // We can also make it a VOID.
{
	string WholeScript = "";
	HANDLE hPipe;
	char buffer[999999]; // Buffer Size.
	DWORD dwRead;
	hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\Axon"), // This is the pipe name.
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_WAIT,
		1,
		999999, // This is a buffer size.
		999999, // This is a buffer size.
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	while (hPipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)
		{
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				buffer[dwRead] = '\0';
				try {
					try {
						WholeScript = WholeScript + buffer;
					}
					catch (...) {
					}
				}
				catch (std::exception e) {
					// An error has occurred.
				}
				catch (...) {
					// I hate errors, do you?
				}
			}
			Execute(WholeScript); // Do something to the pipe name.
			WholeScript = "";
		}
		DisconnectNamedPipe(hPipe);
	}
}


int GetDataModel() // Start the getdatamodel process.
{
	int A = 9;
	int B = 3;
	int result = A + B;
	static DWORD DMPad[16]{};
	r_getdatamodel(getdatamodel2(), (DWORD)DMPad); // Start via address (r_lua.h).
	DWORD DM = DMPad[0];
	return DM + result;
}

int getRawMetaTable(lua_State *L) { // This is called a function.
	Bridge::push(L, m_rL, 1); // ???
	if (r_lua_getmetatable(m_rL, -1) == 0) { // Uses the address from r_lua_getmetatable.
		lua_pushnil(L);
		return 0; // Return.
	}
	Bridge::push(m_rL, L, -1); // ???
	return 1; // Return.
}

int customfunction(lua_State* L) // This is called a custom function.
{
	// Type any C++ here and it will run it when it is executed.
	MessageBoxA(0, "Custom Function from Message Box.", "Message Box.", 0);
	// Create a custom msg box, but you can do whatever.
	return 1; // Return.
}

void RegFuncs(lua_State* L)
{
	lua_register(m_L, "getrawmetatable", getRawMetaTable);
	lua_register(m_L, "showmessagebox", customfunction);
	// Re-name showmessagebox to whatever in lua enviroment.
	// To use it, do showmessagebox() in the execution env.
}

void ConsoleBypass(const char* Title) { // Use Const Char for Custom Name.
	DWORD aaaa; // A DWORD.
	VirtualProtect((PVOID)&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &aaaa);
	*(BYTE*)(&FreeConsole) = 0xC3;
	AllocConsole();
	SetConsoleTitleA(Title); // Set the CS Title to "Const char".
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	HWND ConsoleHandle = GetConsoleWindow();
	::SetWindowPos(ConsoleHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	::ShowWindow(ConsoleHandle, SW_NORMAL); // Normal Default Whatever.
}

const char* GetClass(int self) // This is i have no idea.
{
	return (const char*)(*(int(**)(void))(*(int*)self + 16))();
}

int FindFirstClass(int Instance, const char* Name) // This is i have no idea.
{	
	DWORD StartOfChildren = *(DWORD*)(Instance + 44);
	DWORD EndOfChildren = *(DWORD*)(StartOfChildren + 4);
	for (int i = *(int*)StartOfChildren; i != EndOfChildren; i += 8)
	{
		if (memcmp(GetClass(*(int*)i), Name, strlen(Name)) == 0)
		{
			return *(int*)i;
		}
	}
}

DWORD SetThreadIdentity(DWORD rL, int level) // Set the identity to "level".
{
	return *(DWORD*)(*(DWORD*)(rL + Adresses::Identity2) + 24) = level;
	// PS: Identity2 is a Address.
}

void getdatamodelstart() // Start getdatamodel via lua state.
{
	GDM = GetDataModel(); // GetDataModel is a Void.
	ScriptContext = FindFirstClass(GDM, "ScriptContext"); // Nvm FindFirstClass is Useful.

	DWORD oldRL = Adresses::RBX_LuaState(ScriptContext); // Use via lua state.

	m_rL = r_lua_newthread(oldRL); // Use via new thread address.
}

void InitGlobals() // Wraps up the globals (init main).
{
	std::vector<const char*> Globals = { // List all the globals.
    "printidentity","game","Game","workspace","Workspace",
    "Axes","BrickColor","CFrame","Color3","ColorSequence","ColorSequenceKeypoint",
    "NumberRange","NumberSequence","NumberSequenceKeypoint","PhysicalProperties","Ray",
    "Rect","Region3","Region3int16","TweenInfo","UDim","UDim2","Vector2",
    "Vector2int16","Vector3","Vector3int16","Enum","Faces",
    "Instance","math","warn","typeof","type","print",
    "printidentity","ypcall","Wait","wait","delay","Delay","tick", "pcall", "spawn", "Spawn"
	};
	for (int i = 0; i < Globals.size(); i++)
	{
		r_lua_getglobal(m_rL, Globals[i]); // Getfield address.
		Bridge::push(m_rL, m_L, -1); // ??
		lua_setglobal(m_L, Globals[i]); // Idk what address this.
		r_lua_pop(m_rL, 1); // POP.
		std::cout << "Const Wrapped: " << Globals[i] << std::endl;
		// List down what it successfully wrapped.
	}
}

void PushGlobal(DWORD rL, lua_State* L, const char* s) // Slow way of wrapping globals.
{
	r_lua_getglobal(rL, s);
	Bridge::push(rL, L, -1);
	lua_setglobal(L, s);
	r_lua_pop(rL, 1);
	// PushGlobal(m_rL, m_L, "game"); This is really slow.
}

void main() // DWORD WINAPI this is the main init.
{
	ConsoleBypass("Axon Tutorial / Speed Softwares / SpeedSterKawaii"); 
	getdatamodelstart();
	SetThreadIdentity(m_rL, 7);
	m_L = luaL_newstate();
	Bridge::VehHandlerpush();
	luaL_openlibs(m_L);
	InitGlobals();
	RegFuncs(m_L);
	DrawingAPI::InitiateDrawingAPI(m_L); 
	BitLibrary::RegisterBitLibrary(m_L);
	lua_newtable(m_L);
	lua_setglobal(m_L, "_G");
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)input, NULL, NULL, NULL);
	/*
	    We can make a quick watermark in Lua C.
		This is warn("Axon Loaded") in Lua C.
	*/
	r_lua_getglobal(m_rL, "warn");
	r_lua_pushstring(m_rL, "Axon Loaded.");
	r_lua_pcall(m_rL, 1, 0, 0); // Imagine forgetting this.
}



BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, void* Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(Module);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)main, NULL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		break;
	default: break;
	}

	return TRUE;
}