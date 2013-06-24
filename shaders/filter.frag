uniform sampler2D tex;

void main()
{
	vec4 entry = texture2D(tex,gl_TexCoord[0].xy);
	gl_FragColor =  vec4(entry.xyz*(1.0-entry.a),entry.a);
}
