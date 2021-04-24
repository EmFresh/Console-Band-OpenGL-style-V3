#version 420
//Not currently being used
//Make changes to DeferredRender.vtsh instead 

uniform mat4 uLocalModel;
uniform mat4 uWorldModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform uint uAni = 0;
uniform float uTime;

layout(location = 0) in vec3 in_vert1;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal1;

layout(location = 3) in vec3 in_vert2;
layout(location = 4) in vec3 in_normal2;


out vec2 texcoord;
out vec3 norm;
out vec4 pos;

void main()
{
    texcoord = in_uv;
    texcoord.y = 1 - texcoord.y;

    vec3 normal = mix(in_normal1, in_normal2, uTime);
    vec3 vert = mix(in_vert1, in_vert2, uTime);

    norm = mat3(uWorldModel) * (mat3(uLocalModel) * normal);

    vec4 viewSpace =  uWorldModel * (uLocalModel * vec4(vert, 1.0f));
    viewSpace = uProj * uView * viewSpace; 

    gl_Position = viewSpace;
    
    pos = viewSpace/viewSpace.w;
}