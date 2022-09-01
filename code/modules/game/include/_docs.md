\page game_docs Game Module

This game module builds upon the Renderer and Physics Module to implement a simple Strategy Game.
The goal is to defend the clocktower from a horde of ghosts as long as possible.

When loaded, windows will give the player guidance on the controls and objectives.

To load this module, either edit the Default.modlist to look like the following:

````yaml
- Renderer
- Physics
- Game
````

or manually load these modules via the Editor.