# GRK_PROJECT
Documentation:

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
