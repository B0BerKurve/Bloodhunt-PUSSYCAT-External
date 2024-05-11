#include "function.h"
#include "overlay.h"
#include "driver.h"
#include "xorstr.hpp"

namespace OverlayWindow
{
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

void PrintPtr(std::string text, uintptr_t ptr) {
	std::cout << text << ptr << std::endl;
}

namespace DirectX9Interface
{
	IDirect3D9Ex* Direct3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParams = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}
typedef struct _EntityList
{
	uintptr_t actor_pawn;
	uintptr_t actor_mesh;
	uintptr_t actor_state;
	uintptr_t actor_root;
	int actor_id;
}EntityList;
std::vector<EntityList> entityList;

auto CallAimbot() -> VOID
{
	while (true)
	{
		auto EntityList_Copy = entityList;

		bool isAimbotActive = CFG.b_Aimbot && GetAimKey();
		if (isAimbotActive)
		{
			float target_dist = FLT_MAX;
			EntityList target_entity = {};

			for (int index = 0; index < EntityList_Copy.size(); ++index)
			{
				auto Entity = EntityList_Copy[index];

				if (!Entity.actor_mesh)
					continue;

				auto OutlineComponent = read<uintptr_t>(Entity.actor_pawn + GameOffset.offset_OutlineComponent); // get outline component for simple mode check
				auto mode = read<uint8_t>(OutlineComponent + GameOffset.offset_CurrentMode); // get mode from component
				auto Health = read<float>(Entity.actor_pawn + GameOffset.offset_health);

				if (mode == 9)
				{           // if mode 9 its team mate NO EXECUTE AIM
					continue;
				}

				if (Health > 0.f)
				{
					auto head_pos = GetBoneWithRotation(Entity.actor_mesh, 24);
					auto targethead = WorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 5));

					float x = targethead.x - GameVars.ScreenWidth / 2.0f;
					float y = targethead.y - GameVars.ScreenHeight / 2.0f;
					float crosshair_dist = sqrtf((x * x) + (y * y));

					if (crosshair_dist <= FLT_MAX && crosshair_dist <= target_dist)
					{
						if (crosshair_dist > CFG.fl_AimFov) // FOV
							continue;

						target_dist = crosshair_dist;
						target_entity = Entity;

					}
				}
			}

			if (target_entity.actor_mesh != 0 || target_entity.actor_pawn != 0 || target_entity.actor_id != 0)
			{

				if (target_entity.actor_pawn == GameVars.local_player_pawn)
					continue;

				if (!isVisible(target_entity.actor_mesh))
					continue;

				auto head_pos = GetBoneWithRotation(target_entity.actor_mesh, 24);
				auto targethead = WorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 5));
				move_to(targethead.x, targethead.y);
			}
		}
		Sleep(10);
	}
}

