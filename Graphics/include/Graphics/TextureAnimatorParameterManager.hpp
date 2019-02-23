#pragma once
#include <Graphics/Material.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/TextureAnimatorParameter.hpp>

namespace Graphics
{
	/*
		Particle Emitter, which is a component of a particle system that handles the emission of particles together with the properties of the emitter particles
	*/
	class TextureAnimatorParameterManager
	{
	public:
		~TextureAnimatorParameterManager();

		// Material used for the Animaton
		Material material;

		// Textures to use for the Animaton
		Vector<Texture> textures;

		// Animaton location
		Vector3 position;

		// Amount of loops to make
		// 0 = forever
		uint32 loops = 0;

		// Texture animator parameter accessors
#define TEXTUR_ANIMATOR_PARAMETER(__name, __type)\
	void Set##__name(const ITextureAnimatorParameter<__type>& param)\
	{\
		if(m_param_##__name)\
			delete m_param_##__name;\
		m_param_##__name = param.Duplicate();\
	}
#include <Graphics/TextureAnimatorParameters.hpp>

	// True after all loops are done playing
		bool HasFinished() const { return m_finished; }

		// Restarts a animator manager
		void Reset();

		// Stop a animator manager
		void Deactivate();

	private:
		// Constructed by Texture Animator
		TextureAnimatorParameterManager(class TextureSystem_Impl* sys);
		void Render(const class RenderState& rs, float deltaTime);
		void m_ReallocatePool(uint32 newCapacity);

		float m_AnimatorTime = 0; //how much time has passed
		float m_index = 0;
		bool m_deactivated = false;
		bool m_finished = false;
		uint32 m_AnimatorLoopIndex = 0;

		friend class TextureSystem_Impl;
		friend class Animator;
		TextureSystem_Impl* m_system;

		class Animator* m_Animator = nullptr;
		uint32 m_poolSize = 0;

		// Particle parameters private
#define TEXTUR_ANIMATOR_PARAMETER(__name, __type)\
	ITextureAnimatorParameter<__type>* m_param_##__name = nullptr;
#include <Graphics/TextureAnimatorParameters.hpp>
	};
}