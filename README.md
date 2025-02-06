# Rocket fuel
Welcome to my game that I made in C.

This game is right now just a space simulator but I have plans for more.
The special thing with this game is that it uses unicode characters for rendering and that makes it usable on headless computers.
I have not made it work with ssh but I'm working on it.



# Video

https://github.com/user-attachments/assets/1d04017c-1c74-4b0d-81e6-3d6ae244d1ab

Sorry for the low bitrate. :3

# install
to install the game you will need a linux distro and gcc or any other compiler.
1. clone the repo. You can use `git clone https://github.com/komplex-polygon/rocket_fuel.git` or download as Zip.
2. open folder / extract zip.
3. run gcc or compiler. depending on what distro you use this part can change. For me it worked with `gcc -o out rock.c -lm -O2` but change it if you need. keep the `-lm` it is importing math.h
4. run the program. `sudo ./out` the game must be executed as root or the program will fail. When you run the program you will be prompted for what keyboard to use, type the number with the corresponding name of your keyboard. it will show multiples of every device and soon will not work so try all of them first to see what id the real device has.
when you have selected your keyboard and press enter "do not touch to keyboard" the program will wait 2s before connecting to keyboard and under that time the keyboard should not have any keys pressed.
if a key is pressed and the game starts the key will be stuck until you stop the program and press the key agen. The reason for this is that the os gets the key down signal but never the up signal and that makes it stuck. you can fix a stuck key by resizing your terminal or stopping and starting it again.
you stop the game by pressing `esc` on your keyboard.

# updates
1. The camera can rotate now.
2. Added planets with gravity that affects the players and other planets.
3. Rewrote the 2d sprite rendering function to be more stable with large numbers.
   
# why sudo?
When I made the game I chose to use the usb controller for input and it has some benefits like low latency and a lot more reliability plus it disables the keyboard typing and that is good news for my unicode renderer that gets the screen all for itself.
There are ways to get input without sudo but they are slow and not as reliable. One of them uses the character buffer for input but it has some big flaws.
1. it does not like hold events
2. it displays text on the scene and makes the renderer stutter.

# Problems with ssh
because the game uses the usb controller for the input ssh will not work in the current moment.
I chose the usb controller because it gave me the least delay and most reliability.




# To do
1. make the game more fun.
2. optimize the rendering and the simulation.
3. make the game easier to play and understand.






# About
This game was made by Mikael.back... and some parts ai (deepseek).
Ai did the low level stuff such as reading usb controller data and ANSI codes.
If you don't trust deepseeker you can read the code in the rock.c and judge by yourself if you want to run it.

Ps: This is the first time I program in C so don't judge me too much. ;3















