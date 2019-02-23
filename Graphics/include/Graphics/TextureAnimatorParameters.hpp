/*
	Macro file for creating particle parameter entries in particle systems
	when this file is included it will call PARTICLE_PARAMETER() or PARTICLE_DEFAULT() if they are defined with the values in this file
*/
#ifdef TEXTUR_ANIMATOR_PARAMETER
TEXTUR_ANIMATOR_PARAMETER(Lifetime, float)
TEXTUR_ANIMATOR_PARAMETER(Alpha, float)
TEXTUR_ANIMATOR_PARAMETER(StartColor, Color)
TEXTUR_ANIMATOR_PARAMETER(Size, float)
TEXTUR_ANIMATOR_PARAMETER(Rotation, float)
TEXTUR_ANIMATOR_PARAMETER(Position, Vector3)
TEXTUR_ANIMATOR_PARAMETER(SpawnRate, float)
#undef TEXTUR_ANIMATOR_PARAMETER
#endif

#ifdef TEXTURE_ANIMATOR_DEFAULT
TEXTURE_ANIMATOR_DEFAULT(Lifetime, Constant<float>(10.0f))
TEXTURE_ANIMATOR_DEFAULT(Alpha, Constant<float>(1.0f))
TEXTURE_ANIMATOR_DEFAULT(StartColor, Constant<Color>(Color::White))
TEXTURE_ANIMATOR_DEFAULT(Size, Constant<float>(1.0f))
TEXTURE_ANIMATOR_DEFAULT(Rotation, Constant<float>(0))
TEXTURE_ANIMATOR_DEFAULT(Position, Constant<Vector3>(Vector3(0.0f)))
TEXTURE_ANIMATOR_DEFAULT(SpawnRate, Constant<float>(20))
#undef TEXTURE_ANIMATOR_DEFAULT
#endif