auto GameCache() -> VOID
{
	while (true)
	{
		std::vector<EntityList> tmpList;

		GameVars.u_world = read<DWORD_PTR>(GameVars.dwProcess_Base + GameOffset.offset_uworld);
		GameVars.game_instance = read<DWORD_PTR>(GameVars.u_world + GameOffset.offset_game_instance); //OwningGameInstance
		GameVars.local_player_array = read<DWORD_PTR>(GameVars.game_instance + GameOffset.offset_local_players_array); // LocalPlayers
		GameVars.local_player = read<DWORD_PTR>(GameVars.local_player_array);
		GameVars.local_player_controller = read<DWORD_PTR>(GameVars.local_player + GameOffset.offset_player_controller); // AcknowledgedPawn
		GameVars.local_player_pawn = read<DWORD_PTR>(GameVars.local_player_controller + GameOffset.offset_apawn); // AcknowledgedPawn
		GameVars.local_player_root = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_root_component); // RootComponent
		GameVars.local_player_state = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_player_state); // PlayerState
		GameVars.ranged_weapon_component = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_ranged_weapon_component); // PersistentLevel
		GameVars.equipped_weapon_type = read<DWORD_PTR>(GameVars.ranged_weapon_component + GameOffset.offset_equipped_weapon_type); // MaxPacket
		GameVars.persistent_level = read<DWORD_PTR>(GameVars.u_world + GameOffset.offset_persistent_level); // OwningActor
		GameVars.actors = read<DWORD_PTR>(GameVars.persistent_level + GameOffset.offset_actor_array);
		GameVars.actor_count = read<int>(GameVars.persistent_level + GameOffset.offset_actor_count);

		for (int index = 0; index < GameVars.actor_count; ++index)
		{
			auto actor_pawn = read<uintptr_t>(GameVars.actors + index * 0x8);
			if (!actor_pawn)continue;

			auto actor_id = read<int>(actor_pawn + GameOffset.offset_actor_id);
			if (!actor_id)continue;

			auto actor_mesh = read<uintptr_t>(actor_pawn + GameOffset.offset_actor_mesh); // Mesh
			if (!actor_mesh)continue;

			auto actor_state = read<uintptr_t>(actor_pawn + GameOffset.offset_player_state); //PlayerState
			if (!actor_state)continue;

			auto actor_root = read<uintptr_t>(actor_pawn + GameOffset.offset_root_component); // RootComponent
			if (!actor_root)continue;

			auto name = BloodHunt::GetNameFromFName(actor_id);

			if (actor_pawn == GameVars.local_player_pawn) // Ignore Self?
				continue;

			EntityList Entity{ };
			Entity.actor_pawn = actor_pawn;

			if (actor_id == GameVars.Local_PlayerID || actor_id == GameVars.Local_PlayerID + 765 || name == "TBP_ElysiumPlayer_C" || name == "TBP_Player_C")
			{
				if (actor_mesh != NULL)
				{
					Entity.actor_id = actor_id;
					Entity.actor_state = actor_state;
					Entity.actor_root = actor_root;
					Entity.actor_mesh = actor_mesh;
					tmpList.push_back(Entity);
				}
			}
			/*if (GetAsyncKeyState(VK_F4) & 1)
				b_BotEsp = !b_BotEsp;

			if (b_BotEsp)
			{
				if (name == "TBP_NPC_Primogen_C" || name == "TBP_NPC_C" )
				{
					if (actor_mesh != NULL)
					{
						Entity.actor_mesh = actor_mesh;
						tmpList.push_back(Entity);
					}
				}
			}*/


		}
		entityList = tmpList;
		Sleep(100);
	}
}

