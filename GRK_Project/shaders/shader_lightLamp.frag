#version 430 core
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform vec3 viewPos;
uniform vec3 lightAmbient;
uniform vec3 lightPos;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

void main()
{
    // Ambient
    vec3 ambient = lightAmbient * vec3(0.1, 0.1, 0.1);

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightDiffuse * diff;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = lightSpecular * spec;

    // Combine results
    color = vec4(ambient + diffuse + specular, 1.0);
}