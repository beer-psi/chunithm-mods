# chunithm-mods

Various mods for the arcade rhythm game CHUNITHM.

## Usage

- Download DLLs from [releases](../../releases/latest).
- Open `start.bat` or `launch.bat` in a text editor and add `-k <mod DLL>` to the line starting with `inject_x86`:

```batchfile
inject_x86.exe -d -k chusanhook.dll -k chunithm_pfreeish.dll chusanApp.exe
REM                                 ^^^^^^^^^^^^^^^^^^^^^^^^
```
Some mods may have additional configuration, consult their READMEs for more information.

## Mods

- [Internal level sort](internal-level-sort): Sorts the song list by internal level when sorting by level is chosen.
- [Premium Freeish](premium-freeish): Play unlimited tracks in a single credit, end credit by holding an assigned key.
