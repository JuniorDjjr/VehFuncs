//=============================================================================
// ProperShadersAPI.h - public API of ProperShaders.asi
//
// This header is DEPENDENCY-FREE and C-compatible on purpose: copy it into
// your own .asi/.dll project and you are done. Nothing here is linked - every
// entry point is resolved at runtime with GetProcAddress() from the already
// loaded "ProperShaders.asi" module, so your mod keeps working (with the
// effects simply absent) when ProperShaders is not installed.
//
// Minimal use:
//
//     #include "ProperShadersAPI.h"
//
//     PS_Api ps;
//     PS_ApiLoad(&ps);                       // 0 if not installed
//
//     PS_LightDesc d;
//     PS_LightDescInit(&d);                  // ALWAYS call this first
//     d.type        = PS_LIGHT_POINT;
//     d.position[0] = x; d.position[1] = y; d.position[2] = z;
//     d.radius      = 12.0f;
//     d.color[0]    = 1.0f; d.color[1] = 0.6f; d.color[2] = 0.2f;
//     d.intensity   = 2.0f;
//     d.fogMode     = PS_FOGMODE_NORMAL;     // let it glow in volumetric fog
//
//     PS_LightHandle h = ps.LightCreate(&d);
//     ...
//     ps.LightSetPosition(h, x, y, z);       // cheap, call it every frame
//     ...
//     ps.LightDestroy(h);
//
// ABI stability
//   PS_LightDesc is versioned by its own structSize field, which
//   PS_LightDescInit() fills in. Future ProperShaders versions may APPEND
//   fields to it; they will keep reading older, smaller structs correctly, so
//   a mod built against this header does not need recompiling. Never reorder
//   or resize existing fields, and never set structSize by hand.
//=============================================================================

#ifndef PROPERSHADERS_API_H
#define PROPERSHADERS_API_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Versioning
//-----------------------------------------------------------------------------

// Bumped only when the API gains something. Compare against PS_GetApiVersion()
// to find out what the INSTALLED ProperShaders supports; a runtime version
// lower than the one you built against means the newer entry points are
// missing and PS_ApiLoad() will have left them null.
#define PS_API_VERSION 2

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

// Opaque light identifier. 0 is never a valid light, so it doubles as "none".
// Handles are not reused: destroying a light permanently invalidates its
// handle, and calling any function with a stale one fails cleanly rather than
// touching whatever light took over the slot.
typedef unsigned int PS_LightHandle;

#define PS_INVALID_LIGHT ((PS_LightHandle)0)

// A Direct3D 9 texture, as returned by the render target getters. Declared
// rather than included so this header still pulls in nothing: include d3d9.h
// yourself, in either order, to do anything with the pointer.
#ifdef __cplusplus
struct IDirect3DTexture9;
typedef IDirect3DTexture9* PS_Texture;
#else
typedef void* PS_Texture;
#endif

// PS_LightDesc::type
enum PS_LightType
{
    // Omnidirectional. direction[] is ignored.
    PS_LIGHT_POINT = 0,

    // Cone along direction[], with spotAngle as its outer half-angle.
    PS_LIGHT_SPOT  = 1
};

// PS_LightDesc::fogMode - whether the light lights up the volumetric fog
// MEDIUM around it, i.e. the soft glowing ball a street lamp gets on a foggy
// night. Independent from beamMode: this is the omnidirectional in-scattering,
// that one is the visible cone.
enum PS_FogMode
{
    // Lights surfaces only. Cheapest, and correct for lights that should not
    // read as a physical source (a fill light, a gameplay highlight).
    PS_FOGMODE_NONE   = 0,

    // Scatters in the fog. Costs one slot of a small per-frame budget, handed
    // out closest-and-brightest first, so a light far from the camera may not
    // get one. Requires the user to have volumetric fog enabled.
    PS_FOGMODE_NORMAL = 1
};

