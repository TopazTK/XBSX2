#include "PrecompiledHeader.h"

#include "LuaPS2.h"

u32 LuaPS2::_checksum;
std::string LuaPS2::_loadPath;
std::string LuaPS2::_savePath;

bool LuaPS2::_isSaving = false;

LuaPS2::LuaPS2(std::string Input01, u32 Input02)
{
	luaState.open_libraries
	(
		lib::base,
		lib::package,
		lib::coroutine,
		lib::string,
		lib::os,
		lib::math,
		lib::table,
		lib::io,
		lib::bit32,
		lib::utf8
	);

	SetFunctions();

	_loadPath = Path::Combine(Path::GetDirectory(Input01), "io_load");

	luaState["package"]["path"] = Path::Combine(Path::GetDirectory(Input01), "io_packages/") + "?.lua";
	luaState["package"]["cpath"] = Path::Combine(Path::GetDirectory(Input01), "io_packages/") + "?.dll";

	luaState["LOAD_PATH"] = _loadPath;
	luaState["SCRIPT_PATH"] = Path::GetDirectory(Input01);

	luaState["BASE_ADDR"] = 0x20000000;

	luaState["ENGINE_VERSION"] = 4.3;
	luaState["ENGINE_TYPE"] = "ENGINE";
	luaState["GAME_ID"] = Input02;

	auto _result = luaState.script_file(Input01);

	bootFunction = luaState["_OnBoot"];
	initFunction = luaState["_OnInit"];
	frameFunction = luaState["_OnFrame"];

	if (!bootFunction)
		Console.WriteLn(Color_Red, "LuaEngine: The \"_OnBoot\" function either has errors or does not exist.");

	if (!initFunction)
		Console.WriteLn(Color_Red, "LuaEngine: The \"_OnInit\" function either has errors or does not exist.");

	if (!frameFunction)
		Console.WriteLn(Color_Red, "LuaEngine: The \"_OnFrame\" function either has errors or does not exist.");

	if (!initFunction && !frameFunction && !bootFunction)
	{
		Console.WriteLn(Color_Red, "\nLuaEngine: None of the needed functions exist or all of them have errors.");
		Console.WriteLn(Color_Red, "LuaEngine: Initialization of this script cannot continue...\n");
		return;
	}

	Console.WriteLn(Color_Green, "LuaEngine: Initialization Successful!");
	Console.WriteLn(Color_Black, "");

	if (initFunction)
		initFunction();
}


void LuaPS2::SetFunctions()
{
	// Readers
	luaState.set_function("ReadByte",    Read_UInt08);
	luaState.set_function("ReadShort",   Read_UInt16);
	luaState.set_function("ReadInt",     Read_UInt32);
	luaState.set_function("ReadLong",    Read_UInt64);
	luaState.set_function("ReadBoolean", Read_Boolean);
	luaState.set_function("ReadFloat",   Read_Single);
	luaState.set_function("ReadString",  Read_String);
	luaState.set_function("ReadArray",   Read_UInt08_Array);

	// Calculators
	luaState.set_function("GetPointer", Calculate_Pointer);
    luaState.set_function("ULShift32", UnsignedShift32);

	// IO Operations
	luaState.set_function("ReadFile", sol::overload(File_Read, File_ReadRegion));
    luaState.set_function("ConsolePrint", sol::overload([this](sol::object Text) {
                              Console.WriteLn(Color_StrongMagenta, "[" + luaState.globals()["LUA_NAME"].get<string>() + "] " + Text.as<string>());
                          },
                                                        [this](sol::object Text, int MessageType) {
                                                            auto _scrString = "[" + luaState.globals()["LUA_NAME"].get<string>() + "] ";
                                                            switch (MessageType) {
                                                                default:
                                                                    Console.WriteLn(Color_StrongMagenta, _scrString + "MESSAGE: " + Text.as<string>());
                                                                    break;
                                                                case 1:
                                                                    Console.WriteLn(Color_StrongGreen, _scrString + "SUCCESS: " + Text.as<string>());
                                                                    break;
                                                                case 2:
                                                                    Console.WriteLn(Color_StrongOrange, _scrString + "WARNING: " + Text.as<string>());
                                                                    break;
                                                                case 3:
                                                                    Console.WriteLn(Color_StrongRed, _scrString + "ERROR: " + Text.as<string>());
                                                                    break;
                                                            }
                                                        }));

	// Writers
	luaState.set_function("WriteByte",    Write_UInt08);
	luaState.set_function("WriteShort",   Write_UInt16);
	luaState.set_function("WriteInt",     Write_UInt32);
	luaState.set_function("WriteLong",    Write_UInt64);
	luaState.set_function("WriteBoolean", Write_Boolean);
	luaState.set_function("WriteFloat",   Write_Single);
	luaState.set_function("WriteString",  Write_String);
	luaState.set_function("WriteArray",   Write_UInt08_Array);

	luaState.set_function("GetHertz", []() { return 60; });
    luaState.set_function("SetHertz", []() { return false; });
}

