# BombleMan

![bombleman banner](https://github.com/HarrisonAllen/bombleman/blob/0111c5f0df60b17afda09b639eb5dd1c61465b05/graphics/appstore/banner.png?raw=true)

Bomberman clone for the Pebble smartwatch, just a tech demo for now. I have larger ambitions to connect with Pebble Land for multiplayer but I figured that it's more valuable to put the demo out than let it rot in the void forever.

That does mean that this codebase is a huge mess! Tread with caution!

App store links:

* [Rebble Store](link)
* [Repebble Store](link)
* .pbw: `graphics/appstore/bombleman.pbw`

# Table of Contents

* [How to Play](#how-to-play)
* [Sprites](#sprites)
* [Levels](#levels)
* [CPU Logic](#cpu-logic)
* [Bugs](#bugs)
* [Multiplayer Plans](#multiplayer-plans)

# How to Play

* Destroy the CPU players with your bombs
* Break rocks with explosions to find powerups
* Once the player or all CPUs are defeated, the game will automatically restart

### Controls:
* Select to move
* Up/Down to turn
* Back to place bomb
* Hold back to quit

### Powerups

![powerup tiles](https://github.com/HarrisonAllen/bombleman/blob/75daa13d3d8ce225a3e4ac2c695e79b09465bc8f/graphics/sprites/powerups.png?raw=true)

* **Bomb** - Increases number of bombs
* **Explosion** - Increases explosion radius
* **Invincibility** - Temporarily become invincible

# Sprites

Uses the advanced version of the [Pebble GBC Graphics library](https://github.com/HarrisonAllen/pebble-gbc-graphics)

## Players

`graphics/sprites/player.aseprite`

`player-sprites.txt`

### Color

![player 1 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player.png?raw=true)

![player 2 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player2.png?raw=true)

![player 3 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player3.png?raw=true)

![player 4 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player4.png?raw=true)

### Black and White

![player 1 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player-bw.png?raw=true)

![player 2 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player2-bw.png?raw=true)

![player 3 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player3-bw.png?raw=true)

![player 4 sprites color](https://github.com/HarrisonAllen/bombleman/blob/e764f1760bf812f282cecdb5cd497e2ecec78825/graphics/sprites/player4-bw.png?raw=true)

## Tiles

`graphics/sprites/sheets/tilesheet.aseprite`

`tiles.txt`

### Color

![color tiles](https://github.com/HarrisonAllen/bombleman/blob/75daa13d3d8ce225a3e4ac2c695e79b09465bc8f/graphics/sprites/sheets/tilesheet-color.png?raw=true)

### Black and White

![bw tiles](https://github.com/HarrisonAllen/bombleman/blob/75daa13d3d8ce225a3e4ac2c695e79b09465bc8f/graphics/sprites/sheets/tilesheet-bw.png?raw=true)

# Levels

`utility.c -> load_map`

Each level is 9 x 7, not including the outer walls.

Levels are stored in text files. See [Level Examples](#level-examples) for some examples.

## Level Structure

Each level at a minimum needs a spawn point for all 4 players. Other than that anything goes!

* `1`, `2`, `3`, `4`: Spawn points for players 1-4
* `_`: blank square
* `a`: wall
* `b`: rock

## Level Examples

### `bombleman/resources/map.txt`

![level 1](https://github.com/HarrisonAllen/bombleman/blob/179ce66b0666a5ba2271e100e7b5e63c26d24e74/graphics/levels/level1.png?raw=true)

```
1_b_a_b_3
_abbbbba_
bbb_b_bbb
a_babab_a
bbb_b_bbb
_abbbbba_
4_b_a_b_2
```

### `bombleman/resources/mr-pizza-boy-map.txt`

![level 2](https://github.com/HarrisonAllen/bombleman/blob/179ce66b0666a5ba2271e100e7b5e63c26d24e74/graphics/levels/level2.png?raw=true)

```
bbb_a_bbb
_abbabba_
1_bb_bb_2
aaababaaa
3_bb_bb_4
_abbabba_
bbb_a_bbb
```

### `bombleman/resources/mr-pizza-boy-map-2.txt`

![level 3](https://github.com/HarrisonAllen/bombleman/blob/179ce66b0666a5ba2271e100e7b5e63c26d24e74/graphics/levels/level3.png?raw=true)

```
1_bb_bb_2
_aaa_aaa_
ba_bbb_ab
_abb_bba_
ba_bbb_ab
_aaa_aaa_
3_bb_bb_4
```

### `bombleman/resources/map-no-rocks.txt`

![level 4](https://github.com/HarrisonAllen/bombleman/blob/179ce66b0666a5ba2271e100e7b5e63c26d24e74/graphics/levels/level4.png?raw=true)

```
1___a___3
_a_____a_
_________
a__a_a__a
_________
_a_____a_
4___a___2
```

# CPU Logic

`game.c -> Game_control_cpus`

In general, the CPUs work like this:

1. Calculate path
2. Try to move along path if possible
3. If blocked by a player or rock, place a bomb

## CPU Personalities

Different personality types allow CPUs to behave differently from each other.

If the CPU can't find a special path, it will just target to the closest player

### Implemented

* **Hunter**: prioritizes the nearest Human (player 2 - gray)
* **Friend**: prioritizes the nearest CPU (player 3 - blue)
* **Hoarder**: prioritizes the nearest item (player 4 - red)

### Not yet implemented

* **Coward**: prioritizes the safest square
* **Lazy**: takes longer to think
* **Demolitionist**: prioritizes rocks
* **Smart**: chooses between items and players based on best path

## Pathing

`cpu_control.c -> find_path`

1. If the current square is unsafe, pick the nearest, safer square (see [Square Scores](#square-scores))
2. If safe, pick a target based on personality
3. Find the path to the target using A*
  * Each path has a score, but I'm just taking the first found for now

## Square Scores

`game.c -> Game_calc_scores`

Scores for each square are as follows:

* -1: cannot be stood on (e.g. a wall or rock)
* 0: is active death (bomb explosion)
* 20+: is safe, the higher above this the nicer
  * 21: is an empty square
  * 22: is a powerup
* 1-19: dangerous, a bomb will explode here soon
  * The lower this is, the more dangerous it is
  * Calculated by `1 + time_remaining + distance_from_bomb`

# Bugs

One known bug: when pathing fails, sometimes the game will yeet the CPU off into another dimension and the game breaks

# Multiplayer Plans

The idea was to integrate this with the larger Pebble Land ecosystem, but no idea when/if that will actually happen. Here were my plans:

![multiplayer figma design](https://github.com/HarrisonAllen/bombleman/blob/c74b94669d98a9bbfae7a963076f8c07d61c0974/graphics/figma.png?raw=true)