# dodhooks
SourceMod extension with Detours &amp; Natives for Day of Defeat: Source

Original written and maintained by Joakim Andersson "Andersso".

## Compiling ##

Compiling this version of DoDHooks requires Python with the [AMBuild](https://github.com/alliedmodders/ambuild) module available.

On Windows, this should be done at a VS command prompt or after having manually run vsvars32.bat

#### Configure ####
```
mkdir build
cd build
python ../configure.py --hl2sdk-root=<path to dir containing hl2sdk-dods dir> --sm-path=<path to clone of sourcemod repo> --mms-path=<path to clone of metamod-source repo> --enable-optimize
```

#### Compile ####
```
cd build
ambuild
```

(output will be in build/package/ directory)

#### Creating a VS project file (Windows) ####
```
mkdir build
cd build
python ../configure.py --hl2sdk-root=<path to dir containing hl2sdk-dods dir> --sm-path=<path to clone of sourcemod repo> --mms-path=<path to clone of metamod-source repo> --enable-optimize --gen=vs
```
