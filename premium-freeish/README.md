A hook that emulates "premium free" mode for CHUNITHM.

Hold ESC at the end of the track (before the area animations end) to end the credit.

Originally a hex edit by [the Polish cartel](https://klikwzium.click/chusanplas.html).

## Configuration
Add a new section to `segatools.ini`:

```ini
[pfreeish]
; End the credit with a different key. Refer to Microsoft's
; virtual key code list to know what key code to use.
; https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
end_credit=0x1B  ; VK_ESCAPE

; Set the maximum number of tracks for a credit. This is useful
; if you still wish to have normal credits, but want to take advantage
; of exiting the credit early.
; max_tracks=
```

## Compatibility
- SDBT: CHUNITHM AIR+ - CHUNITHM PARADISE (LOST)
- SDHD: NEW!! - LUMINOUS+
- SDGS (International):
  - SUPERSTAR
  - NEW!!
  - NEW PLUS!!
  - LUMINOUS

Unlisted versions don't have pre-defined offsets and rely on
signature scanning which may be inaccurate.

## Building
This was built with MSVC. GCC is strongly discouraged because
`libg++` is absolutely massive and static linking that
will make the DLL 10MB.

```batchfile
mkdir build
cd build
cmake .. ^
  -G Ninja ^
  -DCMAKE_BUILD_TYPE=<Release|Debug> ^
  -DSTATIC_MSVC_RUNTIME=ON
ninja
```

## Adding version support
This usually isn't necessary because the hook also
supports signature scanning; however patterns might
change between versions.

Create a new file in the [`versions/`](versions) folder
with all the variables filled in this format:

```cmake
set(GAME_ID                  SDHD)  # or SDGS/SDBT
set(GAME_VERSION             _VERSION_HERE)

set(TIME_DATE_STAMP          0x00000000)
set(ADDRESS_OF_ENTRY_POINT   0x00000000)

set(OFFSET_HOOK_TRACK_COUNT  0x00000000)
set(OFFSET_HOOK_END_OF_TRACK 0x00000000)
```

- `TIME_DATE_STAMP` and `ADDRESS_OF_ENTRY_POINT` can be
retrieved by checking in your preferred PE parser.
- `OFFSET_HOOK_TRACK_COUNT` is the offset to the function
returning the default track count (usually 3). You can usually
check a patcher to find this (see the "Maximum tracks" patch),
but if you don't, an easy way to find it is finding 
the function returning the *credit* track count. This can 
be reliably found by scanning for the code which limits
the maximum number of tracks to 7:
`B8 07 00 00 00 3B F0 0F 47 F0` (on older versions the
limit might be 4 instead, so `07 00 00 00` becomes
`04 00 00 00`)
- `OFFSET_HOOK_END_OF_TRACK` is the offset to the code
**calling** the function that checks if the player has
finished all their tracks.

You should refer to existing definitions to get a grasp
for what any of this means. Both offsets should be RVAs
(in Ghidra, this is called "Imagebase Offset").

## Credits
- Keeboy (tested CRYSTAL+/PARADISE on an older version
of this hook)
