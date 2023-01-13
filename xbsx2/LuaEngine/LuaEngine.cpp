#include "PrecompiledHeader.h"

#include "LuaPS2.h"


using namespace sol;
using namespace std;

vector<std::string> _scriptNames;
vector<LuaPS2*> _loadedScripts;

void ForgetScripts()
{
	_scriptNames.clear();

	for (auto&& _p : _loadedScripts)
		delete(_p);

	_loadedScripts.clear();
}

int LoadScriptFromDir(const std::string name, const std::string& folder, const char* friendly_name)
{
	Console.WriteLn(Color_Black, "");
	Console.WriteLn(Color_StrongBlue, "Initializing LuaEngine v4.1");
	Console.WriteLn(Color_Black, "");

	u32 _checksum;
	sscanf(name.c_str(), "%x", &_checksum);

	FileSystem::FindResultsArray files;
	FileSystem::FindFiles(folder.c_str(), StringUtil::StdStringFromFormat("*.lua", name.c_str()).c_str(),
		FILESYSTEM_FIND_FILES | FILESYSTEM_FIND_HIDDEN_FILES, &files);

	if (files.empty())
	{
		Console.WriteLn(Color_Red, "LuaEngine: The \"lua\" folder does not exist! Aborting Initialization...");
		return -1;
	}

	int _interval = 0;
	int _loaded = 0;

	for (const FILESYSTEM_FIND_DATA& fd : files)
	{
		const std::string_view fetchname(Path::GetFileName(fd.FileName));

		if (fetchname.length() < name.length() || StringUtil::Strncasecmp(fetchname.data(), name.c_str(), name.size()) != 0)
			continue;

		else 
		{
			try
			{
				if (fetchname == _scriptNames.at(_interval)) 
				{
					_interval++;
					continue;
				}

				else
				{
					Console.WriteLn(Color_Green, "LuaEngine: Found Lua Script: '%s'! Initializing...");
					_scriptNames.push_back(fetchname.data());
					_loadedScripts.push_back(new LuaPS2(Path::Combine(folder, fetchname), _checksum));
					_loaded++;
				}
			}
			
			catch (const out_of_range& e)
			{
				Console.WriteLn(Color_Green, "LuaEngine: Found Lua Script: '%s'! Initializing...");
				_scriptNames.push_back(fetchname.data());
				_loadedScripts.push_back(new LuaPS2(Path::Combine(folder, fetchname), _checksum));
				_loaded++;
			}

			_interval++;
		}
	}

	if (_loaded == 0)
		Console.WriteLn(Color_Orange, "LuaEngine: No Lua scripts were found for this game.");

	return _loaded;
}

void ExecuteScript(LUAExecutionTime Input01)
{
	for (auto _script : _loadedScripts)
	{
		if (!_script->bootFunction && !_script->frameFunction)
			return;

		switch (Input01)
		{
			case SPT_ONCE_ON_LOAD:
				if (_script->bootFunction)
					_script->bootFunction();
				break;
			case SPT_CONTINOUSLY:
				if (_script->frameFunction)
					_script->frameFunction();
				break;
		}
	}
}
