OBJS = 	RaceEnd.o Game.o RaceM.o AI.o Init.o TimeAttack.o Single.o Font.o \
		LevelEnd.o CastleMEnd.o LevelEd.o SinglePlSel.o SinglePlCSel.o Options.o \
		Highscore.o Structs.o Ghost.o Menu.o Geometrics.o MapSel.o Utils.o Sound.o \
		Console.o ModeSelM.o CastleM.o



LIBS = 	-L/usr/lib -lSDL -lSDL_mixer -lSDL_image -lGLU -lGL


OrbitHopper.x: $(OBJS)
	g++ -o OrbitHopper.x $(OBJS) $(LIBS)  -O3 -s

%.o: source/%.cpp headers/%.h
	g++ -c -o $@ $< -I/usr/include/SDL -I/usr/include/GL -O3 -s
