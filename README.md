# ForerunnerVR

> [!CAUTION]
> Very early work in progress. This mod is not ready for full playthroughs yet and is missing many basic features.

Forerunner VR is a VR mod for Halo: The Master Chief Collection. Due to the nature of the game (i.e. it really being half a dozen different games in an unreal-shaped trenchcoat) the plan is to tackle each game one at a time in different modules (each named after a prominent forerunner artefact from that game), reusing code and techniques where possible.

## Modules
### 🟡Forerunner
The Launcher + title screen/"wrapper" elements of MCC, relevant to all games. This will likely be continually worked on in conjunction with any other modules.
### 🔵Alpha 
Halo Combat Evolved. This will likely be tackled last, as it is already fully playable via [HaloCEVR](https://github.com/LivingFray/HaloCEVR)
### 🟠Delta 
Halo 2. The module currently being work on
### 🔴Ark
Halo 3.
### 🔴Reach
Halo Reach (better module name needed, very little named forerunner stuff in that game)
### 🔴Excession
Halo 3: ODST. (Had to look up the name of the forerunner portal to get a module name for this)
### 🔴Requiem 
Halo 4. As the only non-bungie entry in MCC this is mainly here for completeness and is the lowest priority

## Launching
The mod can be launched from Forerunner.exe in two ways:
1. Click "Launch Game": if you have the game installed on steam it should automatically launch the game without anti-cheats and inject itself once it detects the game is running
2. Launch the game manually (with anti-cheat disabled) and click "Inject" once the button is available
You can configure the launcher in the Settings tab to automatically launch/inject/close itself to streamline this process

## Contributing
Contributions are welcome, and likely the only way this project will get completed in a reasonable amount of time. 
### Building
This project is built using cmake and is split into two projects: the launcher ("Forerunner", source is in `/src/launcher/`) and the dll containing the mod ("ForerunnerPayload" source is in `/src/payload`), anything in `/src/common/` is shared by both projects. There are 3 build configs provided by default, ensure any changes work in all 3 before making a pull request:
- Debug
- Debug Emu (replaces the VR backend with an emulated version which displays in a separate window, useful for debugging without having to put on a headset every 5 minutes)
- Release
  
If you added a file to the project and its not being seen by the compiler you may need to refresh the CMakeLists.txt files as they GLOB for source files and can get cached with an outdated list.
### Style
References to halo game code (patches, variables, struct layouts, etc) should be placed in a corresponding file in the blam directory in the module's folder, e.g. the definitions for vectors for halo 2 can be found in `/payload/blam/math/real_math.h`. Ideally symbol names, files and folders should use the actual names from the original source code, as best as we can infer it from available sources.
Halo code should be written in `snake_case` as that appears to be the style used by the original developers. Mod code should be written in `UpperCamelCase` to make it clear at a glance which code is ours and not just referencing/reimplementing something from the game.
