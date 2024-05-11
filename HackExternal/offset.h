
#pragma once

#ifndef BLOODHUNT_H


#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include <dwmapi.h>
#include  <d3d9.h>
#include  <d3dx9.h>

#include "singleton.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")



#define M_PI 3.14159265358979323846264338327950288419716939937510



class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	float x;
	float y;
	float z;

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(Vector3 v)
	{
		float x = this->x - v.x;
		float y = this->y - v.y;
		float z = this->z - v.z;

		return sqrtf((x * x) + (y * y) + (z * z)) * 0.03048f;
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(float number) const {
		return Vector3(x * number, y * number, z * number);
	}
};

struct FQuat
{
	float x;
	float y;
	float z;
	float w;
};

struct FTransform
{
	FQuat rot;
	Vector3 translation;
	char pad[4];
	Vector3 scale;
	char pad1[4];

	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

float width = 0;
float height = 0;

inline namespace BloodHunt
{
	class Variables : public Singleton<Variables>
	{
	public:
		const char* dwProcessName = "Tiger-Win64-Shipping.exe";
		DWORD dwProcessId = NULL;
		uint64_t dwProcess_Base = NULL;
		HWND gameHWND = NULL;

		int Local_PlayerID = NULL;
		int ScreenHeight = NULL;
		int ScreenWidth = NULL;
		int ScreenLeft = NULL;
		int ScreenRight = NULL;
		int ScreenTop = NULL;
		int ScreenBottom = NULL;

		float ScreenCenterX = ScreenWidth / 2;
		float ScreenCenterY = ScreenHeight / 2;

		DWORD_PTR game_instance = NULL;
		DWORD_PTR u_world = NULL;
		DWORD_PTR local_player_pawn = NULL;
		DWORD_PTR local_player_array = NULL;
		DWORD_PTR local_player = NULL;
		DWORD_PTR local_player_root = NULL;
		DWORD_PTR local_player_controller = NULL;
		DWORD_PTR local_player_state = NULL;
		DWORD_PTR persistent_level = NULL;
		DWORD_PTR actors = NULL;
		DWORD_PTR ranged_weapon_component = NULL;
		DWORD_PTR equipped_weapon_type = NULL;
		int actor_count = NULL;
		int local_player_id = NULL;

	};
#define GameVars BloodHunt::Variables::Get()

	class Offsets : public Singleton<Offsets>
	{
	public:
		DWORD offset_uworld = 0x5d209c0;
		DWORD offset_gnames = 0x5bd92c0;
		DWORD offset_camera_manager = 0x2c0; // APlayerController->PlayerCameraManager
		DWORD offset_camera_cache = 0x1ab0; //APlayerCameraManager->CameraCachePrivate   
		DWORD offset_persistent_level = 0x30; //UWorld->PersistentLevel
		DWORD offset_game_instance = 0x180; //UWolrd->OwningGameInstance
		DWORD offset_local_players_array = 0x38; //UGameInstance->LocalPlayers
		DWORD offset_local_player_pawn = 0x3a0;
		DWORD offset_player_controller = 0x30; //UPlayer->PlayerController´
		DWORD offset_apawn = 0x2a8;  //APlayerController->AcknowledgedPawn
		DWORD offset_root_component = 0x138; //AActor->RootComponent
		DWORD offset_actor_array = 0x98; //UNetConnection->OwningActor
		DWORD offset_actor_count = 0xa0; //UNetConnection->MaxPacket
		DWORD offset_actor_id = 0x18; //0x22c
		DWORD offset_player_name = 0x308;
		DWORD offset_player_state = 0x248; //APawn->PlayerState
		DWORD offset_actor_mesh = 0x288; //ACharacter->Mesh
		DWORD offset_bone_array = 0x4b0;
		DWORD offset_component_to_world = 0x1c0; //0x1c0
		DWORD offset_ranged_weapon_component = 0x4c8;
		DWORD offset_melee_weapon_component = 0x4d0;
		DWORD offset_equipped_melee_weapon_type = 0x188;
		DWORD offset_equipped_weapon_type = 0x168;
		DWORD offset_relative_location = 0x11c; //USceneComponent->RelativeLocation 
		DWORD offset_relative_rotation = 0x128; // USceneComponent	RelativeRotation	0x128	FRotator
		DWORD offset_last_submit_time = 0x288;
		DWORD offset_last_render_time = 0x28C;
		DWORD offset_health = 0x760; //CurrentHealth
		DWORD offset_max_health = 0x5a4; //BaseMaxHealth
		DWORD offset_GroupStateComponent = 0x338;
		DWORD offset_actor_state = 0x18; //0x22c
		DWORD RangedWeaponComponent = 0x4c8;//ATigerCharacter-> UTigerRangedWeaponComponent
		DWORD bIsFiring = 0x1e2;// UTigerRangedWeaponComponent->bIsFiring
		DWORD bIsAiming = 0x1e4;// UTigerRangedWeaponComponent->bIsAiming
		DWORD EquippedWeaponType = 0x168;// UTigerRangedWeaponComponent->EquippedWeaponType
		DWORD AimedFov = 0x1a4;//UTigerRangedWeapon->AimedFov 
		DWORD offset_localpwn = 0x398;
		DWORD offset_bIsDowned = 0xe70;
		DWORD offset_bIsDead = 0x394;
		DWORD offset_ArchetypeType = 0x3d8;
		DWORD offset_OutlineComponent = 0x4f0;
		DWORD offset_CurrentMode = 0xb0;
		DWORD offset_GroupID = 0x43c;
		DWORD offset_BodyType = 0x7a4;
		DWORD offset_fastres = 0xcf0;
		DWORD offset_CurrentShield = 0x76c;

	};

#define GameOffset BloodHunt::Offsets::Get()

	D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2);
	FTransform GetBoneIndex(DWORD_PTR mesh, int index);
	Vector3 GetBoneWithRotation(uintptr_t mesh, int id);
	D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0));
	Vector3 WorldToScreen(Vector3 WorldLocation);
	std::string GetNameFromFName(int key);
}


#endif  !BLOODHUNT_H