// PS_LightDesc::beamMode - the visible volumetric cone of a spot light (the
// shaft of a headlamp in the rain). Point lights have no cone, so AUTO leaves
// them alone.
enum PS_BeamMode
{
    PS_BEAMMODE_NONE   = 0,     // never draw a beam
    PS_BEAMMODE_AUTO   = 1,     // beam for spot lights, nothing for point lights
    PS_BEAMMODE_ALWAYS = 2      // beam even for a point light, along direction[]
};

// PS_LightDesc::flags - bit field.
enum PS_LightFlags
{
    PS_LIGHTFLAG_NONE = 0,

    // Keep the light registered but stop rendering it. Cheaper than
    // destroy/create for something that blinks or is toggled by the player.
    PS_LIGHTFLAG_DISABLED = 1u << 0,

    // Skip the deferred surface pass: the light stops illuminating geometry
    // but still feeds fog and beams. For a light meant to be seen as a glow in
    // the air rather than as something lighting the street.
    PS_LIGHTFLAG_NO_SURFACE_LIGHTING = 1u << 1,

    // Ignore the per-area light tweaks the map config applies (those dim or
    // brighten lights in specific parts of the map). Set it for lights that
    // must keep exactly the intensity you asked for - a scripted cutscene
    // light, a flashlight the player carries.
    PS_LIGHTFLAG_IGNORE_AREA_TWEAKS = 1u << 2
};

// The light description. Always initialise with PS_LightDescInit() - it is the
// only thing that sets structSize, and it fills in defaults that render as a
// plausible white lamp, so you only assign what you actually care about.
typedef struct PS_LightDesc
{
    // Set by PS_LightDescInit(). Do not touch.
    unsigned int structSize;

    int          type;              // PS_LightType
    unsigned int flags;             // PS_LightFlags

    // GTA world space, in game units (roughly metres).
    float        position[3];

    // Spot direction, pointing AWAY from the light. Normalised internally, so
    // any non-zero length is fine. Ignored for point lights.
    float        direction[3];

    // Distance at which the light has fallen off to nothing. This is the
    // whole extent of the light, not a "useful" range: cost grows with the
    // screen area it covers, so prefer a small radius with a higher intensity
    // over a huge dim sphere.
    float        radius;

    // Linear RGB, each clamped to 0..1. To go brighter than white, raise
    // intensity rather than pushing colour components past 1.
    float        color[3];

    // Multiplier on top of the global light intensity the user configured.
    // 1.0 is "as bright as a normal game light of this radius".
    float        intensity;

    // Outer half-angle of the cone, in degrees, 1..89. The soft edge of the
    // cone is added inside this angle. Spot lights only.
    float        spotAngle;

    int          fogMode;           // PS_FogMode
    float        fogIntensity;      // multiplier for the fog glow alone

    int          beamMode;          // PS_BeamMode
    float        beamLength;        // 0 = use the configured default
    float        beamIntensity;     // multiplier for the beam alone

    // Self-destruct timer, in gameplay milliseconds (it does not run while the
    // game is paused). 0 = the light lives until you destroy it. Use it for
    // one-shot lights - a muzzle flash, a spark, an explosion - so a missed
    // cleanup path cannot leak a light that stays lit forever.
    unsigned int lifetimeMs;

    // Tail of lifetimeMs spent fading to black, so a timed light goes out
    // smoothly instead of popping. Clamped to lifetimeMs. Ignored when
    // lifetimeMs is 0.
    unsigned int fadeOutMs;
} PS_LightDesc;

//-----------------------------------------------------------------------------
// Entry points
//
// The signatures below are what ProperShaders.asi exports, by these exact
// names, as undecorated __cdecl. Use PS_Api / PS_ApiLoad() rather than
// declaring them yourself.
//-----------------------------------------------------------------------------

// Returns PS_API_VERSION of the INSTALLED build. Always present, and always
// the first thing to call.
typedef int (__cdecl *PS_GetApiVersion_t)(void);

// Registers a light and returns its handle, or PS_INVALID_LIGHT if desc is
// invalid or the registry is full (see PS_LightGetCapacity).
typedef PS_LightHandle (__cdecl *PS_LightCreate_t)(const PS_LightDesc* desc);

