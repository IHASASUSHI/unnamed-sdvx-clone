#include "stdafx.h"
#include "OpenGL.hpp"
#include "TextureAnimator.hpp"
#include "Mesh.hpp"
#include "VertexFormat.hpp"
#include <Graphics/ResourceManagers.hpp>

namespace Graphics
{
	struct TextureVertex : VertexFormat<Vector3, Vector4, Vector4>
	{
		TextureVertex(Vector3 pos, Color color, Vector4 params) : pos(pos), color(color), params(params) {};
		Vector3 pos;
		Color color;
		Vector4 params;
	};

	class TextureSystem_Impl : public TextureAnimatorRes
	{
		friend class TextureAnimatorParameterManager;
		Vector<Ref<TextureAnimatorParameterManager>> m_managers;

	public:
		OpenGL* gl;

	public:
		virtual void Render(const class RenderState& rs, float deltaTime) override
		{
			// Enable blending for all particles
			glEnable(GL_BLEND);

			// Tick all emitters and remove old ones
			for(auto it = m_managers.begin(); it != m_managers.end();)
			{
				(*it)->Render(rs, deltaTime);

				if (it->GetRefCount() == 1)
				{
					if ((*it)->HasFinished())
					{
						// Remove unreferenced and finished emitters
						it = m_managers.erase(it);
						continue;
					}
					else if ((*it)->loops == 0)
					{
						// Deactivate unreferenced infinte duration emitters
						(*it)->Deactivate();
					}
				}

				it++;
			}
		}
		virtual Ref<TextureAnimatorParameterManager> AddManager() override
		{
			Ref<TextureAnimatorParameterManager> newManager = Utility::MakeRef<TextureAnimatorParameterManager>(new TextureAnimatorParameterManager(this));
			m_managers.Add(newManager);
			return newManager;
		}
		virtual void Reset()
		{
			for (auto mgr : m_managers)
			{
				mgr.Destroy();
			}
			m_managers.clear();
		}
	};

	Ref<TextureAnimatorRes> TextureAnimatorRes::Create(class OpenGL* gl)
	{
		TextureSystem_Impl* impl = new TextureSystem_Impl();
		impl->gl = gl;
		Ref<TextureAnimatorRes> temp = GetResourceManager<ResourceType::TextureAnimator>().Register(impl);
		return temp;
	}


	// Particle/Animator instance class
	class Animator
	{
	public:
		float life = 0.0f;
		float lastlife = 0.0f;
		float maxLife = 0.0f;
		float Size = 0.0f;
		Color color;
		Vector3 pos;
		float alpha;
		float rotation;
		float idx;

		bool IsAlive() const
		{
			return life > 0.0f;
		}
		inline void Init(TextureAnimatorParameterManager* manager)
		{
			life = maxLife = manager->m_param_Lifetime->Init(1);
			idx = 0;
			// Add emitter offset to location
			pos = manager->position;
			rotation = manager->m_param_Rotation->Init(1);
			color = manager->m_param_StartColor->Init(1);
			Size = manager->m_param_Size->Init(1);
			alpha = manager->m_param_Alpha->Sample(1);
		}
		inline void Simulate(TextureAnimatorParameterManager* manager, float deltaTime)
		{
			life -= deltaTime;
			lastlife += deltaTime;
			if (lastlife >= manager->m_param_Lifetime->Sample(1)/manager->textures.size()) {
				idx++;
				lastlife = 0.0f;
			}
			if (idx >= manager->textures.size()) {
				idx--;
			}
			if (life - deltaTime <= 0.0f) {
				manager->m_AnimatorLoopIndex++;
			}
		}
	};

	TextureAnimatorParameterManager::TextureAnimatorParameterManager(TextureSystem_Impl* sys) : m_system(sys)
	{
		// Set parameter defaults
#define TEXTURE_ANIMATOR_DEFAULT(__name, __value)\
	Set##__name(__value);
#include "TextureAnimatorParameters.hpp"
	}
	TextureAnimatorParameterManager::~TextureAnimatorParameterManager()
	{
		// Cleanup animator parameters
#define TEXTURE_ANIMATOR_PARAMETER(__name, __type)\
	if(m_param_##__name){\
		delete m_param_##__name; m_param_##__name = nullptr; }
#include "TextureAnimatorParameters.hpp"

		if (m_Animator)
		{
			delete[] m_Animator;
		}
	}

	void TextureAnimatorParameterManager::m_ReallocatePool(uint32 newCapacity)
	{
		Animator* oldAnimations = m_Animator;
		uint32 oldSize = m_poolSize;

		// Create new pool
		m_poolSize = newCapacity;
		if (newCapacity > 0)
		{
			m_Animator = new Animator[m_poolSize];
			memset(m_Animator, 0, m_poolSize * sizeof(Animator));
		}
		else
		{
			m_Animator = nullptr;
		}

		if (oldAnimations && m_Animator)
		{
			memcpy(m_Animator, oldAnimations, Math::Min(oldSize, m_poolSize) * sizeof(Animator));
		}

		if (oldAnimations)
			delete[] oldAnimations;
	}
	void TextureAnimatorParameterManager::Render(const class RenderState& rs, float deltaTime)
	{
		if (m_finished)
			return;
		uint32 maxSpawns = (uint32)ceilf(m_param_SpawnRate->GetMax());
		uint32 maxFrames = textures.size() - 1;

		if (m_poolSize == 0)
			m_ReallocatePool(1);

		// Resulting vertex bufffer
		Vector<TextureVertex> verts;

		m_index = m_AnimatorTime;

		// Increment Animator time 
		m_AnimatorTime += deltaTime;

		if (loops > 0 && m_AnimatorLoopIndex >= loops) // Should spawn particles ?
			m_deactivated = true;
		
		Animator& Animation = m_Animator[0];
		bool render = false;
		
		if (!m_deactivated) {
			if (!m_Animator[0].IsAlive())
			{
				// Initiate Animation
				Animation.Init(this);
				render = true;
			}
			else
			{	//Start Animation
				Animation.Simulate(this, deltaTime);
				render = true;
			}
			if (render)
			{
				verts.Add({ Animation.pos, Animation.color.WithAlpha(Animation.alpha), Vector4(Animation.Size, Animation.rotation, 0.0f, 0.0f) });
				MaterialParameterSet params;
				if (textures[Animation.idx])
				{
					params.SetParameter("mainTex", textures[Animation.idx]);
				}
				material->Bind(rs, params);

				// Select blending mode based on material
				switch (material->blendMode)
				{
				case MaterialBlendMode::Normal:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case MaterialBlendMode::Additive:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					break;
				case MaterialBlendMode::Multiply:
					glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
					break;
				}
				// Create vertex buffer
				Mesh mesh = MeshRes::Create(m_system->gl);

				mesh->SetData(verts);
				mesh->SetPrimitiveType(PrimitiveType::PointList);
				mesh->Draw();
				mesh.Destroy();
			}
		}
		else
		{
			m_finished = true;
		}
	}

	void TextureAnimatorParameterManager::Reset()
	{
		m_deactivated = false;
		m_finished = false;
		delete[] m_Animator;
		m_Animator = nullptr;
		m_AnimatorLoopIndex = 0;
		m_AnimatorTime = 0;
		m_poolSize = 0;
		m_index = 0;
	}

	void TextureAnimatorParameterManager::Deactivate()
	{
		m_deactivated = true;
	}
}