auto RenderVisual() -> VOID
{
	auto EntityList_Copy = entityList;

	for (int index = 0; index < EntityList_Copy.size(); ++index)
	{
		EntityList Entity = EntityList_Copy[index];
		float max = CFG.fl_AimFov / 2;
		auto head_pos = GetBoneWithRotation(Entity.actor_mesh, 24);
		auto bone_pos = GetBoneWithRotation(Entity.actor_mesh, 0);
		auto BottomBox = WorldToScreen(bone_pos);
		auto HeadBox = WorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 5));

		auto Health = read<float>(Entity.actor_pawn + GameOffset.offset_health);
		auto MaxHealth = read<float>(Entity.actor_pawn + GameOffset.offset_max_health); // idk how this works lmao
		auto procentage = Health * 100 / MaxHealth;
		//auto Sheild = read<uint16_t>(Entity.actor_pawn + GameOffset.offset_CurrentShield);

		auto local_pos = read<Vector3>(GameVars.local_player_root + GameOffset.offset_relative_location);

		auto TopBox = WorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 15));

		auto entity_distance = local_pos.Distance(bone_pos);

		auto CornerHeight = abs(TopBox.y - BottomBox.y);
		auto CornerWidth = CornerHeight * 0.65;

		auto bVisible = isVisible(Entity.actor_mesh);
		auto ESP_Color = GetVisibleColor(bVisible);

		auto Bodytype = read<uint8_t>(Entity.actor_pawn + GameOffset.offset_BodyType);

		auto draw = ImGui::GetBackgroundDrawList();

		if (CFG.b_drawcros)
		{
			draw->AddCircle(ImVec2(GameVars.ScreenWidth / 2, GameVars.ScreenHeight / 2), 6, IM_COL32(255, 255, 255, 255), 100, 0.0f);
		}

		if (CFG.b_Aimbot)
		{
			if (CFG.b_AimFOV)
			{
				DrawCircle(GameVars.ScreenWidth / 2, GameVars.ScreenHeight / 2, CFG.fl_AimFov, CFG.FovColor, 0);
			}
		}

		if (CFG.b_Visual)
		{
			if (Health > 0.f)
			{
				if (entity_distance < CFG.distance)
				{
					if (CFG.b_TeamCheck)
					{
						auto OutlineComponent = read<uintptr_t>(Entity.actor_pawn + GameOffset.offset_OutlineComponent); // get outline component for simple mode check
						auto mode = read<uint8_t>(OutlineComponent + GameOffset.offset_CurrentMode); // get mode from component

						if (mode == 9)// if mode 9 its team mate
						{
							//here inside will be everything that will appear in friends
							char ID[64];
							DWORD offset_GroupID = 0x43c; // new 27
							auto Group_ID = read<int32_t>(Entity.actor_state + GameOffset.offset_GroupID);
							sprintf_s(ID, "", Group_ID);
							DrawOutlinedText(Verdana, ID, ImVec2(BottomBox.x, BottomBox.y + 10), 14.0f, ImColor(0, 255, 0), true);

						}
						else
						{
							if (CFG.b_EspBox)
							{
								DrawCorneredBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ESP_Color, 1.5);
							}

							if (CFG.b_EspLine)
							{

								if (CFG.in_LineType == 0)
								{
									DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight)), ImVec2(BottomBox.x, BottomBox.y), CFG.LineColor, 1.5f); //LINE FROM BOTTOM SCREEN
								}
								if (CFG.in_LineType == 1)
								{
									DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), 0.f), ImVec2(BottomBox.x, BottomBox.y), CFG.LineColor, 1.5f); //LINE FROM TOP SCREEN
								}
								if (CFG.in_LineType == 2)
								{
									DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight / 2)), ImVec2(BottomBox.x, BottomBox.y), CFG.LineColor, 1.5f); //LINE FROM CROSSHAIR
								}
							}
							if (CFG.b_espName)
							{
								auto PlayerName = read<FString>(Entity.actor_state + GameOffset.offset_player_name);
								DrawOutlinedText(Verdana, PlayerName.ToString(), ImVec2(HeadBox.x, HeadBox.y - 20), 14.0f, ImColor(255, 255, 255), true);
							}
							if (CFG.b_EspDistance)
							{
								char dist[64];
								sprintf_s(dist, "%.fm", entity_distance);
								DrawOutlinedText(Verdana, dist, ImVec2(BottomBox.x, BottomBox.y), 14.0f, ImColor(255, 255, 255), true);
							}
							if (CFG.b_EspHealth)
							{
								float width = CornerWidth / 10;
								if (width < 2.f) width = 2.;
								if (width > 3) width = 3.;
	
								HealthBar(HeadBox.x - (CornerWidth / 2) - 7, HeadBox.y, width, BottomBox.y - HeadBox.y, procentage, true);
								
							}

							if (CFG.b_EspSkeleton)
							{
								if (Bodytype == 0) // Female = 0, Male = 1,
								{
									Vector3 vHeadOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 24));
									Vector3 vNeckOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 23));
									Vector3 vSpine1Out = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 20));
									Vector3 vSpine2Out = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 21));
									Vector3 vSpine3Out = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 22));
									Vector3 vPelvisOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 1));
									Vector3 vRightThighOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 11));
									Vector3 vLeftThighOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 2));
									Vector3 vRightCalfOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 12));
									Vector3 vLeftCalfOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 3));
									Vector3 vRightFootOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 13));
									Vector3 vLeftFootOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 4));
									Vector3 vUpperArmRightOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 94));
									Vector3 vUpperArmLeftOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 53));
									Vector3 vLowerArmRightOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 98));
									Vector3 vLowerArmLeftOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 57));
									Vector3 vHandRightOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 102));
									Vector3 vHandLeftOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 61));
									Vector3 vFootOutOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 159));

									DrawLine(ImVec2(vHeadOut.x, vHeadOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vNeckOut.x, vNeckOut.y), ImVec2(vSpine1Out.x, vSpine1Out.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vSpine2Out.x, vSpine2Out.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine2Out.x, vSpine2Out.y), ImVec2(vSpine3Out.x, vSpine3Out.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vPelvisOut.x, vPelvisOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vPelvisOut.x, vPelvisOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vPelvisOut.x, vPelvisOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightFootOut.x, vRightFootOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftFootOut.x, vLeftFootOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vLowerArmRightOut.x, vLowerArmRightOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vLowerArmLeftOut.x, vLowerArmLeftOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLowerArmLeftOut.x, vLowerArmLeftOut.y), ImVec2(vHandLeftOut.x, vHandLeftOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLowerArmRightOut.x, vLowerArmRightOut.y), ImVec2(vHandRightOut.x, vHandRightOut.y), ESP_Color, 1.5f);
								}
								else if (Bodytype == 1) {
									Vector3 vHeadOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 6));
									Vector3 vNeckOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 5));
									Vector3 vSpine1Out = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 2));
									Vector3 vSpine2Out = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 3));
									Vector3 vSpine3Out = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 4));
									Vector3 vPelvisOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 1));
									Vector3 vRightThighOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 121));
									Vector3 vLeftThighOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 112));
									Vector3 vRightCalfOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 124));
									Vector3 vLeftCalfOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 115));
									Vector3 vRightFootOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 125));
									Vector3 vLeftFootOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 116));
									Vector3 vUpperArmRightOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 76));
									Vector3 vUpperArmLeftOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 35));
									Vector3 vLowerArmRightOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 77));
									Vector3 vLowerArmLeftOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 36));
									Vector3 vHandRightOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 78));
									Vector3 vHandLeftOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 37));
									Vector3 vFootOutOut = BloodHunt::WorldToScreen(BloodHunt::GetBoneWithRotation(Entity.actor_mesh, 160));

									DrawLine(ImVec2(vHeadOut.x, vHeadOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vNeckOut.x, vNeckOut.y), ImVec2(vSpine1Out.x, vSpine1Out.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vSpine2Out.x, vSpine2Out.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine2Out.x, vSpine2Out.y), ImVec2(vSpine3Out.x, vSpine3Out.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vSpine1Out.x, vSpine1Out.y), ImVec2(vPelvisOut.x, vPelvisOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vPelvisOut.x, vPelvisOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vPelvisOut.x, vPelvisOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightFootOut.x, vRightFootOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftFootOut.x, vLeftFootOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vLowerArmRightOut.x, vLowerArmRightOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vLowerArmLeftOut.x, vLowerArmLeftOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLowerArmLeftOut.x, vLowerArmLeftOut.y), ImVec2(vHandLeftOut.x, vHandLeftOut.y), ESP_Color, 1.5f);
									DrawLine(ImVec2(vLowerArmRightOut.x, vLowerArmRightOut.y), ImVec2(vHandRightOut.x, vHandRightOut.y), ESP_Color, 1.5f);
								}
							}
						}
					}
				}
			}
		}
	}
}

