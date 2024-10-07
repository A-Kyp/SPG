#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform vec3 generator_position;
uniform float deltaTime;

out float vert_lifetime;
out float vert_iLifetime;

uniform vec3 control_p0_1, control_p1_1, control_p2_1, control_p3_1;
uniform vec3 control_p0_2, control_p1_2, control_p2_2, control_p3_2;
uniform vec3 control_p0_3, control_p1_3, control_p2_3, control_p3_3;
uniform vec3 control_p0_4, control_p1_4, control_p2_4, control_p3_4;
uniform vec3 control_p0_5, control_p1_5, control_p2_5, control_p3_5;

struct Particle
{
    vec4 position;
    vec4 speed;
    vec4 iposition;
    vec4 ispeed;
    float delay;
    float iDelay;
    float lifetime;
    float iLifetime;
};


layout(std430, binding = 0) buffer particles {
    Particle data[];
};


float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 bezier(float t, vec3 control_p0, vec3 control_p1, vec3 control_p2, vec3 control_p3)
{
    return  control_p0 * pow((1 - t), 3) +
            control_p1 * 3 * t * pow((1 - t), 2) +
            control_p2 * 3 * pow(t, 2) * (1 - t) +
            control_p3 * pow(t, 3);
}

vec3 rotateY(vec3 point, float u)
{
    float x = point.x * cos(u) - point.z *sin(u);
    float z = point.x * sin(u) + point.z *cos(u);
    return vec3(x, point.y, z);
}

void main()
{
    vec3 pos = data[gl_VertexID].position.xyz;
    vec3 spd = data[gl_VertexID].speed.xyz;
    float lifetime = data[gl_VertexID].lifetime;

    float dt = deltaTime * 5;

    float t = 1 - lifetime / data[gl_VertexID].iLifetime;
    vec3 bezier_path = vec3(1);

    int Bezier_id = gl_VertexID % 5;
    switch (Bezier_id) {
        case 0:
        {
            bezier_path = bezier(t, control_p0_1, control_p1_1, control_p2_1, control_p3_1);
            break;
        }
        case 1:
        {
            bezier_path = bezier(t, rotateY(control_p0_2, radians(72)), rotateY(control_p1_2, radians(72)), rotateY(control_p2_2, radians(72)), rotateY(control_p3_2, 72));
            break;
        }
        case 2:
        {
            bezier_path = bezier(t, rotateY(control_p0_3, radians(144)), rotateY(control_p1_3, radians(144)), rotateY(control_p2_3, radians(144)), rotateY(control_p3_3, radians(144)));
            break;
        }
        case 3:
        {
            bezier_path = bezier(t, rotateY(control_p0_4, radians(216)), rotateY(control_p1_4, radians(216)), rotateY(control_p2_4, radians(216)), rotateY(control_p3_4, radians(216)));
            break;
        }
        case 4:
        {
            bezier_path = bezier(t, rotateY(control_p0_5, radians(286)), rotateY(control_p1_5, radians(286)), rotateY(control_p2_5, radians(286)), rotateY(control_p3_5, radians(286)));
            break;
        }
        default:
        {
            break; 
        }
    }

    lifetime -= deltaTime;
    pos = bezier_path / 1;

    if (lifetime < 0)
    {
        pos = data[gl_VertexID].iposition.xyz;
        lifetime = data[gl_VertexID].iLifetime;
    }

    data[gl_VertexID].position.xyz = pos;
    data[gl_VertexID].lifetime = lifetime;

    vert_lifetime = lifetime;
    vert_iLifetime = data[gl_VertexID].iLifetime;

    gl_Position = Model * vec4(pos + generator_position, 1);
}
