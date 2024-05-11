#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include "singleton.h"
#include "imgui/imgui.h"

DWORD keys[] = { VK_LMENU, VK_MENU, VK_SHIFT, VK_RSHIFT, VK_CONTROL, VK_RCONTROL, VK_LBUTTON, VK_RBUTTON };
const char* keyItems[] = { "LAlt", "RAlt", "LShift", "RShift", "LControl", "RControl", "LMouse", "RMouse" };

inline namespace Configuration
{
	class Settings : public Singleton<Settings>
	{
	public:

		const char* BoxTypes[2] = { "Full Box","Cornered Box" };
		const char* LineTypes[3] = { "Bottom To Enemy","Top To Enemy","Crosshair To Enemy" };


		bool b_MenuShow = false;

		bool b_Aimbot = true;
		bool b_AimFOV = true;

		bool b_TeamCheck = true;

		bool b_Smoothing = false;
		bool b_fastres = false;
		bool b_NoRecoil = false;
		bool b_Fastreload = false;
		bool NoSpread = false;
		bool b_Rapidfire = false;
		bool b_Visual = true;

		bool b_drawcros = false;

		bool b_EspBox = false;
		bool b_EspSkeleton = true;
		bool b_EspLine = false;
		bool b_EspDistance = true;
		bool b_EspHealth = true;
		bool b_espName = true;

		ImColor InvisibleColor = ImColor(255.f / 255, 0.f, 0.f);
		float fl_InvisibleColor[3] = { 0.f,255.f / 255,0.f };  //

		ImColor VisibleColor = ImColor(0.f, 255.f / 255, 0.f);
		float fl_VisibleColor[3] = { 255.f / 255,0.f,0.f };  //

		ImColor FovColor = ImColor(255.f / 255, 0.f, 0.f);
		float fl_FovColor[3] = { 255.f / 255,0.f,0.f };  //

		ImColor BoxColor = ImColor(255.f / 255, 0.f, 0.f);
		float fl_BoxColor[3] = { 255.f / 255,0.f,0.f };//

		ImColor SkeletonColor = ImColor(255.f / 255, 255.f / 255, 255.f / 255);
		float fl_SkeletonColor[3] = { 255.f / 255,255.f / 255,255.f / 255 };//

		ImColor LineColor = ImColor(255.f / 255, 255.f / 255, 255.f / 255);
		float fl_LineColor[3] = { 255.f / 255,255.f / 255,255.f / 255 };  //


		float fl_CurrentFOV;
		float fl_SmoothingValue = 0.1f; // from 0-1
		float fl_Speed = 7000.0f;

		int in_LineType = 0;
		int in_tab_index = 0;

		float distance = 500.0f;
		int AimKey = 0;
		float fl_AimFov = 120.0f;
		float Smoothing = 2.0f;


	};
#define CFG Configuration::Settings::Get()
};

bool GetAimKey()
{
	return GetAsyncKeyState(keys[CFG.AimKey]);
}
