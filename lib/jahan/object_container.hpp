// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_OBJECT_CONTAINER_H_
#define JAHAN_OBJECT_CONTAINER_H_

#include <memory>

namespace jahan {
	template <class GEM>
	class ObjectContainer
	{
	public:
		static void set(std::shared_ptr<GEM> gem)
		{
			m_gem = gem;
		}

		static std::shared_ptr<GEM> get()
		{
			return m_gem;
		}

		static std::shared_ptr<GEM> pop()
		{
			std::move(m_gem);
		}
	protected:
		static std::shared_ptr<GEM> m_gem;
	};

	template <class GEM>
	std::shared_ptr<GEM> ObjectContainer<GEM>::m_gem;
}

#endif //JAHAN_OBJECT_CONTAINER_H_