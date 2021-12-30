
# ThunderSim

A maze/thunder simulator written in C using SDL2.</br>
Works using the breadth-first search algorithm.</br>
Inspired by [this video](https://www.youtube.com/watch?v=akZ8JJ4gGLs).

# How-To
To build this project you will need to have the [SDL2 library](https://www.libsdl.org/download-2.0.php) installed.</br>

* If installed, simply `cd` into the project and type `make`.
* A binary named `thundersim` will be created, simply execute it with `./thundersim`.
* Tap `r` to reload a new maze/thunder.


# Parameters

You can modify the aspect of the generated maze/thunder with those 5 macros in the `.c` file:</br>
`WINDOW_WIDTH` : choose the width of the window.</br>
`WINDOW_HEIGHT` : choose the height of the window.</br>
`DIVISON` : divide the window according to the desired value.</br>
`P` : choose the probability of generating a up or down border.</br>
`Q` : choose the probability of generating a left or right border.</br>

# Exemples

* Default Parameters:

![ThunderSim_1](https://user-images.githubusercontent.com/45853802/147677088-93c80652-6fd6-4410-87f3-22ca8c6ad8f6.jpg)
  
* Bigger Grid:

![ThunderSim_2](https://user-images.githubusercontent.com/45853802/147680943-090175ba-3abe-4e54-8847-4c9e45ad26de.jpg)

* Smaller Grid:

![ThunderSim_3](https://user-images.githubusercontent.com/45853802/147681108-825067b0-725c-4477-9cbf-5d10bb0f7401.jpg)

* Stretched vertically: 

![ThunderSim_4](https://user-images.githubusercontent.com/45853802/147681562-80519f71-959a-4422-a5ad-f07cbb79547c.jpg)
  
* Stretched horizontally:

![ThunderSim_5](https://user-images.githubusercontent.com/45853802/147681557-9c4ca22d-b8f2-4578-b2ac-b672f788cccb.jpg)
