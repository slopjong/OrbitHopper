uniform sampler2D blurred;
uniform float pixsize;

void main()
{
	vec4 sum = texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,-5))*0.01222447;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,-4))*0.02783468;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,-3))*0.06559061;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,-2))*0.12097757;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,-1))*0.17466632;
	sum += texture2D(blurred,gl_TexCoord[0].xy)*0.19741265;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,1))*0.17466632;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,2))*0.12097757;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,3))*0.06559061;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,4))*0.02783468;
	sum += texture2D(blurred,gl_TexCoord[0].xy+pixsize*vec2(0,5))*0.01222447;
	gl_FragColor = sum;
}