// Replaces every property of an existing light, lifetime included: the
// countdown restarts. Returns 1 on success, 0 for a stale handle or an invalid
// desc. For the common per-frame case, prefer the setters below.
typedef int (__cdecl *PS_LightUpdate_t)(PS_LightHandle light, const PS_LightDesc* desc);

// Fills desc with the current properties of the light. Call PS_LightDescInit()
// on desc first: structSize tells ProperShaders how much of it to write, so
// the call stays correct against a newer build with a bigger struct.
typedef int (__cdecl *PS_LightGet_t)(PS_LightHandle light, PS_LightDesc* desc);

// Cheap single-property setters, safe to call every frame. Each returns 1 on
// success, 0 for a stale handle.
typedef int (__cdecl *PS_LightSetPosition_t)(PS_LightHandle light, float x, float y, float z);
typedef int (__cdecl *PS_LightSetDirection_t)(PS_LightHandle light, float x, float y, float z);
typedef int (__cdecl *PS_LightSetColor_t)(PS_LightHandle light, float r, float g, float b, float intensity);
typedef int (__cdecl *PS_LightSetRadius_t)(PS_LightHandle light, float radius);
typedef int (__cdecl *PS_LightSetEnabled_t)(PS_LightHandle light, int enabled);

// Unregisters the light. Returns 1 if it existed. Destroying an already
// destroyed or never valid handle is harmless and returns 0.
typedef int (__cdecl *PS_LightDestroy_t)(PS_LightHandle light);

// 1 while the handle refers to a live light.
typedef int (__cdecl *PS_LightExists_t)(PS_LightHandle light);

// Diagnostics: how many API lights are registered right now, and how many the
// registry can hold in total (shared by every mod on the system).
typedef int (__cdecl *PS_LightGetCount_t)(void);
typedef int (__cdecl *PS_LightGetCapacity_t)(void);

//-----------------------------------------------------------------------------
// State queries
//
// Read-only, and cheap enough to poll every frame - each one is a single read
// of a value the renderer already maintains. That also means they only mean
// anything on the game thread, while a frame is being built.
//
// Deliberately flat: one function per fact, no structs and no enums, so you
// resolve the handful you need and a missing one costs you nothing but a null
// pointer. All booleans are int: 1 for true, 0 for false.
//-----------------------------------------------------------------------------

// ---- Version and lifecycle ----

// The version of Proper Shaders itself, as major*10000 + minor*100 + patch,
// so 1.2.3 reads as 10203. Compare with <. Unrelated to PS_GetApiVersion(),
// which versions the API rather than the mod.
typedef int (__cdecl *PS_GetVersion_t)(void);

// 1 once Proper Shaders has finished its first-frame initialisation. Until
// then the render state and the buffers below are not meaningful yet.
typedef int (__cdecl *PS_IsInitialized_t)(void);

// Re-reads the ini files, the same thing the PSRLD cheat does.
//
// EXPERIMENTAL - reloading is not fully supported yet: it re-runs feature
// configuration mid-frame and not every feature rebuilds its device resources
// cleanly from there. Fine for tweaking during development, not something to
// call in a shipped mod.
typedef void (__cdecl *PS_ReloadSettings_t)(void);

// ---- Which features the user has enabled ----
//
// These reflect the user's configuration, so they are fixed for the run (until
// a settings reload) and are what to branch on when deciding whether an effect
// of your own is worth doing at all.

typedef int (__cdecl *PS_IsUsingDeferredRenderer_t)(void);
typedef int (__cdecl *PS_IsUsingDeferredLights_t)(void);
typedef int (__cdecl *PS_IsUsingGlobalReflections_t)(void);
typedef int (__cdecl *PS_IsUsingHDR_t)(void);
typedef int (__cdecl *PS_IsUsingReversedDepthBuffer_t)(void);

// Directional (sun/moon) shadows specifically.
typedef int (__cdecl *PS_IsUsingGlobalShadows_t)(void);

// True if ANY shadow system is on: global shadows, skylight, or headlights.
typedef int (__cdecl *PS_IsUsingAnyShadows_t)(void);