void InputHandler() {
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}
void Render()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		CFG.b_MenuShow = !CFG.b_MenuShow;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	RenderVisual();
	ImGui::GetIO().MouseDrawCursor = CFG.b_MenuShow;

	// Set custom colors
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowMinSize = ImVec2(256, 300);
	style.WindowTitleAlign = ImVec2(0.5, 0.5);
	style.FrameBorderSize = 1;
	style.ChildBorderSize = 1;
	style.WindowBorderSize = 1;
	style.WindowRounding = 0;
	style.FrameRounding = 0;
	style.ChildRounding = 0;
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 240);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.90f, 0.43f, 0.80f);
	style.Colors[ImGuiCol_Border] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_Button] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ChildBg] = ImColor(45, 45, 45);
	style.Colors[ImGuiCol_FrameBg] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 255, 255);
	

	static float rainbow;
	rainbow += 0.005f;
	if (rainbow > 1.f)
		rainbow = 0.f;
	DrawOutlinedText(Verdana, (XorStr("P U S S Y C A T").c_str()), ImVec2(55, 12), 12, ImColor::HSV(rainbow, 1.f, 1.f), true);

	if (CFG.b_MenuShow)
	{
		InputHandler();
		ImGui::SetNextWindowSize(ImVec2(675, 400)); // 450,426
		ImGui::PushFont(DefaultFont);
		ImGui::Begin("P U S S Y C A T", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

		ImGui::BeginGroup();

		TabButton("Visuals", &CFG.in_tab_index, 0, false);
		TabButton("Aimbot", &CFG.in_tab_index, 1, false);
		TabButton("Colors", &CFG.in_tab_index, 2, false);

		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();

		if (CFG.in_tab_index == 0)
		{
			ImGui::Checkbox("Enabled Visual", &CFG.b_Visual);
			ImGui::Separator();
			if (CFG.b_Visual)
			{
				
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				ImGui::Checkbox("Draw BOX", &CFG.b_EspBox);
				ImGui::Checkbox("Skeleton", &CFG.b_EspSkeleton);
				ImGui::Checkbox("Tracelines", &CFG.b_EspLine);
				ImGui::Checkbox("PlayerName", &CFG.b_espName);
				ImGui::Checkbox("Distance", &CFG.b_EspDistance);
				ImGui::SliderFloat("Max Distance", &CFG.distance, 1.0f, 1000.0f);
				ImGui::Checkbox("HealthBar", &CFG.b_EspHealth);
				if (CFG.b_EspLine)
				{
					ImGui::Combo("Tracelines Type", &CFG.in_LineType, CFG.LineTypes, 3);
				}
				ImGui::PopStyleVar();
			}
		}
		else if (CFG.in_tab_index == 1)
		{
			ImGui::Checkbox("Enabled Aimbot", &CFG.b_Aimbot);
			ImGui::Separator();
			if (CFG.b_Aimbot)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				ImGui::Checkbox("Draw FOV", &CFG.b_AimFOV);
				ImGui::SliderFloat("Smoothing", &CFG.Smoothing, 1.0f, 10.0f);
				if (CFG.fl_AimFov)
				{
					ImGui::SliderFloat("Field of View Size", &CFG.fl_AimFov, 1.0f, 300.f);
				}
				ImGui::Combo("Aimbot Key", &CFG.AimKey, keyItems, IM_ARRAYSIZE(keyItems));
				ImGui::PopStyleVar();
			}
				
		}
		else if (CFG.in_tab_index == 2)
		{
			
			if (ImGui::ColorEdit3("Skeleton Color", CFG.fl_SkeletonColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.SkeletonColor = ImColor(CFG.fl_SkeletonColor[0], CFG.fl_SkeletonColor[1], CFG.fl_SkeletonColor[2]);
			}
			if (ImGui::ColorEdit3("Line Color", CFG.fl_LineColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.LineColor = ImColor(CFG.fl_LineColor[0], CFG.fl_LineColor[1], CFG.fl_LineColor[2]);
			}
			if (ImGui::ColorEdit3("Visible Color", CFG.fl_VisibleColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.VisibleColor = ImColor(CFG.fl_VisibleColor[0], CFG.fl_VisibleColor[1], CFG.fl_VisibleColor[2]);
			}
			if (ImGui::ColorEdit3("Invisible Color", CFG.fl_InvisibleColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.InvisibleColor = ImColor(CFG.fl_InvisibleColor[0], CFG.fl_InvisibleColor[1], CFG.fl_InvisibleColor[2]);
			}
			if (ImGui::ColorEdit3("FOV Color", CFG.fl_FovColor, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs))
			{
				CFG.FovColor = ImColor(CFG.fl_FovColor[0], CFG.fl_FovColor[1], CFG.fl_FovColor[2]);
			}
		}
		ImGui::EndGroup();

		ImGui::PopFont();
		ImGui::End();
	}
	ImGui::EndFrame();

	DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9Interface::pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9Interface::pDevice->EndScene();
	}

	HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));

	while (DirectX9Interface::Message.message != WM_QUIT) {
		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9Interface::Message);
			DispatchMessage(&DirectX9Interface::Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == GameVars.gameHWND) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameVars.gameHWND;

		POINT TempPoint2;
		GetCursorPos(&TempPoint2);
		io.MousePos.x = TempPoint2.x - TempPoint.x;
		io.MousePos.y = TempPoint2.y - TempPoint.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else {
			io.MouseDown[0] = false;
		}

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			GameVars.ScreenWidth = TempRect.right;
			GameVars.ScreenHeight = TempRect.bottom;
			DirectX9Interface::pParams.BackBufferWidth = GameVars.ScreenWidth;
			DirectX9Interface::pParams.BackBufferHeight = GameVars.ScreenHeight;
			SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, GameVars.ScreenWidth, GameVars.ScreenHeight, SWP_NOREDRAW);
			DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9Interface::pDevice != NULL) {
		DirectX9Interface::pDevice->EndScene();
		DirectX9Interface::pDevice->Release();
	}
	if (DirectX9Interface::Direct3D9 != NULL) {
		DirectX9Interface::Direct3D9->Release();
	}
	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

