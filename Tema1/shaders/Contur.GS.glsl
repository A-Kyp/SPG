#version 430

layout(triangles) in;
// TODO(student): Update max_vertices
layout(line_strip, max_vertices = 12) out;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 viewMatrices[6];
uniform vec3 visual_vector;

in vec3 geom_position[3];
in vec2 geom_texture_coord[3];
in vec3 g_normal[3];

vec3 compute_point(vec3 P1, vec3 P2, float dot1, float dot2) {
    // vec3 P12 = P1 + (P2 - P1) * (dot12 - dot1) / (dot2 - dot1);
    vec3 P12 = P1 + (P2 - P1) * (abs(dot1) / (abs(dot1) + abs(dot2)));

    return P12;
}

void main()
{
    int pt = 0;
    int layer;

    // TODO(student): Update the code to compute the position from each camera view 
    // in order to render a cubemap in one pass using gl_Layer. Use the "viewMatrices"
    // attribute according to the corresponding layer.

    //redarea primitivei din directia camerei specifica layer-ului curent, 
    //folosind setul de matrici de vizualizare primite in variabila viewMatrices
    for (layer = 0; layer < 6; layer++) {
        gl_Layer = layer;

        vec3 P1 = gl_in[0].gl_Position.xyz;
        vec3 P2 = gl_in[1].gl_Position.xyz;
        vec3 P3 = gl_in[2].gl_Position.xyz;

        
        float dot1 = dot((visual_vector), g_normal[0]);
        float dot2 = dot((visual_vector), g_normal[1]);
        float dot3 = dot((visual_vector), g_normal[2]);

        if(dot1 >= 0 && dot2 <= 0) {
            vec3 P12 = compute_point(P1, P2, dot1, dot2);
            gl_Position = Projection * viewMatrices[layer] * vec4(P12, 1);
            EmitVertex();
        }

        if(dot1 >= 0 && dot3 <= 0) {
            vec3 P13 = compute_point(P1, P3, dot1, dot3);
            gl_Position = Projection * viewMatrices[layer] * vec4(P13, 1);
            EmitVertex();
        }

        if(dot2 >= 0 && dot1 <= 0) {
            // vec3 P21 = compute_point(P1, P2, dot1, dot2);
            vec3 P21 = compute_point(P2, P1, dot2, dot1);
            gl_Position = Projection * viewMatrices[layer] * vec4(P21, 1);
            EmitVertex();
        }

        if(dot2 >= 0 && dot3 <= 0) {
            vec3 P23 = compute_point(P2, P3, dot2, dot3);
            gl_Position = Projection * viewMatrices[layer] * vec4(P23, 1);
            EmitVertex();
        }

        if(dot3 >= 0 && dot1 <= 0) {
            vec3 P31 = compute_point(P3, P1, dot3, dot1);
            // vec3 P31 = compute_point(P1, P3, dot1, dot3);
            gl_Position = Projection * viewMatrices[layer] * vec4(P31, 1);
            EmitVertex();
        }

        if(dot3 >= 0 && dot2 <= 0) {
            vec3 P32 = compute_point(P3, P2, dot3, dot2);
            // vec3 P32 = compute_point(P2, P3, dot2, dot3);
            gl_Position = Projection * viewMatrices[layer] * vec4(P32, 1);
            EmitVertex();
        }
 
        EndPrimitive();
    }
}

