// Which pipeline replaced the game's own, per object class. 0 always means the
// stock unlit PC pipeline, i.e. Proper Shaders is not shading that class:
//   World:    0 = PC unlit, 1 = PS2 PBR
//   Vehicles: 0 = PC unlit, 1 = PS2 lit, 2 = Modern PBR
//   Skins:    0 = PC unlit, 1 = PC lit
typedef int (__cdecl *PS_GetWorldPipeline_t)(void);
typedef int (__cdecl *PS_GetVehiclePipeline_t)(void);
typedef int (__cdecl *PS_GetSkinPipeline_t)(void);

// ---- What is rendering right now ----
//
// These change many times per frame. Read them from inside a render hook, not
// from game logic.

typedef int (__cdecl *PS_IsDeferredPass_t)(void);
typedef int (__cdecl *PS_IsForwardPass_t)(void);

// 1 only for the one render call per frame that is the real scene pass.
//
// Proper Shaders renders the world several times a frame - shadow maps,
// reflection captures, and so on - and a render hook of yours fires for every
// one of them. Gating on this is how you run per-frame work exactly once, in
// the pass the player actually sees.
typedef int (__cdecl *PS_IsMostCommonRenderCall_t)(void);

typedef int (__cdecl *PS_IsRenderingScene_t)(void);
typedef int (__cdecl *PS_IsRenderingPostFX_t)(void);

// The re-render passes, each 1 only while that capture is being built.
typedef int (__cdecl *PS_IsCapturingGlobalShadows_t)(void);   // sun/moon shadow map
typedef int (__cdecl *PS_IsCapturingLocalShadows_t)(void);    // headlight shadow map
typedef int (__cdecl *PS_IsCapturingSkylight_t)(void);
typedef int (__cdecl *PS_IsCapturingReflections_t)(void);

typedef int (__cdecl *PS_IsCapturingDepth_t)(void);
typedef int (__cdecl *PS_IsDepthCapturedThisFrame_t)(void);

// ---- Lighting ----

// How much the directional shadows are actually shaping the scene this frame,
// 0..1. Zero whenever they cannot contribute at all - switched off, suppressed
// for the frame, or indoors - and otherwise the night fade scaled by the
// current sun strength.
//
// A different question from PS_IsUsingGlobalShadows(): that says the feature
// is on, this says whether it is doing anything at this moment.
typedef float (__cdecl *PS_GetGlobalShadowsInfluence_t)(void);

// ---- Render targets ----
//
// Every one of these is a BORROWED pointer, valid only for the call that asked
// for it:
//   * Re-fetch it every time. They are recreated on a device reset and on any
//     resolution change.
//   * Always null check. A buffer is null when the feature that owns it is off
//     or has not run yet this frame.
//   * Never Release() one, and never AddRef(). You do not own it.
// Reading one outside a render hook is meaningless; writing to one is not
// supported.

typedef PS_Texture (__cdecl *PS_GetDepthBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetNormalsLayerBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetMotionVectorBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetShadowsMask_t)(void);

// The deferred G-buffer. All null unless the deferred renderer is on.
typedef PS_Texture (__cdecl *PS_GetDeferredAlbedoBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetDeferredMaterialsBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetDeferredEmissiveBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetDeferredLightingBuffer_t)(void);
typedef PS_Texture (__cdecl *PS_GetDeferredLightsBuffer_t)(void);

//-----------------------------------------------------------------------------
// Loader
//-----------------------------------------------------------------------------