bool DirectXInit() {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) {
		return false;
	}

	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = OverlayWindow::Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = GameVars.ScreenWidth;
	Params.BackBufferHeight = GameVars.ScreenHeight;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D16;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice))) {
		DirectX9Interface::Direct3D9->Release();
		return false;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
	ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
	DirectX9Interface::Direct3D9->Release();
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9Interface::pDevice != NULL) {
			DirectX9Interface::pDevice->EndScene();
			DirectX9Interface::pDevice->Release();
		}
		if (DirectX9Interface::Direct3D9 != NULL) {
			DirectX9Interface::Direct3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
			DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	OverlayWindow::WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
	};

	RegisterClassEx(&OverlayWindow::WindowClass);
	if (GameVars.gameHWND) {
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		GameVars.ScreenWidth = TempRect.right;
		GameVars.ScreenHeight = TempRect.bottom;
	}

	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, GameVars.ScreenLeft, GameVars.ScreenTop, GameVars.ScreenWidth, GameVars.ScreenHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
}

bool CreateConsole = true;


int main()
{
	system("kdmapper.exe driver.sys");
	driver::find_driver();
	ProcId = driver::find_process(GameVars.dwProcessName);
	BaseId = driver::find_image();
	GameVars.dwProcessId = ProcId;
	GameVars.dwProcess_Base = BaseId;
	printf("Driver: %d\n", driver_handle);
	PrintPtr("ProcessId: ", GameVars.dwProcessId);
	PrintPtr("BaseId: ", GameVars.dwProcess_Base);



	HWND tWnd = FindWindowA("UnrealWindow", nullptr);
	if (tWnd)
	{

		GameVars.gameHWND = tWnd;
		RECT clientRect;
		GetClientRect(GameVars.gameHWND, &clientRect);
		POINT screenCoords = { clientRect.left, clientRect.top };
		ClientToScreen(GameVars.gameHWND, &screenCoords);
		printf("HWND: %d\n", tWnd);
	}

	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GameCache), nullptr, NULL, nullptr);
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CallAimbot), nullptr, NULL, nullptr);

	if (CreateConsole == false)
		ShowWindow(GetConsoleWindow(), SW_HIDE);

	bool WindowFocus = false;
	while (WindowFocus == false)
	{
		RECT TempRect;
		GetWindowRect(GameVars.gameHWND, &TempRect);
		GameVars.ScreenWidth = TempRect.right - TempRect.left;
		GameVars.ScreenHeight = TempRect.bottom - TempRect.top;
		GameVars.ScreenLeft = TempRect.left;
		GameVars.ScreenRight = TempRect.right;
		GameVars.ScreenTop = TempRect.top;
		GameVars.ScreenBottom = TempRect.bottom;
		WindowFocus = true;

	}

	OverlayWindow::Name = RandomString(10).c_str();
	SetupWindow();
	DirectXInit();

	ImGuiIO& io = ImGui::GetIO();
	DefaultFont = io.Fonts->AddFontDefault();
	Verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 16.0f);
	io.Fonts->Build();


	while (TRUE)
	{
		MainLoop();
	}

}
