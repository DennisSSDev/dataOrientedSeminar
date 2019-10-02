#version 120
in vec4 vPosition;
void main()
{
    gl_Position    = gl_ModelViewProjectionMatrix * vPosition;
    gl_FrontColor  = gl_Color;
}