attribute vec3 pos;
attribute vec3 color_in;

varying vec3 color;

void main(){
    gl_Position = vec4(pos, 1.0);
    color = color_in;
}
