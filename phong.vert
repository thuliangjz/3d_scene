attribute vec3 pos_vtx;
attribute vec3 normal_vtx;
attribute vec2 tex_uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalization;

varying vec3 frag_pos;
varying vec3 frag_normal;
varying vec2 texture_coord;
void main(){
    frag_pos = vec3(model * vec4(pos_vtx, 1.0));
    frag_normal = normalization * normal_vtx;
    texture_coord = tex_uv;
    gl_Position = projection * view * model * vec4(pos_vtx, 1.0);
}
