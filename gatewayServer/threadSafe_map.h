#include <boost/thread.hpp>
#include <map>

template<typename K, typename V>
class CThreadSafeMap
{
public:
	CThreadSafeMap() {}
	~CThreadSafeMap() 
	{
		if (!map_.empty())
		{
			map_.clear();
		}
	}

	void insert(const K& key, const V& value)
	{
		boost::mutex::scoped_lock oLock(mutex_);
		map_.insert(std::pair<K, V>(key, value));
	}

	bool find(const K& key, V& value)
	{
		bool ret = false;
		boost::mutex::scoped_lock oLock(mutex_);
		if (map_.size() > 0)
		{
			typedef typename std::map<K, V>::iterator iter_thread;
			iter_thread iter= map_.find(key);
			if(iter != map_.end())
			{
				value = iter->second;
				ret = true;
			}
		}

		return ret;
	}

	bool findValueByKey(const K& key, const V& value)
	{
		bool ret = false;
		boost::mutex::scoped_lock oLock(mutex_);
		if (map_.size() > 0)
		{
			typedef typename std::map<K, V>::iterator iter_thread;
			iter_thread iter = map_.find(key);
			if (iter != map_.end())
			{
				if (value == iter->second)
				{
					ret = true;
				}
			}
		}

		return ret;
	}

	bool findAndSet(const K& key, const V& new_value, V& old_value)
	{
		bool ret = false;
		boost::mutex::scoped_lock oLock(mutex_);
		if (map_.size() > 0)
		{
			typedef typename std::map<K, V>::iterator iter_thread;
			iter_thread iter= map_.find(key);
			if(iter != map_.end())
			{
				old_value = iter->second;
				map_.erase(iter);
				map_.insert(std::pair<K, V>(key, new_value));
				ret = true;
			}
		}

		return ret;
	}

	void erase(const K& key)
	{
		boost::mutex::scoped_lock oLock(mutex_);
		map_.erase(key);
	}

	void clear()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		if (!map_.empty())
		{
			map_.clear();
		}

		return;
	}

	int size()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		return map_.size();
	}

	bool empty()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		return map_.empty();
	}

private:
	boost::mutex mutex_;
	std::map<K, V> map_;
};