// Every entry point in one struct. A member is null when the installed
// ProperShaders is older than the version that introduced it, so a mod
// supporting several versions can test the pointer instead of the version
// number.
typedef struct PS_Api
{
    // 0 when ProperShaders is not installed.
    int                     version;

    PS_GetApiVersion_t      GetApiVersion;

    PS_LightCreate_t        LightCreate;
    PS_LightUpdate_t        LightUpdate;
    PS_LightGet_t           LightGet;
    PS_LightSetPosition_t   LightSetPosition;
    PS_LightSetDirection_t  LightSetDirection;
    PS_LightSetColor_t      LightSetColor;
    PS_LightSetRadius_t     LightSetRadius;
    PS_LightSetEnabled_t    LightSetEnabled;
    PS_LightDestroy_t       LightDestroy;
    PS_LightExists_t        LightExists;
    PS_LightGetCount_t      LightGetCount;
    PS_LightGetCapacity_t   LightGetCapacity;

    // State queries (API version 2 and up)
    PS_GetVersion_t                 GetVersion;
    PS_IsInitialized_t              IsInitialized;
    PS_ReloadSettings_t             ReloadSettings;

    PS_IsUsingDeferredRenderer_t    IsUsingDeferredRenderer;
    PS_IsUsingDeferredLights_t      IsUsingDeferredLights;
    PS_IsUsingGlobalReflections_t   IsUsingGlobalReflections;
    PS_IsUsingHDR_t                 IsUsingHDR;
    PS_IsUsingReversedDepthBuffer_t IsUsingReversedDepthBuffer;
    PS_IsUsingGlobalShadows_t       IsUsingGlobalShadows;
    PS_IsUsingAnyShadows_t          IsUsingAnyShadows;
    PS_GetWorldPipeline_t           GetWorldPipeline;
    PS_GetVehiclePipeline_t         GetVehiclePipeline;
    PS_GetSkinPipeline_t            GetSkinPipeline;

    PS_IsDeferredPass_t             IsDeferredPass;
    PS_IsForwardPass_t              IsForwardPass;
    PS_IsMostCommonRenderCall_t     IsMostCommonRenderCall;
    PS_IsRenderingScene_t           IsRenderingScene;
    PS_IsRenderingPostFX_t          IsRenderingPostFX;
    PS_IsCapturingGlobalShadows_t   IsCapturingGlobalShadows;
    PS_IsCapturingLocalShadows_t    IsCapturingLocalShadows;
    PS_IsCapturingSkylight_t        IsCapturingSkylight;
    PS_IsCapturingReflections_t     IsCapturingReflections;
    PS_IsCapturingDepth_t           IsCapturingDepth;
    PS_IsDepthCapturedThisFrame_t   IsDepthCapturedThisFrame;

    PS_GetGlobalShadowsInfluence_t  GetGlobalShadowsInfluence;

    PS_GetDepthBuffer_t             GetDepthBuffer;
    PS_GetNormalsLayerBuffer_t      GetNormalsLayerBuffer;
    PS_GetMotionVectorBuffer_t      GetMotionVectorBuffer;
    PS_GetShadowsMask_t             GetShadowsMask;
    PS_GetDeferredAlbedoBuffer_t    GetDeferredAlbedoBuffer;
    PS_GetDeferredMaterialsBuffer_t GetDeferredMaterialsBuffer;
    PS_GetDeferredEmissiveBuffer_t  GetDeferredEmissiveBuffer;
    PS_GetDeferredLightingBuffer_t  GetDeferredLightingBuffer;
    PS_GetDeferredLightsBuffer_t    GetDeferredLightsBuffer;
} PS_Api;

// Fills desc with defaults: a white omnidirectional lamp, 10 units across,
// lighting surfaces only.
static __inline void PS_LightDescInit(PS_LightDesc* desc)
{
    unsigned char* p = (unsigned char*)desc;
    unsigned int   i;
    for (i = 0; i < (unsigned int)sizeof(PS_LightDesc); ++i) p[i] = 0;

    desc->structSize    = (unsigned int)sizeof(PS_LightDesc);
    desc->type          = PS_LIGHT_POINT;
    desc->flags         = PS_LIGHTFLAG_NONE;
    desc->radius        = 10.0f;
    desc->color[0]      = 1.0f;
    desc->color[1]      = 1.0f;
    desc->color[2]      = 1.0f;
    desc->intensity     = 1.0f;
    desc->direction[2]  = -1.0f;    // straight down, the useful default for a spot
    desc->spotAngle     = 45.0f;
    desc->fogMode       = PS_FOGMODE_NONE;
    desc->fogIntensity  = 1.0f;
    desc->beamMode      = PS_BEAMMODE_NONE;
    desc->beamLength    = 0.0f;
    desc->beamIntensity = 1.0f;
    desc->lifetimeMs    = 0;
    desc->fadeOutMs     = 0;
}

