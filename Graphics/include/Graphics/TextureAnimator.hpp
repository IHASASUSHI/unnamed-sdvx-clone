#pragma once
#include <Graphics/ResourceTypes.hpp>
#include <Graphics/TextureAnimatorParameterManager.hpp>

namespace Graphics
{
	/*
		Texture Animator
		contains managers and handles the cleanup/lifetime of them.
	*/
	class TextureAnimatorRes
	{
	public:
		virtual ~TextureAnimatorRes() = default;
		static Ref<TextureAnimatorRes> Create(class OpenGL* gl);
	public:
		// Create a new manager
		virtual Ref<TextureAnimatorParameterManager> AddManager() = 0;
		virtual void Render(const class RenderState& rs, float deltaTime) = 0;
		// Removes all active Animators
		virtual void Reset() = 0;
	};

	typedef Ref<TextureAnimatorRes> TextureAnimator;

	DEFINE_RESOURCE_TYPE(TextureAnimator, TextureAnimatorRes);
}