attribute vec3 pos_vtx;
attribute vec3 normal_vtx;
attribute vec2 tex_uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalization;
uniform mat4 light_transform;
varying vec3 frag_pos;
varying vec3 frag_normal;
varying vec2 texture_coord;
varying vec2 shadow_pos;
varying vec4 frag_pos_light;

void main(){
    frag_pos = vec3(model * vec4(pos_vtx, 1.0));
    frag_pos_light = light_transform * vec4(pos_vtx, 1.);
    frag_normal = normalization * normal_vtx;
    texture_coord = tex_uv;
    shadow_pos = vec2(pos_vtx);
    gl_Position = projection * view * model * vec4(pos_vtx, 1.0);
}
