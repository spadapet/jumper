#pragma once

namespace ff
{
	class AppGlobals;
	class EntityDomain;
}

class IRenderQueueService;

class SystemBase
{
public:
	SystemBase();
	virtual ~SystemBase();

	template<class T, class... Args>
	static std::shared_ptr<T> New(Args&&...args)
	{
		std::shared_ptr<T> system = std::make_shared<T>(std::forward<Args>(args)...);
		system->SetName(ff::String::from_acp(typeid(T).name() + 6));
		return system;
	}

	static void AdvanceHelper(ff::EntityDomain* domain, const ff::Vector<std::shared_ptr<SystemBase>>& systems);

	virtual void Advance(ff::EntityDomain* domain);
	virtual void Render(ff::EntityDomain* domain, IRenderQueueService* render);

	void SetName(ff::StringRef value);
	ff::StringRef GetName() const;
	void SetAdvanceTime(int64_t value);
	int64_t GetAdvanceTime() const;
	void SetRenderQueueTime(int64_t value);
	int64_t GetRenderQueueTime() const;

private:
	ff::String _name;
	int64_t _advanceTime;
	int64_t _renderQueueTime;
};

typedef ff::Vector<std::shared_ptr<SystemBase>> SystemVector;
typedef SystemVector::const_iterator SystemIter;
