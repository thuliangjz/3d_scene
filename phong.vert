attribute vec3 pos_vtx;
attribute vec3 normal_vtx;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalization;
//
varying vec3 frag_pos;
varying vec3 frag_normal;
void main(){
    frag_pos = vec3(model * vec4(pos_vtx, 1.0));
    frag_normal = normalization * normal_vtx;

    gl_Position = projection * view * model * vec4(pos_vtx, 1.0);
}
