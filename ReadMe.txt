Orbit Hopper - Version 1.16c			released 07/19/2009
Windows & Linux Version



--------------------------------------------------------------------------

Orbit-Hopper and work related to it may be distributed under the
terms of the GNU GPL. See "license.txt" for details.


--------------------------------------------------------------------------

This package contains three pre-compiled binaries:

OrbitHopper.exe		Windows XP 32 bit
OrbitHopper32.x		Ubuntu Linux 9.04 32-bit
OrbitHopper64.x		Ubuntu Linux 9.04 64-bit


--------------------------------------------------------------------------

		
								!! NOTE !!

The precompiled form of this game needs at least SDL 1.2.9, SDL_mixer 1.2.6 and 
SDL_image 1.2.4 runtime libraries.
Free binaries for Linux, Windows etc. are available at www.libsdl.org :
		
SDL 1.2.x runtime-libraries: 	http://www.libsdl.org/download-1.2.php                             
SDL_mixer binaries:				http://www.libsdl.org/projects/SDL_mixer/                          
SDL_image binaries:				http://www.libsdl.org/projects/SDL_image/		

Place according binaries (e.g: SDL.dll) in your game- or system-folder.
 

Make sure you have the latest OpenGL/Mesa-compatible drivers.

If you experience trouble starting the game, try to change
the sound or screen-resolution settings in the "settings.ini" file.



--------------------------------------------------------------------------

Performance hints:

Reducing "Draw-Dist", "Shadow-Dist", and screen resolution in the settings.ini file
increases performance significally.



--------------------------------------------------------------------------





Default Controls:


In-Game:
	key: 			action:
	[^/~/TAB]		toggle console
	[RETURN]			confirm console command
	[esc] 			back to menu	


Menu:
	key:				action:
	[a/left]			scroll left
	[d/right]		scroll right
	[return]			select
	[esc]				quit game



LevelEditor:
	key:						action:
	[w/up]						scroll up
	[s/down]						scroll down
	[a/left]						scroll left
	[d/right]					scroll right
	[m]							increase block height
	[n]							decrease block height
	[up/down]					scroll file list 
	[del]							delete selected/last floor
	[CTRL]						hold and click on floor to create trigger-floor
	[Mouse]						click to add/select cubes, drag to move/resize
	[esc] 						back to menu
	[MouseWheel]				zoom in/out
	[Middle-Mouse-Button]	reset view

	Read Leveleditor.pdf for detailed instructions.
	

SinglePlayer:
	key:				action:
	[s]				skip selected level, if allowed



Console commands:

	"wireframe [on/off]"	
	"debug [on/off]"
	"glow [on/off]"
	"details [on/off]"
	"texset [0-?]"
	"speed [on/off]"
	"animspeed [0-50]"
	"shadows [on/off]"




--------------------------------------------------------------------------------




Creating custom texturesets:


	If you want to create a custom textureset, copy one of the existing
	directories (such as "comic", "classic" etc.) and rename it - you
	can now edit all textures. 
	Do not change their names and make sure they have dimensions of the power of two.
	Format of the textures needs to be RGBA-PNG.

	If everything was done right, you will be able to select your 
	textureset in the options menu.





Creating animated textures:


	You can animate all of the following textures 
	(located in the folder of a textureset):

		checkp.png
		wall.png
		tex0.png
		.
		.
		.
		tex12.png

	Note: Do NOT animate textures, that are not listed above !


	Animating textures is quite simple:

		1)	Create a set of *.png files
			(textures must have dimensions of the power of two 
			(e.g: 128x128 or 128x256 or 512x256 ...))


		2)	Name the *.png files:

			"[Original texture name][0-9][0-9].png"

			An animated set of textures for wall.png with 3 frames would be:
			wall00.png
			wall01.png
			wall02.png


		3)	Delete or rename the original *.png file

	



----------------------------------------------------------------------------------------




Special Thanks go to:

Stefan Obermaier	(for bug reports and testing as well as mapping numerous levels)




----------------------------------------------------------------------------------------



© 2006-2009 Harald "Zneaker" Obermaier
webmaster@zneaker.com						
http://www.zneaker.com

