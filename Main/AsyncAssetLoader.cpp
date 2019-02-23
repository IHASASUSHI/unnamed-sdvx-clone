#include "stdafx.h"
#include "AsyncAssetLoader.hpp"
#include "Application.hpp"

struct AsyncLoadOperation : public IAsyncLoadable
{
	String name;
};
struct AsyncTextureLoadOperation : public AsyncLoadOperation
{
	Texture& target;
	Image image;
	AsyncTextureLoadOperation(Texture& target, const String& path) : target(target)
	{
		name = path;
	}
	bool AsyncLoad()
	{
		image = g_application->LoadImage(name);
		return image.IsValid();
	}
	bool AsyncFinalize()
	{
		target = TextureRes::Create(g_gl, image);
		return target.IsValid();
	}
};
//I have no idea what to call this thing nor do I know what exactly its returning a boolean for
struct AsyncTexturesLoadOperation : public AsyncLoadOperation
{
	Vector<Texture>& target;
	Vector<Image> images;
	AsyncTexturesLoadOperation(Vector<Texture>& target, const String& path) : target(target)
	{
		name = path;
	}
	bool AsyncLoad()
	{
		images = g_application->LoadImages(name);
		return images[0].IsValid();
	}
	bool AsyncFinalize()
	{
		for (int i = 0 ; i < images.size() ; i++)
		{
			target.push_back(TextureRes::Create(g_gl, images[i]));
		}
		return target[0].IsValid();
	}
};
struct AsyncMeshLoadOperation : public AsyncLoadOperation
{
	Mesh& target;
	AsyncMeshLoadOperation(Mesh& target, const String& path) : target(target)
	{
	}
	bool AsyncLoad()
	{
		/// TODO: No mesh loading yet
		return false;
	}
	bool AsyncFinalize()
	{
		/// TODO: No mesh loading yet
		return false;
	}
}; 
struct AsyncMaterialLoadOperation : public AsyncLoadOperation
{
	Material& target;
	AsyncMaterialLoadOperation(Material& target, const String& path) : target(target)
	{
		name = path;
	}
	bool AsyncLoad()
	{
		return true;
	}
	bool AsyncFinalize()
	{
		return (target = g_application->LoadMaterial(name)).IsValid();
	}
};
struct AsyncWrapperOperation : public AsyncLoadOperation
{
	IAsyncLoadable& target;
	AsyncWrapperOperation(IAsyncLoadable& target, const String& name) : target(target)
	{
		this->name = name;
	}
	bool AsyncLoad()
	{
		return target.AsyncLoad();
	}
	bool AsyncFinalize()
	{
		return target.AsyncFinalize();
	}
};

class AsyncAssetLoader_Impl
{
public:
	Vector<AsyncLoadOperation*> loadables;
	~AsyncAssetLoader_Impl()
	{
		for(auto& loadable : loadables)
		{
			delete loadable;
		}
	}
};

AsyncAssetLoader::AsyncAssetLoader()
{
	m_impl = new AsyncAssetLoader_Impl();
}
AsyncAssetLoader::~AsyncAssetLoader()
{
	delete m_impl;
}

void AsyncAssetLoader::AddTexture(Texture& out, const String& path)
{
	m_impl->loadables.Add(new AsyncTextureLoadOperation(out, path));
}
void AsyncAssetLoader::AddTextures(Vector<Texture>& out, const String& path)
{
	m_impl->loadables.Add(new AsyncTexturesLoadOperation(out, path));
}
void AsyncAssetLoader::AddMesh(Mesh& out, const String& path)
{
	m_impl->loadables.Add(new AsyncMeshLoadOperation(out, path));
}
void AsyncAssetLoader::AddMaterial(Material& out, const String& path)
{
	m_impl->loadables.Add(new AsyncMaterialLoadOperation(out, path));
}
void AsyncAssetLoader::AddLoadable(IAsyncLoadable& loadable, const String& id /*= "unknown"*/)
{
	m_impl->loadables.Add(new AsyncWrapperOperation(loadable, id));
}

bool AsyncAssetLoader::Load()
{
	bool success = true;
	for(auto& ld : m_impl->loadables)
	{
		if(!ld->AsyncLoad())
		{
			Logf("[AsyncLoad] Load failed on %s", Logger::Error, ld->name);
			success = false;
		}
	}
	return success;
}
bool AsyncAssetLoader::Finalize()
{
	bool success = true;
	for(auto& ld : m_impl->loadables)
	{
		if(!ld->AsyncFinalize())
		{
			Logf("[AsyncLoad] Finalize failed on %s", Logger::Error, ld->name);
			success = false;
		}
	}

	// Clear state
	delete m_impl;
	m_impl = new AsyncAssetLoader_Impl();

	return success;
}