#if defined(_WIN32) || defined(_WIN64)

// Resolves every entry point from an already loaded ProperShaders.asi.
// Returns 1 when ProperShaders is present, 0 when it is not - in which case
// api is zeroed, so `if (api.LightCreate)` is a valid guard everywhere.
//
// Call it AFTER ProperShaders has loaded. From a plugin-sdk mod that means an
// initialisation event rather than DllMain, which runs while the loader is
// still bringing the other .asi files up.
static __inline int PS_ApiLoad(PS_Api* api)
{
    HMODULE module;
    unsigned char* p = (unsigned char*)api;
    unsigned int   i;
    for (i = 0; i < (unsigned int)sizeof(PS_Api); ++i) p[i] = 0;

    module = GetModuleHandleA("ProperShaders.asi");
    if (!module) return 0;

    api->GetApiVersion = (PS_GetApiVersion_t)GetProcAddress(module, "PS_GetApiVersion");
    if (!api->GetApiVersion) return 0;

    api->version = api->GetApiVersion();

    api->LightCreate       = (PS_LightCreate_t)      GetProcAddress(module, "PS_LightCreate");
    api->LightUpdate       = (PS_LightUpdate_t)      GetProcAddress(module, "PS_LightUpdate");
    api->LightGet          = (PS_LightGet_t)         GetProcAddress(module, "PS_LightGet");
    api->LightSetPosition  = (PS_LightSetPosition_t) GetProcAddress(module, "PS_LightSetPosition");
    api->LightSetDirection = (PS_LightSetDirection_t)GetProcAddress(module, "PS_LightSetDirection");
    api->LightSetColor     = (PS_LightSetColor_t)    GetProcAddress(module, "PS_LightSetColor");
    api->LightSetRadius    = (PS_LightSetRadius_t)   GetProcAddress(module, "PS_LightSetRadius");
    api->LightSetEnabled   = (PS_LightSetEnabled_t)  GetProcAddress(module, "PS_LightSetEnabled");
    api->LightDestroy      = (PS_LightDestroy_t)     GetProcAddress(module, "PS_LightDestroy");
    api->LightExists       = (PS_LightExists_t)      GetProcAddress(module, "PS_LightExists");
    api->LightGetCount     = (PS_LightGetCount_t)    GetProcAddress(module, "PS_LightGetCount");
    api->LightGetCapacity  = (PS_LightGetCapacity_t) GetProcAddress(module, "PS_LightGetCapacity");

    api->GetVersion                 = (PS_GetVersion_t)                 GetProcAddress(module, "PS_GetVersion");
    api->IsInitialized              = (PS_IsInitialized_t)              GetProcAddress(module, "PS_IsInitialized");
    api->ReloadSettings             = (PS_ReloadSettings_t)             GetProcAddress(module, "PS_ReloadSettings");

    api->IsUsingDeferredRenderer    = (PS_IsUsingDeferredRenderer_t)    GetProcAddress(module, "PS_IsUsingDeferredRenderer");
    api->IsUsingDeferredLights      = (PS_IsUsingDeferredLights_t)      GetProcAddress(module, "PS_IsUsingDeferredLights");
    api->IsUsingGlobalReflections   = (PS_IsUsingGlobalReflections_t)   GetProcAddress(module, "PS_IsUsingGlobalReflections");
    api->IsUsingHDR                 = (PS_IsUsingHDR_t)                 GetProcAddress(module, "PS_IsUsingHDR");
    api->IsUsingReversedDepthBuffer = (PS_IsUsingReversedDepthBuffer_t) GetProcAddress(module, "PS_IsUsingReversedDepthBuffer");
    api->IsUsingGlobalShadows       = (PS_IsUsingGlobalShadows_t)       GetProcAddress(module, "PS_IsUsingGlobalShadows");
    api->IsUsingAnyShadows          = (PS_IsUsingAnyShadows_t)          GetProcAddress(module, "PS_IsUsingAnyShadows");
    api->GetWorldPipeline           = (PS_GetWorldPipeline_t)           GetProcAddress(module, "PS_GetWorldPipeline");
    api->GetVehiclePipeline         = (PS_GetVehiclePipeline_t)         GetProcAddress(module, "PS_GetVehiclePipeline");
    api->GetSkinPipeline            = (PS_GetSkinPipeline_t)            GetProcAddress(module, "PS_GetSkinPipeline");

    api->IsDeferredPass             = (PS_IsDeferredPass_t)             GetProcAddress(module, "PS_IsDeferredPass");
    api->IsForwardPass              = (PS_IsForwardPass_t)              GetProcAddress(module, "PS_IsForwardPass");
    api->IsMostCommonRenderCall     = (PS_IsMostCommonRenderCall_t)     GetProcAddress(module, "PS_IsMostCommonRenderCall");
    api->IsRenderingScene           = (PS_IsRenderingScene_t)           GetProcAddress(module, "PS_IsRenderingScene");
    api->IsRenderingPostFX          = (PS_IsRenderingPostFX_t)          GetProcAddress(module, "PS_IsRenderingPostFX");
    api->IsCapturingGlobalShadows   = (PS_IsCapturingGlobalShadows_t)   GetProcAddress(module, "PS_IsCapturingGlobalShadows");
    api->IsCapturingLocalShadows    = (PS_IsCapturingLocalShadows_t)    GetProcAddress(module, "PS_IsCapturingLocalShadows");
    api->IsCapturingSkylight        = (PS_IsCapturingSkylight_t)        GetProcAddress(module, "PS_IsCapturingSkylight");
    api->IsCapturingReflections     = (PS_IsCapturingReflections_t)     GetProcAddress(module, "PS_IsCapturingReflections");
    api->IsCapturingDepth           = (PS_IsCapturingDepth_t)           GetProcAddress(module, "PS_IsCapturingDepth");
    api->IsDepthCapturedThisFrame   = (PS_IsDepthCapturedThisFrame_t)   GetProcAddress(module, "PS_IsDepthCapturedThisFrame");

    api->GetGlobalShadowsInfluence  = (PS_GetGlobalShadowsInfluence_t)  GetProcAddress(module, "PS_GetGlobalShadowsInfluence");

    api->GetDepthBuffer             = (PS_GetDepthBuffer_t)             GetProcAddress(module, "PS_GetDepthBuffer");
    api->GetNormalsLayerBuffer      = (PS_GetNormalsLayerBuffer_t)      GetProcAddress(module, "PS_GetNormalsLayerBuffer");
    api->GetMotionVectorBuffer      = (PS_GetMotionVectorBuffer_t)      GetProcAddress(module, "PS_GetMotionVectorBuffer");
    api->GetShadowsMask             = (PS_GetShadowsMask_t)             GetProcAddress(module, "PS_GetShadowsMask");
    api->GetDeferredAlbedoBuffer    = (PS_GetDeferredAlbedoBuffer_t)    GetProcAddress(module, "PS_GetDeferredAlbedoBuffer");
    api->GetDeferredMaterialsBuffer = (PS_GetDeferredMaterialsBuffer_t) GetProcAddress(module, "PS_GetDeferredMaterialsBuffer");
    api->GetDeferredEmissiveBuffer  = (PS_GetDeferredEmissiveBuffer_t)  GetProcAddress(module, "PS_GetDeferredEmissiveBuffer");
    api->GetDeferredLightingBuffer  = (PS_GetDeferredLightingBuffer_t)  GetProcAddress(module, "PS_GetDeferredLightingBuffer");
    api->GetDeferredLightsBuffer    = (PS_GetDeferredLightsBuffer_t)    GetProcAddress(module, "PS_GetDeferredLightsBuffer");

    return 1;
}

#endif // _WIN32

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PROPERSHADERS_API_H
