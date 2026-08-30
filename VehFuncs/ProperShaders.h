#pragma once
#include <windows.h>

// Proper Shaders (MixMods) integration.
//
// ProperShadersAPI.h is copied verbatim from Proper Shaders and resolves every
// entry point with GetProcAddress, so nothing is linked here and VehFuncs keeps
// working normally when Proper Shaders isn't installed - every function pointer
// is simply null.
#include "ProperShadersAPI.h"

// Every entry point of the installed Proper Shaders, or all null when it isn't
// installed (or is too old to export that one). Always null check before calling.
extern PS_Api properShaders;

// Resolves the API. Must be called after the ASI loader brought
// ProperShaders.asi up, so from a game init event and never from DllMain.
void LoadProperShadersApi();

// True when the world is being re-rendered for a shadow map, a reflection
// capture or similar, instead of the scene the player actually sees.
//
// Proper Shaders renders the world several times per frame, and our vehicle
// render hooks fire on every one of them. Per-frame work must be skipped on
// those extra calls, otherwise it runs several times a frame: anims play too
// fast, the odometer counts double, particles are spawned twice...
//
// Always false when Proper Shaders isn't installed, so the caller keeps its
// original once-per-frame behaviour.
bool IsDuplicatedRenderCall();
