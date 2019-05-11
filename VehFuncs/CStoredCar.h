#pragma once
#ifndef CStoredCar_h
#define CStoredCar_h
#include <cstdint>
#include <game_sa\CVector.h>

/*00000000 pos             RwV3D ?
0000000C handling_flags  dd ?
00000010 flags           db ? ; 0x1 = BulletProof(immunities | 0x40000)
00000010; 0x2 = FireProof(immunities | 0x80000)
00000010; 0x4 = ExplosionProof(immunities | 0x800000)
00000010; 0x8 = CollisionProof(immunities | 0x100000)
00000010; 0x10 = MeeleProof(immunities | 0x200000)
00000010; 0x20 = HasBassUpgrade
00000010; 0x40 = HasHydraulics(handlingFlags | 0x20000)
00000010; 0x80 = HasNitro(handlingFlags | 0x80000)
00000011 field_11        db ?
00000012 model           dw ?
00000014 carmods         dw 15 dup(? )
00000032 colour          db 4 dup(? )
00000036 radio_station   db ? ; go into CVehicle + 0x1D2
00000037 extra1          db ?
00000038 extra2          db ?
00000039 bomb_type       db ? ; first 3 bits from CVehicle + 0x4A8
0000003A paintjob        db ?
0000003B nitro_count     db ?
0000003C angleX          db ? ; multiply by 0.009999999776482582 to get real value
0000003D angleY          db ?
0000003E angleZ          db ?
0000003F field_3F        db ?*/

#pragma pack(push, 1)
class CStoredCar
{
public:
	CVector pos;
	uint32_t handling_flags;
	uint8_t flags;
	uint8_t field_11;
	uint16_t vehicle;
	uint16_t carmods[15];
	uint8_t colour[4];
	uint8_t radio_station;
	uint8_t extra1;
	uint8_t extra2;
	uint8_t bomb_type;
	uint8_t paintjob;
	uint8_t nitro_count;
	uint8_t angleX;
	uint8_t angleY;
	uint8_t angleZ;
	uint8_t field_3F;
};
#pragma pack(pop)

VALIDATE_SIZE(CStoredCar, 0x40);
#endif
