# London Metropolis: A Dynamic 2D Cityscape Simulation

> **An interactive 2D London cityscape simulation built using C++ and OpenGL/FreeGLUT that demonstrates fundamental computer graphics algorithms, 2D transformations, real-time animation, and dynamic environmental effects.**

---

## About

**London Metropolis: A Dynamic 2D Cityscape Simulation** is an interactive computer graphics project that recreates a simplified London urban environment using fundamental 2D graphics concepts. The scene combines buildings, roads, bridges, the River Thames, Big Ben, the London Eye, vehicles, boats, trees, clouds, and other city elements into a dynamic environment. The project demonstrates how fundamental graphics algorithms and transformations can be combined with animation and user interaction to create a visually engaging real-time city simulation.

### Key Highlights

* **Dynamic 2D Cityscape** – Represents a London-inspired urban environment with buildings, roads, bridges, river, and landmarks.
* **Animated London Eye** – Continuously rotates around its center using rotation transformation.
* **Moving Vehicles** – Vehicles move continuously along the roads with looping traffic animation.
* **Moving Boats** – Boats travel across the river to create a dynamic water environment.
* **Animated Big Ben** – Clock hands rotate to create a dynamic clock effect.
* **Day-Night Transition** – Smoothly changes the environment between daytime and nighttime.
* **Weather Effects** – Includes interactive rain, snow, and fog modes.
* **Moving Clouds** – Clouds drift across the sky with continuous animation.
* **Interactive Camera Movement** – Users can move horizontally across the scene using keyboard controls.
* **Real-Time Animation** – Timer-based updates provide continuous scene rendering and movement.

---

##  Features

### 1. 2D Scene Construction

The cityscape is constructed using fundamental graphical primitives such as lines, circles, rectangles, polygons. These primitives are combined to create complex objects including buildings, bridges, vehicles, trees, Big Ben, and the London Eye.

### 2. Graphics Algorithms

The project implements fundamental computer graphics algorithms:

**Bresenham Line Drawing Algorithm** : Used for structural and linear elements such as:

* Bridge cables
* Support bars
* London Eye spokes
* Road elements
* Big Ben clock hands

**Midpoint Circle Drawing Algorithm** : Used for circular outlines such as:

* Big Ben clock
* Clock center
* London Eye circular details

A smooth polygon-based circle technique is also used for larger filled objects such as the sun, moon, clouds, tree leaves, vehicle wheels, and lights.

---

### 3. 2D Transformations

The simulation applies several transformation techniques:

* **Translation** – Moving vehicles, clouds, boats, and the camera.
* **Rotation** – Rotating the London Eye and Big Ben clock hands.
* **Scaling** – Controlling the size and proportions of objects through parameters.

---

### 4. Real-Time Animation

The project uses a timer-based update system to continuously update object positions, rotations, and environmental states.

Animated elements include:

*  Vehicles
*  Clouds
*  Boats
*  London Eye
*  Big Ben clock
*  Rain
*  Snow
*  Fog
*  Water waves
*  Day-night transition

The scene is refreshed approximately every 16 milliseconds to maintain smooth real-time animation.

---

### 5. Environmental Effects

The project includes several dynamic environmental modes:

* **Day Mode**
* **Night Mode**
* **Rain Mode**
* **Snow Mode**
* **Fog Mode**

The day-night system uses gradual blending rather than an instant change, while rain and snow are animated using time-dependent particle-like movement.

---

### 6. User Interaction

Keyboard controls allow users to interact with the simulation and control different aspects of the environment, including:

* Camera movement
* Weather modes
* Traffic
* Lighting
* Animation states

This makes the project more than a static graphical scene and provides an interactive viewing experience.

---

##  Project Screenshots

### ☀️ Day Mode

<img width="792" height="407" alt="image" src="https://github.com/user-attachments/assets/70dd0bf3-8cd9-4493-85f6-76a8ac31a108" />


---

### 🌙 Night Mode

<img width="790" height="407" alt="image" src="https://github.com/user-attachments/assets/eca8e671-7799-45e5-9622-a297925ed123" />


---

### 🌧️ Rain Mode

<img width="792" height="402" alt="image" src="https://github.com/user-attachments/assets/4d46809f-1830-482c-92d4-82df9a436d49" />


---

### ❄️ Snow Mode

<img width="790" height="402" alt="image" src="https://github.com/user-attachments/assets/c847133f-c7bc-4c4e-a271-e76777b320ac" />


---

### 🌫️ Fog Mode

Fog mode introduces moving transparent fog bands that create additional depth and atmospheric effects within the city scene.

**Screenshot:**

`Add your Fog Mode screenshot here`

---

##  System Workflow

**Scene Initialization → Object Construction → Graphics Algorithms → Transformations → Animation Logic → Environmental Effects → User Interaction → Real-Time Rendering**

---

## 🛠️ Tech Stack

* **C++**
* **OpenGL**
* **FreeGLUT / GLUT**
* **Code::Blocks**
* **Bresenham Line Drawing Algorithm**
* **Midpoint Circle Drawing Algorithm**
* **2D Transformations**
* **Real-Time Animation**
* **Keyboard Interaction**

The project was developed using C++ with OpenGL/FreeGLUT for 2D rendering, transformations, window management, keyboard input, and animation timing.
