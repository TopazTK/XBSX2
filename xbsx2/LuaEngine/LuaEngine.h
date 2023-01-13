#ifndef LUA_HEAD
#define LUA_HEAD

#include "Memory.h"

#include <iostream>
#include <fstream>

#include <memory>
#include <vector>

#include <common/FileSystem.h>
#include <common/StringUtil.h>
#include <common/Path.h>

enum LUAExecutionTime
{
	SPT_ONCE_ON_LOAD = 0,
	SPT_CONTINOUSLY = 1,

	_SPT_END_MARKER
};

extern void ForgetScripts();
extern void ExecuteScript(LUAExecutionTime Input01);
extern int LoadScriptFromDir(const std::string Input01, const std::string& Input02, const char* Input03);

#endif