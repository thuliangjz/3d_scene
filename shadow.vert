attribute vec3 pos_vtx;

uniform mat4 light_transform;

void main(){
    gl_Position = light_transform * vec4(pos_vtx, 1.f);
}
