.include "hdr.asm"

.section ".rodata" superfree
logo_map:
.incbin "data/logo.map"
logo_palette:
.incbin "data/logo.clr"
.ends

.section ".rodata1" superfree
logo_tiles:
.incbin "data/logo.dat"
.ends

.section ".rodata2" superfree
level_tiles:
.incbin "data/level1.dat"
level1_palette:
.incbin "data/level1.clr"
level1_map_csv:
.incbin "data/level1_map.csv"
level1_collision_csv:
.incbin "data/level1_collision.csv"
level2_palette:
.incbin "data/level2.clr"
level2_map_csv:
.incbin "data/level2_map.csv"
level2_collision_csv:
.incbin "data/level2_collision.csv"
level3_palette:
.incbin "data/level3.clr"
level3_map_csv:
.incbin "data/level3_map.csv"
level3_collision_csv:
.incbin "data/level3_collision.csv"
.ends

.section ".rodata3" superfree
level4_palette:
.incbin "data/level4.clr"
level4_map_csv:
.incbin "data/level4_map.csv"
level4_collision_csv:
.incbin "data/level4_collision.csv"
level5_palette:
.incbin "data/level5.clr"
level5_map_csv:
.incbin "data/level5_map.csv"
level5_collision_csv:
.incbin "data/level5_collision.csv"
level6_palette:
.incbin "data/level6.clr"
level6_map_csv:
.incbin "data/level6_map.csv"
level6_collision_csv:
.incbin "data/level6_collision.csv"
level7_palette:
.incbin "data/level7.clr"
level7_map_csv:
.incbin "data/level7_map.csv"
level7_collision_csv:
.incbin "data/level7_collision.csv"
level8_palette:
.incbin "data/level8.clr"
level8_map_csv:
.incbin "data/level8_map.csv"
level8_collision_csv:
.incbin "data/level8_collision.csv"
.ends