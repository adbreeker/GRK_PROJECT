# GRK_PROJECT

<!-- TABLE OF CONTENTS -->
## Table of Contents
* [Project description](#description)
* [Link to tasks documentation](#tasks-documentation)
* [List of objects](#list-of-objects)
* [Video presentation](#short-video-presentation-of-created-scene)

<!-- Project description -->
## Description
All scene models are in ./GRK_Project/models/{model category}/{model name} here you have {model name}.obj and optionaly {model name}.mtl, texture.png

Worth mentioning is function drawObjectPBR(context, modelMatrix, color, textureId, roughness, metallic, brightness) which is responsible for drawing all objects in scene,

    *context - model to draw from namespace models
    *modelMatrix - position, rotation, and scale of object to draw
    *color - vector containing information of color (use empty glm::vec3() if you use texture instead of color)
    *textureId - id of texture we want to use or just NULL if we using colors
    *roughness - roughness in scale from 0 to 1
    *metallic - metallic in scale from 0 to 1
    *brightness - brightness of color/texture, 1.0 is unchanged brightness

if textureId is NULL then function draws object with use of single passed color
else function draws object with texture connected with this textureId

<!-- Link to tasks documentation -->
## Tasks documentation
[docs](https://docs.google.com/document/d/15XWFKD8g9I5ik855HuAhfDH8_2lMuJDCnHIDn98vivM/edit?usp=sharing)

<!-- List of objects -->
## List of objects

1. Objects downloaded from internet:
   - bug(player) (modified and animated by us)
   - dog (modified and animated by us)
   - bird (modfied and animated by us)
   - jacket
   - books (modified by us)
   - open book (modified by us)
2. Objects created by us:
   - barbells
   - bench
   - beret
   - bone toy
   - bush
   - cabinet
   - cap hat
   - carpet
   - ceiling
   - dog bed
   - duvet
   - floor
   - food bowl
   - ground
   - hat shelf
   - hatstand
   - high hat
   - lamp
   - mat
   - painting
   - pillows
   - plant
   - poster
   - pot
   - rock
   - roof
   - shelf
   - skybox
   - stool
   - switch
   - table
   - table lamp
   - tennis ball
   - tree
   - umbrella
   - umbrella holder
   - wallpaper
   - wardrobe
   - water bowl
3. Objects from project base:
   - bed 
   - chair
   - desk (modified by us)
   - door (modified and animated by us)
   - drawer
   - windows (modified by us)
   - jamb (modified by us)
   - marble bust
   - mattres
   - pencils
   - room (modifed by us)

<!-- Video presentation -->
## Short video presentation of created scene


https://user-images.githubusercontent.com/111668308/218335853-f66d2366-c06f-41e9-997c-ccff22489b88.mp4

