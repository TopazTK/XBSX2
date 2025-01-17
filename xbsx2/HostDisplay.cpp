/*  XBSX2 - PS2 Emulator for Xbox Consoles
 *  Copyright (C) 2002-2021  PCSX2 Dev Team
 * 
 *  XBSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  XBSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with XBSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include "HostDisplay.h"
#include "common/Assertions.h"
#include "common/Console.h"
#include "common/StringUtil.h"
#include <cerrno>
#include <cmath>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

std::unique_ptr<HostDisplay> g_host_display;

HostDisplayTexture::~HostDisplayTexture() = default;

HostDisplay::~HostDisplay() = default;

const char* HostDisplay::RenderAPIToString(RenderAPI api)
{
	switch (api)
	{
#define CASE(x) case RenderAPI::x: return #x
		CASE(None);
		CASE(D3D11);
		CASE(D3D12);
#undef CASE
		default:
			return "Unknown";
	}
}

bool HostDisplay::UsesLowerLeftOrigin() const
{
	const RenderAPI api = GetRenderAPI();
	return (api == RenderAPI::OpenGL || api == RenderAPI::OpenGLES);
}

bool HostDisplay::GetHostRefreshRate(float* refresh_rate)
{
	if (m_window_info.surface_refresh_rate > 0.0f)
	{
		*refresh_rate = m_window_info.surface_refresh_rate;
		return true;
	}

	return WindowInfo::QueryRefreshRateForWindow(m_window_info, refresh_rate);
}

bool HostDisplay::SetGPUTimingEnabled(bool enabled)
{
	return false;
}

float HostDisplay::GetAndResetAccumulatedGPUTime()
{
	return 0.0f;
}

bool HostDisplay::ParseFullscreenMode(const std::string_view& mode, u32* width, u32* height, float* refresh_rate)
{
	if (!mode.empty())
	{
		std::string_view::size_type sep1 = mode.find('x');
		if (sep1 != std::string_view::npos)
		{
			std::optional<u32> owidth = StringUtil::FromChars<u32>(mode.substr(0, sep1));
			sep1++;

			while (sep1 < mode.length() && std::isspace(mode[sep1]))
				sep1++;

			if (owidth.has_value() && sep1 < mode.length())
			{
				std::string_view::size_type sep2 = mode.find('@', sep1);
				if (sep2 != std::string_view::npos)
				{
					std::optional<u32> oheight = StringUtil::FromChars<u32>(mode.substr(sep1, sep2 - sep1));
					sep2++;

					while (sep2 < mode.length() && std::isspace(mode[sep2]))
						sep2++;

					if (oheight.has_value() && sep2 < mode.length())
					{
						std::optional<float> orefresh_rate = StringUtil::FromChars<float>(mode.substr(sep2));
						if (orefresh_rate.has_value())
						{
							*width = owidth.value();
							*height = oheight.value();
							*refresh_rate = orefresh_rate.value();
							return true;
						}
					}
				}
			}
		}
	}

	*width = 0;
	*height = 0;
	*refresh_rate = 0;
	return false;
}

std::string HostDisplay::GetFullscreenModeString(u32 width, u32 height, float refresh_rate)
{
	return StringUtil::StdStringFromFormat("%u x %u @ %f hz", width, height, refresh_rate);
}

#include "Frontend/D3D11HostDisplay.h"
#include "Frontend/D3D12HostDisplay.h"

std::unique_ptr<HostDisplay> HostDisplay::CreateDisplayForAPI(RenderAPI api)
{
	switch (api)
	{
		case RenderAPI::D3D11:
			return std::make_unique<D3D11HostDisplay>();
		case RenderAPI::D3D12:
			return std::make_unique<D3D12HostDisplay>();

		default:
			Console.Error("Unknown render API %u", static_cast<unsigned>(api));
			return {};
	}
}

