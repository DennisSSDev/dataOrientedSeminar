#version 120

in vec4 vPosition;
in vec4 vSpherePosition;
in vec4 vColor;
void main()
{
	vec4 endColor = vColor;
    gl_Position    = gl_ModelViewProjectionMatrix * (vPosition + vSpherePosition);
    gl_FrontColor  = endColor;
}